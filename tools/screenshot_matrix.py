#!/usr/bin/env python3
"""
Screenshot-matrix harness for TimeStyle.

Boots the Pebble emulator for each requested platform, drives the watchface
through every named config in tools/screenshot_configs.json (via
`pebble send-app-message`, the same AppMessage path the real config page
uses), pins time/battery/health/weather so shots are deterministic, and
saves one PNG per (platform, config) plus an index.html contact sheet.

Usage:
  python3 tools/screenshot_matrix.py                        # default platforms
  python3 tools/screenshot_matrix.py --platforms basalt,emery,chalk
  python3 tools/screenshot_matrix.py --only layout-right,sidebar-packed
  python3 tools/screenshot_matrix.py --list                 # list config names

Requires the project to be built first (`pebble build`) -- the .pbw and
build/js/message_keys.json must exist.
"""

import argparse
import datetime
import html
import json
import subprocess
import sys
import time
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
CONFIGS_PATH = Path(__file__).resolve().parent / "screenshot_configs.json"
MESSAGE_KEYS_PATH = PROJECT_ROOT / "build" / "js" / "message_keys.json"

DEFAULT_PLATFORMS = ["basalt", "chalk", "diorite", "emery", "flint", "gabbro"]

# pinned environment: Wednesday 2026-03-25, 10:08:30 local time
PINNED_TIME = datetime.datetime(2026, 3, 25, 10, 8, 30)
PINNED_BATTERY_PCT = 70
PINNED_STEPS = 7241
PINNED_SLEEP_MINUTES = 452
PINNED_HEART_RATE = 68  # emery only; ignored elsewhere

# enums mirrored from src/c (sidebar_layout.h, sidebar_widgets.h, weather.h)
LAYOUTS = {
    "SIDEBAR_RIGHT": 0,
    "SIDEBAR_LEFT": 1,
    "SIDEBAR_RIGHT_BOTTOM_BAR": 2,
    "SIDEBAR_LEFT_BOTTOM_BAR": 3,
    "BOTTOM_BAR_ONLY": 4,
}
WIDGETS = {
    "EMPTY": 0,
    "BLUETOOTH_DISCONNECT": 1,
    "BATTERY_METER": 2,
    "WORLD_TIME_1": 3,
    "DATE": 4,
    "SECONDS": 5,
    "WEEK_NUMBER": 6,
    "WEATHER_CURRENT": 7,
    "WEATHER_FORECAST_TODAY": 8,
    "SLEEP_TIMER": 9,
    "STEP_COUNTER": 10,
    "BEATS": 11,
    "HEARTRATE": 12,
    "WEATHER_UV_INDEX": 13,
    "WORLD_TIME_2": 14,
    "NEXT_SUN_EVENT": 15,
    "WEATHER_WIND": 16,
    "WEATHER_PRECIP_CHANCE": 17,
}
WEATHER_CONDITIONS = {
    "CLEAR_DAY": 0,
    "CLEAR_NIGHT": 1,
    "CLOUDY_DAY": 2,
    "HEAVY_RAIN": 3,
    "HEAVY_SNOW": 4,
    "LIGHT_RAIN": 5,
    "LIGHT_SNOW": 6,
    "PARTLY_CLOUDY_NIGHT": 7,
    "PARTLY_CLOUDY": 8,
    "RAINING_AND_SNOWING": 9,
    "THUNDERSTORM": 10,
    "WEATHER_GENERIC": 11,
}
SYMBOLS = {}
SYMBOLS.update(LAYOUTS)
SYMBOLS.update(WIDGETS)
SYMBOLS.update(WEATHER_CONDITIONS)


def run_pebble(args, platform, check=True, attempts=3):
    """Run a pebble CLI command; retry on failure (the emulator connection
    occasionally throws transient TimeoutErrors under rapid calls)."""
    cmd = ["pebble"] + args + ["--emulator", platform]
    for attempt in range(attempts):
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode == 0 or not check:
            return result
        if attempt < attempts - 1:
            time.sleep(2)
    raise RuntimeError(
        f"`{' '.join(cmd)}` failed after {attempts} attempts:\n"
        f"{result.stdout}\n{result.stderr}"
    )


def resolve_value(raw):
    """Resolve a config value to (kind, int_or_str). kind: uint/int/string."""
    if isinstance(raw, bool):
        return ("uint", int(raw))
    if isinstance(raw, int):
        return ("int", raw) if raw < 0 else ("uint", raw)
    if isinstance(raw, str):
        if raw in SYMBOLS:
            return ("uint", SYMBOLS[raw])
        if raw.startswith("#") and len(raw) == 7:
            return ("uint", int(raw[1:], 16))
        if raw.startswith("0x"):
            return ("uint", int(raw, 16))
        return ("string", raw)
    raise ValueError(f"can't resolve config value: {raw!r}")


def send_config(platform, app_uuid, key_ids, merged_settings):
    groups = {"uint": [], "int": [], "string": []}
    for key_name, raw in merged_settings.items():
        if key_name.startswith("_"):
            continue
        if key_name not in key_ids:
            raise KeyError(f"unknown message key {key_name!r} (not in message_keys.json)")
        kind, value = resolve_value(raw)
        groups[kind].append(f"{key_ids[key_name]}={value}")

    args = ["send-app-message", "--app-uuid", app_uuid]
    for kind in ("uint", "int", "string"):
        if groups[kind]:
            args += [f"--{kind}"] + groups[kind]
    run_pebble(args, platform)


def pin_environment(platform):
    run_pebble(["emu-battery", "--percent", str(PINNED_BATTERY_PCT)], platform)
    run_pebble(["emu-bt-connection", "--connected", "yes"], platform)
    # health injection isn't supported on every platform; best-effort
    run_pebble(["emu-steps", str(PINNED_STEPS)], platform, check=False)
    run_pebble(["emu-sleep", str(PINNED_SLEEP_MINUTES)], platform, check=False)
    if platform == "emery":
        run_pebble(["emu-heart-rate", str(PINNED_HEART_RATE)], platform, check=False)


def write_contact_sheet(out_dir, config_order):
    """Build index.html from every PNG on disk, so partial reruns
    (--only/--platforms) refresh the sheet without dropping older shots."""
    platforms = sorted(
        (d.name for d in out_dir.iterdir() if d.is_dir()),
        key=lambda p: (DEFAULT_PLATFORMS.index(p) if p in DEFAULT_PLATFORMS else 99, p),
    )
    on_disk = {
        (p, png.stem): f"{p}/{png.name}"
        for p in platforms
        for png in (out_dir / p).glob("*.png")
    }
    names_on_disk = {name for (_, name) in on_disk}
    config_names = [n for n in config_order if n in names_on_disk]
    config_names += sorted(names_on_disk - set(config_names))

    rows = []
    for name in config_names:
        cells = []
        for platform in platforms:
            rel = on_disk.get((platform, name))
            if rel:
                cells.append(
                    f'<td><a href="{rel}"><img src="{rel}" alt="{html.escape(name)} on {platform}"></a></td>'
                )
            else:
                cells.append("<td class='missing'>missing</td>")
        rows.append(f"<tr><th>{html.escape(name)}</th>{''.join(cells)}</tr>")

    header_cells = "".join(f"<th>{p}</th>" for p in platforms)
    page = f"""<!doctype html>
<meta charset="utf-8">
<title>TimeStyle screenshot matrix</title>
<style>
  body {{ font-family: -apple-system, sans-serif; background: #1c1c1e; color: #eee; margin: 1.5em; }}
  table {{ border-collapse: collapse; }}
  th, td {{ padding: 6px 10px; text-align: left; vertical-align: middle; }}
  thead th {{ position: sticky; top: 0; background: #1c1c1e; }}
  tbody th {{ font-weight: 500; font-size: 0.85em; white-space: nowrap; }}
  img {{ display: block; image-rendering: pixelated; height: 168px; width: auto; }}
  td.missing {{ color: #f66; font-size: 0.85em; }}
  p {{ color: #999; font-size: 0.85em; }}
</style>
<h1>TimeStyle screenshot matrix</h1>
<p>Generated {datetime.datetime.now():%Y-%m-%d %H:%M} &middot; pinned to {PINNED_TIME:%a %Y-%m-%d %H:%M:%S}, battery {PINNED_BATTERY_PCT}%, {PINNED_STEPS} steps</p>
<table>
<thead><tr><th></th>{header_cells}</tr></thead>
<tbody>{''.join(rows)}</tbody>
</table>
"""
    index = out_dir / "index.html"
    index.write_text(page)
    return index


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[1])
    parser.add_argument("--platforms", default=",".join(DEFAULT_PLATFORMS),
                        help="comma-separated emulator platforms")
    parser.add_argument("--only", default=None,
                        help="comma-separated config names to run (default: all)")
    parser.add_argument("--out", default=str(PROJECT_ROOT / "screenshots"),
                        help="output directory")
    parser.add_argument("--list", action="store_true", help="list config names and exit")
    parser.add_argument("--settle", type=float, default=1.0,
                        help="seconds to wait between config send and screenshot")
    args = parser.parse_args()

    matrix = json.loads(CONFIGS_PATH.read_text())
    configs = matrix["configs"]
    if args.list:
        for config in configs:
            print(config["name"])
        return 0

    if not MESSAGE_KEYS_PATH.exists():
        sys.exit("build/js/message_keys.json not found -- run `pebble build` first")
    key_ids = json.loads(MESSAGE_KEYS_PATH.read_text())
    app_uuid = json.loads((PROJECT_ROOT / "package.json").read_text())["pebble"]["uuid"]

    if args.only:
        wanted = set(args.only.split(","))
        unknown = wanted - {c["name"] for c in configs}
        if unknown:
            sys.exit(f"unknown config names: {', '.join(sorted(unknown))}")
        configs = [c for c in configs if c["name"] in wanted]

    platforms = args.platforms.split(",")
    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    results = {}
    failures = []
    for platform in platforms:
        print(f"=== {platform}: installing (boots emulator if needed)...")
        try:
            run_pebble(["install"], platform)
        except RuntimeError:
            # wedged emulator: kill everything, wipe emulator state, retry
            print(f"  install timed out; killing + wiping emulators and retrying...")
            subprocess.run(["pebble", "kill"], capture_output=True)
            subprocess.run(["pebble", "wipe"], capture_output=True)
            run_pebble(["install"], platform)
        time.sleep(3)
        pin_environment(platform)

        platform_dir = out_dir / platform
        platform_dir.mkdir(exist_ok=True)

        for config in configs:
            name = config["name"]
            merged = dict(matrix["defaults"])
            merged.update(config["settings"])
            shot = platform_dir / f"{name}.png"
            try:
                send_config(platform, app_uuid, key_ids, merged)
                # pin time AFTER the config message: send-app-message's
                # connection re-syncs the emulator clock to host time, and
                # the time jump fires a tick that redraws the face
                run_pebble(["emu-set-time", str(int(PINNED_TIME.timestamp()))], platform)
                time.sleep(args.settle)
                run_pebble(["screenshot", "--no-open", str(shot)], platform)
                results[(platform, name)] = f"{platform}/{name}.png"
                print(f"  [ok] {platform}/{name}.png")
            except Exception as err:
                failures.append((platform, name, str(err)))
                print(f"  [FAIL] {platform}/{name}: {err}")

    index = write_contact_sheet(out_dir, [c["name"] for c in matrix["configs"]])
    print(f"\n{len(results)} screenshots, {len(failures)} failures")
    print(f"contact sheet: {index}")
    if failures:
        for platform, name, err in failures:
            print(f"  FAILED {platform}/{name}: {err.splitlines()[0] if err else ''}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

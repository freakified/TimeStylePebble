# Settings migration from master

Every non-aplite user upgrading from the appstore is coming from master (7.x); the phone app auto-updates, so no older version needs handling. Aplite keeps master's binary and its settings (see [aplite-legacy.md](aplite-legacy.md)), so it needs no migration at all.

Settings live in two places, and each migrates on its own side:

| Where | Master | This branch | Migrated by |
| --- | --- | --- | --- |
| Watch persist key 100 | 36-byte struct, version 7 at key 4 | `Settings`, version 19 (add-compatible settings append to the struct's tail without bumping it, so a shorter blob loads with the new fields at their defaults) | `migrate_from_master()` in `settings.c` |
| Phone localStorage | `weather_loc` / `_lat` / `_lng` | `location_*` | `migrateFromMaster()` in `location.js` |

Master's config page kept its own state on its own origin, which the new page can't read. It doesn't need to: the new page is prefilled from the watch's config snapshot.

## Watch

Load accepts exactly two stored versions: `CURRENT_SETTINGS_VERSION` loads as-is, and `SETTINGS_VERSION_MASTER` (7) is migrated and immediately re-saved. Anything else is an unreleased development blob and falls back to the defaults.

`MasterSettings` freezes master's struct. arm-none-eabi packs enums into a byte, so master's enum fields are `uint8_t` here; a `_Static_assert` pins the size at 36. Offsets were measured with the SDK toolchain: widgets at 9, `altclockName` at 18, `altclockOffset` at 28, `decimalSeparator` at 34.

| Master | Becomes |
| --- | --- |
| time / background / sidebar colors, language, leading zero, vibes, large fonts, units, restful sleep, decimal separator | the same fields (language ids are identical) |
| `sidebarTextColor` | dropped; text colors are derived now |
| `clockFontId` 2–4 (Avenir bold variants), 1 (LECO) | font id plus `hoursBold` / `minutesBold` (`migrate_clock_font`) |
| `sidebarOnLeft` | `layoutId` |
| `widgets[3]` (top / middle / bottom, gaps allowed) | packed `vRegionSlots`; round puts `[0]` on the left arc (slot 0) and `[2]` on the right (slot `ROUND_ARC_SLOT_CAP`), since master's round face never drew `[1]` |
| widget 1 (bluetooth disconnect) | removed from the slots |
| `activateDisconnectIcon`, or widget 1 in the sidebar | `enableStatusBar` |
| `disableAutobattery` | dropped; there's no mini battery |
| `showBatteryPct == false` | dropped; battery percentage is shown by default and compacted dynamically when needed |
| `healthUseDistance` with a step counter | the step counter becomes `DISTANCE_WALKED` |
| `altclockName`, `altclockOffset` (whole hours) | same name, offset × 60 |

Master's emery font migration flag (key 200) is deleted; its weather cache (key 223) is already deleted by `weather.c`.

Known visual changes: a lone widget in master's bottom slot now sits at the top of the sidebar, and master's custom sidebar text color is replaced by the derived one.

## World time

A migrated world time is a fixed offset with no city behind it. Two pieces make it work:

- The phone always sends `SettingLocalUtcOffset` (deduplicated), not only when a city is stored. The watch shifts world times relative to local time, so without it a migrated clock would be off by the local offset.
- When the config page opens and the phone has no stored city, `worldtime.getConfigSettings` turns the snapshot's offset into an `Etc/GMT∓N` zone (POSIX inverts the sign: `Etc/GMT-9` is UTC+9) and keeps the label. The config page shows saved `Etc/` zones as "UTC+9". Saving then stores the zone like any other. Fixed zones have no DST, the same as master.

## Phone

`location.js` copies master's chosen city into the `location_*` keys the first time this version runs (when `location_use_gps` is unset). An empty `weather_loc` meant GPS, which is also the new default, so nothing is written then. Master's keys stay in place for the aplite legacy build. `weather_datasource`, `weather_api_key` and `disable_weather` are unchanged; a missing `disable_sun` means enabled.

## Testing

Build master in a worktree, install it on an emulator, push settings with libpebble2 using master's `message_keys.json`, then install this branch's PBW over it and request a config snapshot. `pebble kill` before `pebble wipe`: wiping a running emulator doesn't clear persisted storage, and a leftover v18 blob read by master produces garbage that looks like a migration bug.

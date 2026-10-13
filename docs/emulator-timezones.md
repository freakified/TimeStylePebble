# Timezones in the emulator

World time offsets come from the phone's own IANA timezone database via `Intl` (see `src/pkjs/timezone.js`). This works on both the Android and iOS apps. If `Intl` isn't usable, the phone sends world time labels without an offset, and the watch keeps the offset it last had (UTC on a fresh install).

The emulator's JS runtime (pypkjs) ships a broken STPyV8 that never loads its ICU data. Creating any `Intl.DateTimeFormat`, or calling `toLocaleString` with options, kills the whole JS runtime with `Fatal process out of memory: DateTimePatternGeneratorCache::CreateGenerator`. That's a fatal abort, not an exception, so `try/catch` can't guard against it. The app detects the problem safely and skips the offsets.

- Upstream: [cloudflare/stpyv8#125](https://github.com/cloudflare/stpyv8/issues/125) (unfixed, repo archived)
- Pebble: [coredevices/PebbleOS#1579](https://github.com/coredevices/PebbleOS/issues/1579), fixed by [coredevices/pypkjs#5](https://github.com/coredevices/pypkjs/pull/5) (merged July 2026). It still wasn't in a release as of October 2026: PyPI was on pypkjs 2.0.7.

**Is it fixed for you?** Check whether your installed pypkjs is newer than 2.0.7 (`pip show pypkjs`, or look in pebble-tool's environment). If not, set `ICU_DATA` to a folder containing the bundled ICU data under ICU's expected name:

```sh
mkdir -p ~/Library/Application\ Support/pypkjs
cp ~/.local/share/uv/tools/pebble-tool/lib/python3.13/site-packages/stpyv8-icu/icudtl.dat \
   ~/Library/Application\ Support/pypkjs/icudt74l.dat
export ICU_DATA=~/Library/Application\ Support/pypkjs   # e.g. in ~/.zshrc
```

Adjust the paths to match your pebble-tool install and STPyV8's ICU version (`icudt<version>l.dat`). The JS log prints `Timezone source: Intl` when it's working, and `none (offsets not sent)` when it isn't.

The screenshot harness (`tools/screenshot_matrix.py`) isn't affected: it sends world time offsets to the watch directly.

# Aplite legacy build

This version of TimeStyle doesn't fit in aplite's RAM. Instead of dropping aplite, the release PBW includes master's aplite build, with its frozen phone-side JS and its frozen config page. Aplite users get the 7.x watchface for good, from the same appstore listing.

A PBW is a zip with one folder per platform (`aplite/`, `basalt/`, …). Each folder holds that platform's `pebble-app.bin` and `app_resources.pbpack`. Only those folders are per-platform: `pebble-js-app.js`, `appinfo.json` and the config page are shared by every platform. So the hard part isn't the watch binary. It's making sure the shared pieces still talk correctly to the old binary.

## Pieces

| Piece | Aplite | Everything else |
| --- | --- | --- |
| Watch binary | `legacy/aplite` branch (master as of `04ca472`) | this branch |
| Phone JS | `src/pkjs/legacy/` (vendored master pkjs) | `src/pkjs/` |
| Config page | `gh-pages` of the timestyle-legacy repo | `config-page/` → timestyle-config.freakified.net |
| Persisted settings | master's struct, untouched | this branch's struct |

Aplite users keep running master's binary, which reads its own persisted settings. That means aplite needs no settings migration, and the migration pass from master only has to cover the other platforms.

## 1. Shared message keys

Message keys are numbered by their position in `messageKeys`. The two lists have diverged:

- master has 30 keys, this branch has 46
- they first differ at index 8
- this branch removed six keys: `SettingDisableAutobattery`, `SettingDisconnectIcon`, `SettingHealthUseDistance`, `SettingShowBatteryPct`, `SettingSidebarOnLeft`, `SettingSidebarTextColor`

Both binaries and the one JS bundle have to agree on the numbering, so there is one key list: **this branch's `package.json`**. Add the six legacy keys to the end of it. This branch's C code never uses them, but the legacy JS needs them to exist in `message_keys`.

The legacy C code (like master's JS) refers to keys by name only (`MESSAGE_KEY_SettingSidebarOnLeft`). So when the build script copies this branch's key list into the legacy worktree, the legacy binary just needs a rebuild. Nothing in this branch's list has to stay append-only. Reordering keys is fine because the legacy build is regenerated from the list every release.

## 2. Legacy branch

`legacy/aplite` is created from master and otherwise left alone. Bug fixes that should reach aplite go there. The build script makes its changes in a throwaway worktree, so the branch never carries build-time edits.

## 3. Phone JS

Copy master's `src/pkjs/index.js`, `weather.js` and `weather_openmeteo.js` into `src/pkjs/legacy/`, unchanged except for `BASE_CONFIG_URL`, which points at the legacy config repo. They `require('./weather')` by relative path, so they resolve within `legacy/`.

`src/pkjs/index.js` becomes a small dispatcher. The current contents move to `src/pkjs/app.js`:

```js
var platform = 'basalt';
try { platform = Pebble.getActiveWatchInfo().platform; } catch (err) {}

if (platform === 'aplite') {
  require('./legacy/index');
} else {
  require('./app');
}
```

Both modules register their `Pebble.addEventListener` handlers when loaded, so only one set is ever active. The aplite binary sends a bare appmessage to ask for weather, and master's handlers already handle that.

**Verify first:** that `Pebble.getActiveWatchInfo()` returns the right platform when the module loads, on both phone apps and in the emulator. If it doesn't, change the vendored legacy index to export its handlers instead of registering them, and have `index.js` register one set of listeners that picks a handler set per event.

**Shared localStorage:** localStorage belongs to the app UUID on the phone, so someone with both an aplite watch and a newer watch shares it between the two. Both versions use the same keys (`weather_loc`, `weather_loc_lat`, `weather_loc_lng`, `weather_datasource`, `weather_api_key`, `disable_weather`) with compatible meanings, so sharing weather location is fine. New localStorage keys on this branch shouldn't reuse those names for anything else.

## 4. Legacy config page

The freakified/timestyle-legacy repo is a frozen copy of 7.x: `master` holds the source (`v7.11.2` tags the released commit, `04ca472`) and `gh-pages` holds the old config site, served at https://freakified.github.io/timestyle-legacy/. The legacy JS opens `config_aplite.html?appversion=9` there.

The original site at freakified.github.io/TimeStylePebble/ (the main repo's `gh-pages`) has to stay up too: 7.11.2 has that URL built in, so it serves anyone who hasn't updated yet.

## 5. Build script

`tools/build_release.sh` (or `.py`):

1. Check that every key in `git show legacy/aplite:package.json` exists in this branch's `messageKeys`. Fail if any are missing.
2. `git worktree add` a temporary checkout of `legacy/aplite`. Overwrite its `pebble.messageKeys` with this branch's list, and its `version` with this branch's version. Set `targetPlatforms` to `["aplite"]`. Run `pebble build`.
3. Run `pebble build` on this branch.
4. Unzip both PBWs. Copy the legacy `aplite/` folder into the new one. Add `"aplite"` to `targetPlatforms` in the new `appinfo.json`.
5. Zip the result into `Releases/TimeStyle-Release-<version>.pbw` and remove the worktree.

The new build's `pebble-js-app.js` and `appinfo.json` are the only ones shipped. The aplite folder brings its own `manifest.json`, binary and resource pack, so its CRCs stay consistent.

## 6. Testing

- `pebble wipe`, then install the spliced PBW on aplite, basalt and emery emulators.
- Aplite: the face loads, the legacy config page opens, and saving it changes the face (check one setting that uses a legacy-only key, like sidebar on left). Weather arrives.
- Other platforms: no regressions. The new config page still works, and the config snapshot round-trip still works.
- Sideload on a real aplite watch through the phone app, to confirm the phone accepts a PBW whose per-platform binaries were built from different sources. Neither the emulator nor `pebble install` checks this the way the appstore does.

## Open questions

- Does the phone/appstore care that `appinfo.json`'s `resources.media` list doesn't match aplite's resource pack? It's not read at runtime as far as I know, but the real-watch sideload test settles it.
- Should `legacy/aplite` be master exactly as it is now, or a tagged 7.x release?

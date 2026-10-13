/* world time widget support
 *
 * The phone owns the zone choice (stored in localStorage, like the weather
 * location): any IANA zone id, e.g. "America/New_York". The watch only ever
 * sees a display label and the zone's *current* UTC offset in minutes.
 * Recomputing the offset every time we talk to the watch is what keeps the
 * widgets correct across DST changes.
 */

var Timezone = require('./timezone');

var LAST_SENT_KEY = 'world_time_last_sent';

// C-side name buffers are 8 bytes (7 chars + NUL)
var MAX_LABEL_LENGTH = 7;

function storageKey(zoneNum, field) {
  return 'world_time_' + zoneNum + '_' + field;
}

// the label a zone gets until the user types their own: the first three
// letters of its city ("America/New_York" -> "NEW"). Must match
// worldTimeDefaultLabel() in config-page/src/data/worldTimes.ts.
function defaultLabel(tz) {
  return tz.split('/').pop().replace(/[^A-Za-z]/g, '').slice(0, 3).toUpperCase();
}

function addZoneToPayload(zoneNum, nameKey, offsetKey, dict) {
  var tz = window.localStorage.getItem(storageKey(zoneNum, 'zone'));
  if (!tz) {
    return;
  }

  // a blank custom label uses the zone's own (hiding the label is the
  // widget's "Show" setting, not a blank name)
  var label = window.localStorage.getItem(storageKey(zoneNum, 'label'));
  var name = String(label || defaultLabel(tz));
  dict[nameKey] = name.toUpperCase().slice(0, MAX_LABEL_LENGTH);

  // without Intl (the emulator) the offset is left out, and the watch
  // keeps whichever one it last had
  var offset = Timezone.offsetMinutes(tz, new Date());
  if (offset !== null) {
    dict[offsetKey] = offset;
  }
}

// dict of watch settings for the currently-selected zones (empty if the
// user has never configured a world time widget)
function buildWatchPayload() {
  var dict = {};
  addZoneToPayload(1, 'SettingAltClockName', 'SettingAltClockOffset', dict);
  addZoneToPayload(2, 'SettingAltClock2Name', 'SettingAltClock2Offset', dict);

  if (Object.keys(dict).length > 0) {
    // the watch can't reliably determine its own UTC offset (tm_gmtoff's
    // sign convention varies), so send the phone's
    dict.SettingLocalUtcOffset = -new Date().getTimezoneOffset();
  }

  return dict;
}

// remembers the zone selections from a config page result and returns the
// watch payload entries for them
function storeConfig(configData) {
  if (configData.worldTime1Zone !== undefined) {
    window.localStorage.setItem(storageKey(1, 'zone'), configData.worldTime1Zone);
    window.localStorage.setItem(storageKey(1, 'label'), configData.worldTime1Label || '');
  }

  if (configData.worldTime2Zone !== undefined) {
    window.localStorage.setItem(storageKey(2, 'zone'), configData.worldTime2Zone);
    window.localStorage.setItem(storageKey(2, 'label'), configData.worldTime2Label || '');
  }

  return buildWatchPayload();
}

// current selections, for prefilling the config page
function getConfigSettings() {
  var settings = {};

  var zone1 = window.localStorage.getItem(storageKey(1, 'zone'));
  if (zone1) {
    settings.worldTime1Zone = zone1;
    settings.worldTime1Label = window.localStorage.getItem(storageKey(1, 'label')) || '';
  }

  var zone2 = window.localStorage.getItem(storageKey(2, 'zone'));
  if (zone2) {
    settings.worldTime2Zone = zone2;
    settings.worldTime2Label = window.localStorage.getItem(storageKey(2, 'label')) || '';
  }

  return settings;
}

// pushes fresh offsets to the watch if they differ from the last send
// (called on startup and whenever the watch pings us, so DST transitions
// get picked up within half an hour)
function sendOffsetsIfChanged() {
  var dict = buildWatchPayload();
  if (Object.keys(dict).length === 0) {
    return;
  }

  var serialized = JSON.stringify(dict);
  if (window.localStorage.getItem(LAST_SENT_KEY) === serialized) {
    return;
  }

  Pebble.sendAppMessage(dict, function () {
    console.log('Sent world time offsets: ' + serialized);
    window.localStorage.setItem(LAST_SENT_KEY, serialized);
  }, function () {
    console.log('Failed to send world time offsets');
  });
}

module.exports.storeConfig = storeConfig;
module.exports.getConfigSettings = getConfigSettings;
module.exports.sendOffsetsIfChanged = sendOffsetsIfChanged;

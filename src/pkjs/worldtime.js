/* world time widget support
 *
 * The phone owns the city choice (stored in localStorage, like the weather
 * location); the watch only ever sees a display label and the city's
 * *current* UTC offset in minutes. Recomputing the offset every time we
 * talk to the watch is what keeps the widgets correct across DST changes.
 */

var Cities = require('./cities');

var LAST_SENT_KEY = 'world_time_last_sent';

// C-side name buffers are 8 bytes (7 chars + NUL)
var MAX_LABEL_LENGTH = 7;

function storageKey(zoneNum, field) {
  return 'world_time_' + zoneNum + '_' + field;
}

function addZoneToPayload(zoneNum, nameKey, offsetKey, dict) {
  var cityName = window.localStorage.getItem(storageKey(zoneNum, 'city'));
  if (!cityName) {
    return;
  }

  var city = Cities.findCity(cityName);
  if (!city) {
    return;
  }

  // the label is exactly what the user typed; blank means no label
  var name = String(window.localStorage.getItem(storageKey(zoneNum, 'label')) || '');
  dict[nameKey] = name.toUpperCase().slice(0, MAX_LABEL_LENGTH);
  dict[offsetKey] = Cities.cityOffsetMinutes(city, new Date());
}

// dict of watch settings for the currently-selected cities (empty if the
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

// remembers the city selections from a config page result and returns the
// watch payload entries for them
function storeConfig(configData) {
  if (configData.worldTime1City !== undefined) {
    window.localStorage.setItem(storageKey(1, 'city'), configData.worldTime1City);
    window.localStorage.setItem(storageKey(1, 'label'), configData.worldTime1Label || '');
  }

  if (configData.worldTime2City !== undefined) {
    window.localStorage.setItem(storageKey(2, 'city'), configData.worldTime2City);
    window.localStorage.setItem(storageKey(2, 'label'), configData.worldTime2Label || '');
  }

  return buildWatchPayload();
}

// current selections, for prefilling the config page
function getConfigSettings() {
  var settings = {};

  var city1 = window.localStorage.getItem(storageKey(1, 'city'));
  if (city1) {
    settings.worldTime1City = city1;
    settings.worldTime1Label = window.localStorage.getItem(storageKey(1, 'label')) || '';
  }

  var city2 = window.localStorage.getItem(storageKey(2, 'city'));
  if (city2) {
    settings.worldTime2City = city2;
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

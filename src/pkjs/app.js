
var weather = require('./weather');
var settingsMapper = require('./settings');
var worldtime = require('./worldtime');
var userLocation = require('./location');

var USE_LOCAL_CONFIG = true;
var BASE_CONFIG_URL = 'https://timestylepebble.pages.dev/';
var LOCAL_CONFIG_URL = 'http://192.168.11.9:3000/';
var CONFIG_SNAPSHOT_TIMEOUT_MS = 1200;
var SETTINGS_CACHE_KEY = 'watch_settings_cache';
var CACHE_REFRESH_RETRY_MS = 3000;
var CACHE_REFRESH_ATTEMPTS = 3;
var pendingConfigSnapshot = null;
var cacheRefreshTimeoutId = null;

function getWatchInfo() {
  if (Pebble.getActiveWatchInfo) {
    try {
      return Pebble.getActiveWatchInfo();
    } catch (err) {
      return { platform: 'basalt' };
    }
  }

  return { platform: 'aplite' };
}

function openConfigPage(watchInfo, settings) {
  var configUrl = USE_LOCAL_CONFIG ? LOCAL_CONFIG_URL : BASE_CONFIG_URL;
  var url = configUrl
    + '?watchInfo=' + encodeURIComponent(JSON.stringify(watchInfo))
    + '&settings=' + encodeURIComponent(JSON.stringify(settings || {}));

  Pebble.openURL(url);
}

// the watch is the source of truth for settings, but asking it on every
// config page open adds a visible round trip. So the phone keeps a mirror
// of the watch's settings payload, refreshed from a snapshot on every
// launch and updated whenever a config save is acked; the page opens
// straight from the mirror, and only waits on the watch when there's none
function loadCachedWatchSettings() {
  try {
    return JSON.parse(window.localStorage.getItem(SETTINGS_CACHE_KEY));
  } catch (err) {
    return null;
  }
}

function cacheWatchSettings(payload, merge) {
  var cached = (merge && loadCachedWatchSettings()) || {};
  for (var key in payload) {
    cached[key] = payload[key];
  }
  window.localStorage.setItem(SETTINGS_CACHE_KEY, JSON.stringify(cached));
}

// the watch only knows labels and offsets; merge in the world time cities
// and the location, which live on the phone
function configSettingsFromWatchPayload(payload) {
  var settings = settingsMapper.settingsFromWatchPayload(payload);
  var phoneSettings = [worldtime.getConfigSettings(settings), userLocation.getConfigSettings()];
  phoneSettings.forEach(function (extra) {
    for (var key in extra) {
      settings[key] = extra[key];
    }
  });
  return settings;
}

function sendSnapshotRequest(onFailure) {
  Pebble.sendAppMessage({ ConfigSnapshotRequest: 1 }, function () {
    console.log('Requested config snapshot from watch.');
  }, function () {
    console.log('Failed to request config snapshot from watch.');
    if (onFailure) {
      onFailure();
    }
  });
}

function requestConfigSnapshot(watchInfo) {
  if (pendingConfigSnapshot && pendingConfigSnapshot.timeoutId) {
    clearTimeout(pendingConfigSnapshot.timeoutId);
  }

  pendingConfigSnapshot = {
    watchInfo: watchInfo,
    timeoutId: setTimeout(function () {
      pendingConfigSnapshot = null;
      openConfigPage(watchInfo, {});
    }, CONFIG_SNAPSHOT_TIMEOUT_MS)
  };

  sendSnapshotRequest(function () {
    if (pendingConfigSnapshot && pendingConfigSnapshot.timeoutId) {
      clearTimeout(pendingConfigSnapshot.timeoutId);
    }
    pendingConfigSnapshot = null;
    openConfigPage(watchInfo, {});
  });
}

// refreshes the mirror in the background. The watch drops the request if
// its outbox is busy (say, with its startup weather ping), so retry a
// couple of times until a snapshot shows up
function refreshSettingsCache(attemptsLeft) {
  clearTimeout(cacheRefreshTimeoutId);
  if (attemptsLeft <= 0) {
    return;
  }

  sendSnapshotRequest();
  cacheRefreshTimeoutId = setTimeout(function () {
    refreshSettingsCache(attemptsLeft - 1);
  }, CACHE_REFRESH_RETRY_MS);
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function (e) {
    console.log('JS component is now READY');

    userLocation.migrateFromMaster();

    // updateWeather skips this if weather is disabled or still fresh
    weather.updateWeather();

    // refresh world time offsets in case a DST boundary was crossed
    worldtime.sendOffsetsIfChanged();

    refreshSettingsCache(CACHE_REFRESH_ATTEMPTS);
  }
);

// Listen for incoming messages
Pebble.addEventListener('appmessage',
  function (msg) {
    console.log('Recieved message: ' + JSON.stringify(msg.payload));

    if (settingsMapper.isConfigSnapshot(msg.payload)) {
      clearTimeout(cacheRefreshTimeoutId);
      cacheWatchSettings(msg.payload, false);

      if (pendingConfigSnapshot) {
        clearTimeout(pendingConfigSnapshot.timeoutId);
        openConfigPage(pendingConfigSnapshot.watchInfo, configSettingsFromWatchPayload(msg.payload));
        pendingConfigSnapshot = null;
      }
      return;
    }

    weather.updateWeather();
    worldtime.sendOffsetsIfChanged();
  }
);

Pebble.addEventListener('showConfiguration', function (e) {
  var cached = loadCachedWatchSettings();
  if (cached) {
    console.log('Opening config page from cached settings.');
    openConfigPage(getWatchInfo(), configSettingsFromWatchPayload(cached));
  } else {
    requestConfigSnapshot(getWatchInfo());
  }
});

Pebble.addEventListener('webviewclosed', function (e) {
  if (e.response) {
    var configData = JSON.parse(decodeURIComponent(e.response));

    console.log("Config data recieved!" + JSON.stringify(configData));

    var dict = settingsMapper.settingsToWatchPayload(configData);

    // store the world time city choices and translate them into
    // label + current-UTC-offset settings for the watch
    var worldTimeDict = worldtime.storeConfig(configData);
    for (var worldTimeKey in worldTimeDict) {
      dict[worldTimeKey] = worldTimeDict[worldTimeKey];
    }

    // location and weather source configs are not the watch's concern
    userLocation.storeConfig(configData);

    if (configData.weatherDataSource) {
      window.localStorage.setItem('weather_datasource', configData.weatherDataSource);
      window.localStorage.setItem('weather_api_key', configData.weatherApiKey);
    }

    // determine whether the weather and sun times need updating at all
    // every slot the config page can fill counts: the phone doesn't know
    // which platform it's talking to, and the extra slots are only ever
    // EMPTY on watches that don't render them. (The bar's slots are
    // always live too -- holding a widget is what puts the bar on
    // screen, so anything in them renders.)
    var widgetIDs = [
      configData.widget0Id,
      configData.widget1Id,
      configData.widget2Id,
      configData.widget3Id,
      configData.widget4Id,
      configData.widget5Id,
      configData.bottomBarWidget0Id,
      configData.bottomBarWidget1Id,
      configData.bottomBarWidget2Id
    ];

    // weather widgets: 7=current, 8=forecast, 13=UV, 17=rain chance.
    // sunrise/sunset (15) is computed on the phone, so it only needs the
    // location, not the weather service
    var weatherWidgetIDs = [7, 8, 13, 17];
    var hasWeatherWidget = widgetIDs.some(function (id) {
      return weatherWidgetIDs.indexOf(id) != -1;
    });
    var hasSunWidget = widgetIDs.indexOf(15) != -1;

    window.localStorage.setItem('disable_weather', hasWeatherWidget ? 'no' : 'yes');
    window.localStorage.setItem('disable_sun', hasSunWidget ? 'no' : 'yes');

    console.log('Preparing message: ', JSON.stringify(dict));

    // Send settings to Pebble watchapp
    Pebble.sendAppMessage(dict, function () {
      console.log('Sent config data to Pebble, now trying to get weather');

      // only once acked, so the mirror never gets ahead of the watch
      cacheWatchSettings(dict, true);

      // after sending config data, force a weather refresh in case that changed
      weather.updateWeather(true);
    }, function () {
      console.log('Failed to send config data!');
    });
  } else {
    console.log("No settings changed!");
  }

});

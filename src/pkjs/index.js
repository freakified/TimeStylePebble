
var weather = require('./weather');
var settingsMapper = require('./settings');
var worldtime = require('./worldtime');

var CONFIG_VERSION = 9;
var USE_LOCAL_CONFIG = true;
var BASE_CONFIG_URL = 'https://timestylepebble.pages.dev/';
var LOCAL_CONFIG_URL = 'http://192.168.11.9:3000/';
var CONFIG_SNAPSHOT_TIMEOUT_MS = 1200;
var pendingConfigSnapshot = null;

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

  Pebble.sendAppMessage({ ConfigSnapshotRequest: 1 }, function () {
    console.log('Requested config snapshot from watch.');
  }, function () {
    console.log('Failed to request config snapshot from watch.');
    if (pendingConfigSnapshot && pendingConfigSnapshot.timeoutId) {
      clearTimeout(pendingConfigSnapshot.timeoutId);
    }
    pendingConfigSnapshot = null;
    openConfigPage(watchInfo, {});
  });
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function (e) {
    console.log('JS component is now READY');

    // if it has never been started, set the weather to enabled
    if (window.localStorage.getItem('disable_weather') === null) {
      window.localStorage.setItem('disable_weather', 'no');
    }

    console.log('the wdisabled value is: "' + window.localStorage.getItem('disable_weather') + '"');
    // if applicable, get the weather data
    if (window.localStorage.getItem('disable_weather') != 'yes') {
      weather.updateWeather();
    }

    // refresh world time offsets in case a DST boundary was crossed
    worldtime.sendOffsetsIfChanged();
  }
);

// Listen for incoming messages
Pebble.addEventListener('appmessage',
  function (msg) {
    console.log('Recieved message: ' + JSON.stringify(msg.payload));

    if (settingsMapper.isConfigSnapshot(msg.payload)) {
      if (pendingConfigSnapshot) {
        clearTimeout(pendingConfigSnapshot.timeoutId);

        // the watch only knows labels and offsets; merge in the city
        // selections, which live on the phone
        var settings = settingsMapper.settingsFromWatchPayload(msg.payload);
        var worldTimeSettings = worldtime.getConfigSettings();
        for (var key in worldTimeSettings) {
          settings[key] = worldTimeSettings[key];
        }

        openConfigPage(pendingConfigSnapshot.watchInfo, settings);
        pendingConfigSnapshot = null;
      }
      return;
    }

    weather.updateWeather();
    worldtime.sendOffsetsIfChanged();
  }
);

Pebble.addEventListener('showConfiguration', function (e) {
  requestConfigSnapshot(getWatchInfo());
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

    // weather location/source configs are not the watch's concern
    if (configData.weatherMode) {
      window.localStorage.removeItem('weather_loc');
      window.localStorage.removeItem('weather_loc_lat');
      window.localStorage.removeItem('weather_loc_lng');
    } else if (configData.weatherLocation !== undefined) {
      window.localStorage.setItem('weather_loc', configData.weatherLocation);
      window.localStorage.setItem('weather_loc_lat', configData.weatherLocationLat);
      window.localStorage.setItem('weather_loc_lng', configData.weatherLocationLng);
    }

    if (configData.weatherDataSource) {
      window.localStorage.setItem('weather_datasource', configData.weatherDataSource);
      window.localStorage.setItem('weather_api_key', configData.weatherApiKey);
    }

    // determine whether or not the weather checking should be enabled
    var disableWeather;

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

    // if none of the weather widgets are present, disable the weather
    // (7=current, 8=forecast, 13=UV, 15=sunrise/sunset, 16=wind, 17=rain chance)
    var weatherWidgetIDs = [7, 8, 13, 15, 16, 17];
    var hasWeatherWidget = widgetIDs.some(function (id) {
      return weatherWidgetIDs.indexOf(id) != -1;
    });
    disableWeather = hasWeatherWidget ? 'no' : 'yes';

    window.localStorage.setItem('disable_weather', disableWeather);

    console.log('Preparing message: ', JSON.stringify(dict));

    // Send settings to Pebble watchapp
    Pebble.sendAppMessage(dict, function () {
      console.log('Sent config data to Pebble, now trying to get weather');

      // after sending config data, force a weather refresh in case that changed
      weather.updateWeather(true);
    }, function () {
      console.log('Failed to send config data!');
    });
  } else {
    console.log("No settings changed!");
  }

});

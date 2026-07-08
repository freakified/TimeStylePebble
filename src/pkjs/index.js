
var weather = require('./weather');
var settingsMapper = require('./settings');

var CONFIG_VERSION = 9;
var USE_LOCAL_CONFIG = true;
var BASE_CONFIG_URL = 'https://timestylepebble.pages.dev/';
var LOCAL_CONFIG_URL = 'http://10.25.219.9:3000/';
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
  }
);

// Listen for incoming messages
Pebble.addEventListener('appmessage',
  function (msg) {
    console.log('Recieved message: ' + JSON.stringify(msg.payload));

    if (settingsMapper.isConfigSnapshot(msg.payload)) {
      if (pendingConfigSnapshot) {
        clearTimeout(pendingConfigSnapshot.timeoutId);
        openConfigPage(pendingConfigSnapshot.watchInfo, settingsMapper.settingsFromWatchPayload(msg.payload));
        pendingConfigSnapshot = null;
      }
      return;
    }

    weather.updateWeather();
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

    var widgetIDs = [];
    if (configData.layout !== 'bottom-bar-only') {
      widgetIDs.push(
        configData.widget0Id,
        configData.widget1Id,
        configData.widget2Id,
        configData.widget3Id
      );
    }
    if (configData.layout === 'right-sidebar-bottom-bar'
      || configData.layout === 'left-sidebar-bottom-bar'
      || configData.layout === 'bottom-bar-only') {
      widgetIDs.push(
        configData.bottomBarWidget0Id,
        configData.bottomBarWidget1Id,
        configData.bottomBarWidget2Id,
        configData.bottomBarWidget3Id
      );
    }

    // if none of the weather widgets are present, disable the weather
    if (widgetIDs.indexOf(7) != -1 || widgetIDs.indexOf(8) != -1 || widgetIDs.indexOf(13) != -1) {
      disableWeather = 'no';
    } else {
      disableWeather = 'yes';
    }

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

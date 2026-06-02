
var weather = require('./weather');

var CONFIG_VERSION = 9;
var USE_LOCAL_CONFIG = true;
var BASE_CONFIG_URL = 'https://timestylepebble.pages.dev/';
var LOCAL_CONFIG_URL = 'http://localhost:3000/';
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

function padHex(value) {
  var hex = Number(value || 0).toString(16).toUpperCase();
  while (hex.length < 6) {
    hex = '0' + hex;
  }
  return hex;
}

function getNumber(value, fallback) {
  if (value === undefined || value === null) {
    return fallback;
  }
  return Number(value);
}

function getBoolean(value) {
  return getNumber(value, 0) !== 0;
}

function fontFromId(id) {
  if (id === 1) {
    return 'leco';
  } else if (id === 2) {
    return 'bold';
  } else if (id === 3) {
    return 'bold-h';
  } else if (id === 4) {
    return 'bold-m';
  }

  return 'default';
}

function hourlyVibeFromId(id) {
  if (id === 1) {
    return 'everyHour';
  } else if (id === 2) {
    return 'everyHalfHour';
  }

  return 'none';
}

function decimalSeparatorFromPayload(value) {
  if (value === ',') {
    return ',';
  }

  if (Number(value) === 44) {
    return ',';
  }

  return '.';
}

function settingsFromWatchPayload(payload) {
  return {
    colorBackground: padHex(payload.SettingColorBG),
    colorTime: padHex(payload.SettingColorTime),
    colorSidebar: padHex(payload.SettingColorSidebar),
    colorSidebarText: padHex(payload.SettingSidebarTextColor),

    widget0Id: getNumber(payload.SettingWidget0ID, 7),
    widget1Id: getNumber(payload.SettingWidget1ID, 0),
    widget2Id: getNumber(payload.SettingWidget2ID, 4),

    languageId: getNumber(payload.SettingLanguageID, 0),
    leadingZero: getBoolean(payload.SettingShowLeadingZero),
    clockFont: fontFromId(getNumber(payload.SettingClockFontId, 0)),
    sidebarPosition: getBoolean(payload.SettingSidebarOnLeft) ? 'left' : 'right',
    largeSidebarFont: getBoolean(payload.SettingUseLargeFonts),
    decimalSeparator: decimalSeparatorFromPayload(payload.SettingDecimalSep),

    disconnectIcon: getBoolean(payload.SettingDisconnectIcon),
    bluetoothVibe: getBoolean(payload.SettingBluetoothVibe),

    hourlyVibe: hourlyVibeFromId(getNumber(payload.SettingHourlyVibe, 0)),

    weatherUnits: getBoolean(payload.SettingUseMetric) ? 'c' : 'f',

    batteryMeter: getBoolean(payload.SettingShowBatteryPct) ? 'icon-and-percent' : 'icon-only',
    autoBattery: !getBoolean(payload.SettingDisableAutobattery),

    altClockName: payload.SettingAltClockName || 'ALT',
    altClockOffset: getNumber(payload.SettingAltClockOffset, 0),

    healthActivityMetric: getBoolean(payload.SettingHealthUseDistance) ? 'distance' : 'steps',
    healthSleepType: getBoolean(payload.SettingHealthUseRestfulSleep) ? 'restful' : 'total'
  };
}

function isConfigSnapshot(payload) {
  return payload && (
    payload.SettingColorBG !== undefined ||
    payload.SettingWidget0ID !== undefined ||
    payload.SettingClockFontId !== undefined
  );
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

    if (isConfigSnapshot(msg.payload)) {
      if (pendingConfigSnapshot) {
        clearTimeout(pendingConfigSnapshot.timeoutId);
        openConfigPage(pendingConfigSnapshot.watchInfo, settingsFromWatchPayload(msg.payload));
        pendingConfigSnapshot = null;
      }
      return;
    }

    // in the case of recieving this, we assume the watch does, in fact, need weather data
    window.localStorage.setItem('disable_weather', 'no');
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

    // prepare a structure to hold everything we'll send to the watch
    var dict = {};

    // color settings
    if (configData.colorBackground) {
      dict.SettingColorBG = parseInt(configData.colorBackground, 16);
    }

    if (configData.colorSidebar) {
      dict.SettingColorSidebar = parseInt(configData.colorSidebar, 16);
    }

    if (configData.colorTime) {
      dict.SettingColorTime = parseInt(configData.colorTime, 16);
    }

    if (configData.colorSidebarText) {
      dict.SettingSidebarTextColor = parseInt(configData.colorSidebarText, 16);
    }

    // general options
    if (configData.languageId !== undefined) {
      dict.SettingLanguageID = configData.languageId;
    }

    if (configData.leadingZero !== undefined) {
      dict.SettingShowLeadingZero = configData.leadingZero ? 1 : 0;
    }

    if (configData.clockFont) {
      if (configData.clockFont == 'default') {
        dict.SettingClockFontId = 0;
      } else if (configData.clockFont == 'leco') {
        dict.SettingClockFontId = 1;
      } else if (configData.clockFont == 'bold') {
        dict.SettingClockFontId = 2;
      } else if (configData.clockFont == 'bold-h') {
        dict.SettingClockFontId = 3;
      } else if (configData.clockFont == 'bold-m') {
        dict.SettingClockFontId = 4;
      }
    }

    // bluetooth settings
    if (configData.disconnectIcon !== undefined) {
      dict.SettingDisconnectIcon = configData.disconnectIcon ? 1 : 0;
    }

    if (configData.bluetoothVibe !== undefined) {
      dict.SettingBluetoothVibe = configData.bluetoothVibe ? 1 : 0;
    }

    // notification settings
    if (configData.hourlyVibe !== undefined) {
      if (configData.hourlyVibe == 'everyHour') {
        dict.SettingHourlyVibe = 1;
      } else if (configData.hourlyVibe == 'everyHalfHour') {
        dict.SettingHourlyVibe = 2;
      } else {
        dict.SettingHourlyVibe = 0;
      }
    }

    // sidebar settings
    dict.SettingWidget0ID = configData.widget0Id;
    dict.SettingWidget1ID = configData.widget1Id;
    dict.SettingWidget2ID = configData.widget2Id;

    if (configData.sidebarPosition) {
      if (configData.sidebarPosition == 'right') {
        dict.SettingSidebarOnLeft = 0;
      } else {
        dict.SettingSidebarOnLeft = 1;
      }
    }

    if (configData.largeSidebarFont !== undefined) {
      dict.SettingUseLargeFonts = configData.largeSidebarFont ? 1 : 0;
    }

    // weather widget settings
    if (configData.weatherUnits) {
      if (configData.weatherUnits == 'c') {
        dict.SettingUseMetric = 1;
      } else {
        dict.SettingUseMetric = 0;
      }
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

    // battery widget settings
    if (configData.batteryMeter) {
      if (configData.batteryMeter == 'icon-and-percent') {
        dict.SettingShowBatteryPct = 1;
      } else if (configData.batteryMeter == 'icon-only') {
        dict.SettingShowBatteryPct = 0;
      }
    }

    if (configData.autoBattery !== undefined) {
      dict.SettingDisableAutobattery = configData.autoBattery ? 0 : 1;
    }

    if (configData.altClockName) {
      dict.SettingAltClockName = configData.altClockName;
    }

    if (configData.altClockOffset !== null) {
      dict.SettingAltClockOffset = parseInt(configData.altClockOffset, 10);
    }

    if (configData.decimalSeparator) {
      dict.SettingDecimalSep = configData.decimalSeparator;
    }

    if (configData.healthActivityMetric !== undefined) {
      dict.SettingHealthUseDistance = configData.healthActivityMetric == 'distance' ? 1 : 0;
    }

    if (configData.healthSleepType !== undefined) {
      dict.SettingHealthUseRestfulSleep = configData.healthSleepType == 'restful' ? 1 : 0;
    }

    // determine whether or not the weather checking should be enabled
    var disableWeather;

    var widgetIDs = [configData.widget0Id, configData.widget1Id, configData.widget2Id];

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

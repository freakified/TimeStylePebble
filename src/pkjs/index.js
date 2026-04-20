
var weather = require('./weather');

var CONFIG_VERSION = 9;
var USE_LOCAL_CONFIG = true;
var BASE_CONFIG_URL = 'https://timestylepebble.pages.dev/';
var LOCAL_CONFIG_URL = 'http://localhost:3000/';

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
// when one is received, we simply assume that it is a request for new weather data
Pebble.addEventListener('appmessage',
  function (msg) {
    console.log('Recieved message: ' + JSON.stringify(msg.payload));

    // in the case of recieving this, we assume the watch does, in fact, need weather data
    window.localStorage.setItem('disable_weather', 'no');
    weather.updateWeather();
  }
);

Pebble.addEventListener('showConfiguration', function (e) {
  var watchInfo;

  if (Pebble.getActiveWatchInfo) {
    try {
      watchInfo = Pebble.getActiveWatchInfo();
    } catch (err) {
      watchInfo = { platform: 'basalt' };
    }
  } else {
    watchInfo = { platform: 'aplite' };
  }

  var savedSettings = JSON.parse(window.localStorage.getItem('savedSettings') || '{}');
  var configUrl = USE_LOCAL_CONFIG ? LOCAL_CONFIG_URL : BASE_CONFIG_URL;
  var url = configUrl
    + '?watchInfo=' + encodeURIComponent(JSON.stringify(watchInfo))
    + '&settings=' + encodeURIComponent(JSON.stringify(savedSettings));

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function (e) {
  if (e.response) {
    var configData = JSON.parse(decodeURIComponent(e.response));

    console.log("Config data recieved!" + JSON.stringify(configData));

    // persist settings so they're pre-filled next time the config page opens
    window.localStorage.setItem('savedSettings', JSON.stringify(configData));

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

    if (configData.weatherLocation !== undefined) {
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

    if (configData.autoBattery) {
      if (configData.autoBattery == 'on') {
        dict.SettingDisableAutobattery = 0;
      } else if (configData.autoBattery == 'off') {
        dict.SettingDisableAutobattery = 1;
      }
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

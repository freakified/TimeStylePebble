
var weather = require('./weather');

var CONFIG_VERSION = 10;
// var BASE_CONFIG_URL = 'http://localhost:4000/';
var BASE_CONFIG_URL = 'http://freakified.github.io/TimeStylePebble/';

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('JS component is now READY');

    // if it has never been started, set the weather to enabled
    if(window.localStorage.getItem('disable_weather') === null) {
      window.localStorage.setItem('disable_weather', 'no');
    }

    console.log('the wdisabled value is: "' + window.localStorage.getItem('disable_weather') + '"');
    // if applicable, get the weather data
    if(window.localStorage.getItem('disable_weather') != 'yes') {
      weather.updateWeather();
    }
  }
);

// Listen for incoming messages
// when one is received, we simply assume that it is a request for new weather data
Pebble.addEventListener('appmessage',
  function(msg) {
    console.log('Recieved message: ' + JSON.stringify(msg.payload));

    // in the case of recieving this, we assume the watch does, in fact, need weather data
    window.localStorage.setItem('disable_weather', 'no');
    weather.updateWeather();
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  var availabileConfigPlatforms = ['aplite', 'basalt', 'chalk', 'diorite', 'emery'];

  var watchInfo;

  if(Pebble.getActiveWatchInfo) {
    try {
      watchInfo = Pebble.getActiveWatchInfo();
    } catch(err) {
      watchInfo = {
        platform: "basalt"
      };
    }
  } else {
    watchInfo = {
      platform: "aplite"
    };
  }

  // if the reported platform isn't one of the 5 known platforms, just assume it's basalt
  if(availabileConfigPlatforms.indexOf(watchInfo.platform) === -1) {
    watchInfo = {
      platform: "basalt"
    };
  }

  Pebble.openURL(BASE_CONFIG_URL + 'config_' + watchInfo.platform + '.html?appversion=' + CONFIG_VERSION)
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = decodeURIComponent(e.response);

  if(configData) {
    configData = JSON.parse(decodeURIComponent(e.response));

    console.log("Config data recieved!" + JSON.stringify(configData));

    // prepare a structure to hold everything we'll send to the watch
    var dict = {};

    // color settings
    if(configData.color_bg) {
      dict.SettingColorBG = parseInt(configData.color_bg, 16);
    }

    if(configData.color_sidebar) {
      dict.SettingColorSidebar = parseInt(configData.color_sidebar, 16);
    }

    if(configData.color_time) {
      dict.SettingColorTime = parseInt(configData.color_time, 16);
    }

    if(configData.sidebar_text_color) {
      dict.SettingSidebarTextColor = parseInt(configData.sidebar_text_color, 16);
    }

    // general options
    if(configData.language_id !== undefined) {
      dict.SettingLanguageID = configData.language_id;
    }

    if(configData.leading_zero_setting) {
      if(configData.leading_zero_setting == 'yes') {
        dict.SettingShowLeadingZero = 1;
      } else {
        dict.SettingShowLeadingZero = 0;
      }
    }

    if(configData.clock_font_setting) {
      if(configData.clock_font_setting == 'default') {
        dict.SettingClockFontId = 0;
      } else if(configData.clock_font_setting == 'leco') {
        dict.SettingClockFontId = 1;
      } else if(configData.clock_font_setting == 'bold') {
        dict.SettingClockFontId = 2;
      } else if(configData.clock_font_setting == 'bold-h') {
        dict.SettingClockFontId = 3;
      } else if(configData.clock_font_setting == 'bold-m') {
        dict.SettingClockFontId = 4;
      }
    }

    // bluetooth settings
    if(configData.disconnect_icon_setting) {
      if(configData.disconnect_icon_setting == 'yes') {
        dict.SettingDisconnectIcon = 1;
      } else {
        dict.SettingDisconnectIcon = 0;
      }
    }

    if(configData.bluetooth_vibe_setting) {
      if(configData.bluetooth_vibe_setting == 'yes') {
        dict.SettingBluetoothVibe = 1;
      } else {
        dict.SettingBluetoothVibe = 0;
      }
    }

    // notification settings
    if(configData.hourly_vibe_setting) {
      if(configData.hourly_vibe_setting == 'yes') {
        dict.SettingHourlyVibe = 1;
      } else if (configData.hourly_vibe_setting == 'half') {
        dict.SettingHourlyVibe = 2;
      } else {
        dict.SettingHourlyVibe = 0;
      }
    }

    // sidebar settings
    dict.SettingWidget0ID = configData.widget_0_id;
    dict.SettingWidget1ID = configData.widget_1_id;
    dict.SettingWidget2ID = configData.widget_2_id;

    if(configData.sidebar_position) {
      if(configData.sidebar_position == 'right') {
        dict.SettingSidebarOnLeft = 0;
      } else {
        dict.SettingSidebarOnLeft = 1;
      }
    }

    if(configData.use_large_sidebar_font_setting) {
      if(configData.use_large_sidebar_font_setting == 'yes') {
        dict.SettingUseLargeFonts = 1;
      } else {
        dict.SettingUseLargeFonts = 0;
      }
    }

    // weather widget settings
    if(configData.units) {
      if(configData.units == 'c') {
        dict.SettingUseMetric = 1;
      } else {
        dict.SettingUseMetric = 0;
      }
    }

    // weather location/source configs are not the watch's concern

    if(configData.weather_loc !== undefined) {
      window.localStorage.setItem('weather_loc', configData.weather_loc);
      window.localStorage.setItem('weather_loc_lat', configData.weather_loc_lat);
      window.localStorage.setItem('weather_loc_lng', configData.weather_loc_lng);
    }

    if(configData.weather_datasource) {
      window.localStorage.setItem('weather_datasource', configData.weather_datasource);
      window.localStorage.setItem('weather_api_key', configData.weather_api_key);
    }

    // battery widget settings
    if(configData.battery_meter_setting) {
      if(configData.battery_meter_setting == 'icon-and-percent') {
        dict.SettingShowBatteryPct = 1;
      } else if(configData.battery_meter_setting == 'icon-only') {
        dict.SettingShowBatteryPct = 0;
      }
    }

    if(configData.autobattery_setting) {
      if(configData.autobattery_setting == 'on') {
        dict.SettingDisableAutobattery = 0;
      } else if(configData.autobattery_setting == 'off') {
        dict.SettingDisableAutobattery = 1;
      }
    }

    if(configData.altclock_name) {
      dict.SettingAltClockName = configData.altclock_name;
    }

    if(configData.altclock_offset !== null) {
      dict.SettingAltClockOffset = parseInt(configData.altclock_offset, 10);
    }

    if(configData.decimal_separator) {
      dict.SettingDecimalSep = configData.decimal_separator;
    }

    if(configData.health_use_distance) {
      if(configData.health_use_distance == 'yes') {
        dict.SettingHealthUseDistance = 1;
      } else {
        dict.SettingHealthUseDistance = 0;
      }
    }

    // heath settings
    if(configData.health_use_restful_sleep) {
      if(configData.health_use_restful_sleep == 'yes') {
        dict.SettingHealthUseRestfulSleep = 1;
      } else {
        dict.SettingHealthUseRestfulSleep = 0;
      }
    }

    // determine whether or not the weather checking should be enabled
    var disableWeather;

    var widgetIDs = [configData.widget_0_id, configData.widget_1_id, configData.widget_2_id];

    // if none of the weather widgets are present, disable the weather
    if(widgetIDs.indexOf(7) != -1 || widgetIDs.indexOf(8) != -1 || widgetIDs.indexOf(13) != -1) {
        disableWeather = 'no';
    } else {
        disableWeather = 'yes';
    }

    window.localStorage.setItem('disable_weather', disableWeather);

    console.log('Preparing message: ', JSON.stringify(dict));

    // Send settings to Pebble watchapp
    Pebble.sendAppMessage(dict, function(){
      console.log('Sent config data to Pebble, now trying to get weather');

      // after sending config data, force a weather refresh in case that changed
      weather.updateWeather(true);
    }, function() {
        console.log('Failed to send config data!');
    });
  } else {
    console.log("No settings changed!");
  }

});

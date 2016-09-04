
var weather = require('./weather');

var CONFIG_VERSION = 8;
// var BASE_CONFIG_URL = 'http://localhost:4000/';
// var BASE_CONFIG_URL = 'http://192.168.0.108:40000/';
var BASE_CONFIG_URL = 'http://freakified.github.io/TimeStylePebble/';
// var BASE_CONFIG_URL = 'http://192.168.0.106:4000/';

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('JS component is now READY');

    // if it has never been started, set the weather to disabled
    // this is because the weather defaults to "off"
    if(window.localStorage.getItem('disable_weather') === null) {
      window.localStorage.setItem('disable_weather', 'yes');
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
  // var bwConfigURL    = BASE_CONFIG_URL + 'config_bw.html';
  // var colorConfigURL = BASE_CONFIG_URL + 'config_color.html';
  // var roundConfigURL = BASE_CONFIG_URL + 'config_color_round.html';

  // var versionString = '?appversion=' + CONFIG_VERSION;

  // if(Pebble.getActiveWatchInfo) {
  //   try {
  //     watch = Pebble.getActiveWatchInfo();
  //   } catch(err) {
  //     watch = {
  //       platform: "basalt"
  //     };
  //   }
  // } else {
  //   watch = {
  //     platform: "aplite"
  //   };
  // }

  // if(watch.platform == "aplite"){
  //   Pebble.openURL(bwConfigURL + versionString);
  // } else if(watch.platform == "chalk") {
  //   Pebble.openURL(roundConfigURL + versionString);
  // } else {
  //   Pebble.openURL(colorConfigURL + versionString);
  // }

  Pebble.openURL('http://www.example.com');
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
      dict.KEY_SETTING_COLOR_BG = parseInt(configData.color_bg, 16);
    }

    if(configData.color_sidebar) {
      dict.KEY_SETTING_COLOR_SIDEBAR = parseInt(configData.color_sidebar, 16);
    }

    if(configData.color_time) {
      dict.KEY_SETTING_COLOR_TIME = parseInt(configData.color_time, 16);
    }

    if(configData.sidebar_text_color) {
      dict.KEY_SETTING_SIDEBAR_TEXT_COLOR = parseInt(configData.sidebar_text_color, 16);
    }

    // general options
    if(configData.language_id !== undefined) {
      dict.KEY_SETTING_LANGUAGE_ID = configData.language_id;
    }

    if(configData.leading_zero_setting) {
      if(configData.leading_zero_setting == 'yes') {
        dict.KEY_SETTING_SHOW_LEADING_ZERO = 1;
      } else {
        dict.KEY_SETTING_SHOW_LEADING_ZERO = 0;
      }
    }

    if(configData.clock_font_setting) {
      if(configData.clock_font_setting == 'default') {
        dict.KEY_SETTING_CLOCK_FONT_ID = 0;
      } else if(configData.clock_font_setting == 'leco') {
        dict.KEY_SETTING_CLOCK_FONT_ID = 1;
      } else if(configData.clock_font_setting == 'bold') {
        dict.KEY_SETTING_CLOCK_FONT_ID = 2;
      } else if(configData.clock_font_setting == 'bold-h') {
        dict.KEY_SETTING_CLOCK_FONT_ID = 3;
      } else if(configData.clock_font_setting == 'bold-m') {
        dict.KEY_SETTING_CLOCK_FONT_ID = 4;
      }
    }

    // vibration settings
    if(configData.bluetooth_vibe_setting) {
      if(configData.bluetooth_vibe_setting == 'yes') {
        dict.KEY_SETTING_BT_VIBE = 1;
      } else {
        dict.KEY_SETTING_BT_VIBE = 0;
      }
    }

    if(configData.hourly_vibe_setting) {
      if(configData.hourly_vibe_setting == 'yes') {
        dict.KEY_SETTING_HOURLY_VIBE = 1;
      } else if (configData.hourly_vibe_setting == 'half') {
        dict.KEY_SETTING_HOURLY_VIBE = 2;
      } else {
        dict.KEY_SETTING_HOURLY_VIBE = 0;
      }
    }

    // sidebar settings
    dict.KEY_WIDGET_0_ID = configData.widget_0_id;
    dict.KEY_WIDGET_1_ID = configData.widget_1_id;
    dict.KEY_WIDGET_2_ID = configData.widget_2_id;

    if(configData.sidebar_position) {
      if(configData.sidebar_position == 'right') {
        dict.KEY_SETTING_SIDEBAR_LEFT = 0;
      } else {
        dict.KEY_SETTING_SIDEBAR_LEFT = 1;
      }
    }

    if(configData.use_large_sidebar_font_setting) {
      if(configData.use_large_sidebar_font_setting == 'yes') {
        dict.KEY_SETTING_USE_LARGE_FONTS = 1;
      } else {
        dict.KEY_SETTING_USE_LARGE_FONTS = 0;
      }
    }

    // weather widget settings
    if(configData.units) {
      if(configData.units == 'c') {
        dict.KEY_SETTING_USE_METRIC = 1;
      } else {
        dict.KEY_SETTING_USE_METRIC = 0;
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
        dict.KEY_SETTING_SHOW_BATTERY_PCT = 1;
      } else if(configData.battery_meter_setting == 'icon-only') {
        dict.KEY_SETTING_SHOW_BATTERY_PCT = 0;
      }
    }

    if(configData.autobattery_setting) {
      if(configData.autobattery_setting == 'on') {
        dict.KEY_SETTING_DISABLE_AUTOBATTERY = 0;
      } else if(configData.autobattery_setting == 'off') {
        dict.KEY_SETTING_DISABLE_AUTOBATTERY = 1;
      }
    }

    if(configData.altclock_name) {
      dict.KEY_SETTING_ALTCLOCK_NAME = configData.altclock_name;
    }

    if(configData.altclock_offset !== null) {
      dict.KEY_SETTING_ALTCLOCK_OFFSET = parseInt(configData.altclock_offset, 10);
    }

    if(configData.decimal_separator) {
      dict.KEY_SETTING_DECIMAL_SEPARATOR = configData.decimal_separator;
    }

    if(configData.health_use_distance) {
      if(configData.health_use_distance == 'yes') {
        dict.KEY_SETTING_HEALTH_USE_DISTANCE = 1;
      } else {
        dict.KEY_SETTING_HEALTH_USE_DISTANCE = 0;
      }
    }

    // heath settings
    if(configData.health_use_restful_sleep) {
      if(configData.health_use_restful_sleep == 'yes') {
        dict.KEY_SETTING_HEALTH_USE_RESTFUL_SLEEP = 1;
      } else {
        dict.KEY_SETTING_HEALTH_USE_RESTFUL_SLEEP = 0;
      }
    }

    // determine whether or not the weather checking should be enabled
    var disableWeather;

    var widgetIDs = [configData.widget_0_id, configData.widget_1_id, configData.widget_2_id];

    // if there is either a current conditions or a today's forecast widget, enable the weather
    if(widgetIDs.indexOf(7) != -1 || widgetIDs.indexOf(8) != -1) {
        disableWeather = 'no';
    } else {
        disableWeather = 'yes';
    }

    window.localStorage.setItem('disable_weather', disableWeather);

    var enableForecast;

    if(widgetIDs.indexOf(8) != -1) {
      enableForecast = 'yes';
    }

    window.localStorage.setItem('enable_forecast', enableForecast);

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

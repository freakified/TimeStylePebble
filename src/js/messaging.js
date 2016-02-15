var CONFIG_VERSION = 6;
// var BASE_CONFIG_URL = 'http://localhost:4000/';
// var BASE_CONFIG_URL = 'http://192.168.0.108:40000/';
// var BASE_CONFIG_URL = 'http://freakified.github.io/TimeStylePebble/';
var BASE_CONFIG_URL = 'http://192.168.0.106:4000/';

var WEATHER_CACHE_LIFETIME = 86400; // 1 day

var failureRetryAmount = 3;
var currentFailures = 0;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationError(err) {
  console.log('location error on the JS side! Failure #' + currentFailures);
  //if we fail, try using the cached location
  if(currentFailures <= failureRetryAmount) {
    // reset cache time
    window.localStorage.setItem('weather_loc_cache_time', (new Date().getTime() / 1000));

    getWeather();
    currentFailures++;
  } else {
    // until we get too many failures, at which point give up
    currentFailures = 0;
  }
}

function locationSuccess(pos) {
  // Construct URL
  // first, do a reverse geocode lookup on the coordinates

  var query = 'select woeid from geo.places where text="(' +
       pos.coords.latitude + ',' + pos.coords.longitude + ')" limit 1';

  var url = 'https://query.yahooapis.com/v1/public/yql?q=' +
       encodeURIComponent(query) + '&format=json';

  console.log("Reverse geocoding url: " + url);

  xhrRequest(url, 'GET',
    function(responseText) {

      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      var location = json.query.results.place.woeid;

      console.log("We did it! Location was " + location);

      window.localStorage.setItem('weather_loc_cache', location);
      window.localStorage.setItem('weather_loc_cache_time', (new Date().getTime() / 1000));

      getWeather();
    }
  );
}

function getLocation() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function getWeather() {
  var weatherDisabled = window.localStorage.getItem('disable_weather');
  console.log("Get weather function called! DisableWeather is '" + weatherDisabled + "'");
  if(weatherDisabled !== "yes") {
    window.localStorage.setItem('disable_weather', 'no');

    var weatherLoc = window.localStorage.getItem('weather_loc');
    var weatherLocCache = window.localStorage.getItem('weather_loc_cache');
    var weatherLocCacheTime = window.localStorage.getItem('weather_loc_cache_time');
    var cacheAge = (new Date().getTime() / 1000) - weatherLocCacheTime;

    var location = false;
    var is_woeid = false;

    if(weatherLoc) {
      location = weatherLoc;
      console.log("it thinks we have a location");
    } else if ((new Date().getTime() / 1000) - weatherLocCacheTime < WEATHER_CACHE_LIFETIME ) {
      location = weatherLocCache;
      is_woeid = true; //cached locations are woeids
      console.log("it thinks we have a cache! Loc:" + weatherLocCache + ", Age: " + cacheAge);
    }

    if(location !== false) {
      var url = "";
      if(is_woeid === true) {
        var url = 'https://query.yahooapis.com/v1/public/yql?q=' +
            encodeURIComponent('select item.condition, item.forecast from weather.forecast where woeid="' +
            location + '" and u="c" limit 1') + '&format=json';
      } else {
        var url = 'https://query.yahooapis.com/v1/public/yql?q=' +
            encodeURIComponent('select item.condition, item.forecast from weather.forecast where woeid in (select woeid from geo.places(1) where text="' +
            location + '") and u="c" limit 1') + '&format=json';
      }
      console.log(url);

      getAndSendWeatherData(url);
    } else {
      getLocation();
    }
  }
}

// accepts an openweathermap url, gets weather data from it, and sends it to the watch
function getAndSendWeatherData(url) {
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      var condition = json.query.results.channel.item.condition;
      var forecast = json.query.results.channel.item.forecast;

      if(json.query.count == "1") {
        // Temperature in Kelvin requires adjustment
        var temperature = Math.round(condition.temp);
        // console.log('Temperature is ' + temperature);

        // Conditions
        var conditionCode = parseInt(condition.code, 10);
        // console.log('Condition code is ' + conditionCode);

        // forecast conditions
        var forecastCondition = parseInt(forecast.code, 10);
        var forecastHighTemp = Math.round(forecast.high);
        var forecastLowTemp = Math.round(forecast.low);


        // night state is not used with yahoo weather
        var isNight = false;

        // Assemble dictionary using our keys
        var dictionary = {
          'KEY_TEMPERATURE': temperature,
          'KEY_CONDITION_CODE': conditionCode,
          'KEY_USE_NIGHT_ICON': isNight,
          'KEY_FORECAST_CONDITION': forecastCondition,
          'KEY_FORECAST_TEMP_HIGH': forecastHighTemp,
          'KEY_FORECAST_TEMP_LOW': forecastLowTemp
        };

        console.log(JSON.stringify(dictionary));

        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log('Weather info sent to Pebble successfully!');
          },
          function(e) {
            // if we fail, wait a couple seconds, then try again
            if(currentFailures < failureRetryAmount) {
              getWeather();
              currentFailures++;
            } else {
              currentFailures = 0;
            }

            console.log('Error sending weather info to Pebble! Count: #' + currentFailures);
          }
        );
        }

    }
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('JS component is now READY');

    console.log('the wdisabled value is: "' + window.localStorage.getItem('disable_weather') + '"');
    // if applicable, get the weather data
    if(window.localStorage.getItem('disable_weather') != 'yes') {
      getWeather();
    }
  }
);

// Listen for incoming messages
// When we get one, just assume that the watch wants new weather
// data because OF COURSE it wants some weather data!
Pebble.addEventListener('appmessage',
  function(msg) {
    console.log('Recieved message: ' + JSON.stringify(msg.payload));

    getWeather();
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  var bwConfigURL    = BASE_CONFIG_URL + 'config_bw.html';
  var colorConfigURL = BASE_CONFIG_URL + 'config_color.html';
  var roundConfigURL = BASE_CONFIG_URL + 'config_color_round.html';

  var versionString = '?appversion=' + CONFIG_VERSION;

  if(Pebble.getActiveWatchInfo) {
    try {
      watch = Pebble.getActiveWatchInfo();
    } catch(err) {
      watch = {
        platform: "basalt",
      };
    }
  } else {
    watch = {
      platform: "aplite",
    };
  }

  if(watch.platform == "aplite"){
    Pebble.openURL(bwConfigURL + versionString);
  } else if(watch.platform == "chalk") {
    Pebble.openURL(roundConfigURL + versionString);
  } else {
    Pebble.openURL(colorConfigURL + versionString);
  }
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

    if(configData.weather_loc !== undefined) {
      // weather location can be placed into window.localStorage
      window.localStorage.setItem('weather_loc', configData.weather_loc);
    }

    // battery widget settings
    if(configData.battery_meter_setting) {
      if(configData.battery_meter_setting == 'icon-and-percent') {
        dict.KEY_SETTING_SHOW_BATTERY_PCT = 1;
      } else if(configData.battery_meter_setting == 'icon-only') {
        dict.KEY_SETTING_SHOW_BATTERY_PCT = 0;
      }
    }

    if(configData.altclock_name) {
      dict.KEY_SETTING_ALTCLOCK_NAME = configData.altclock_name;
    }

    if(configData.altclock_offset) {
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

    console.log('Preparing message: ', JSON.stringify(dict));

    // Send settings to Pebble watchapp
    Pebble.sendAppMessage(dict, function(){
      console.log('Sent config data to Pebble, now trying to get weather');

      // after sending config data, force a weather refresh in case that changed
      getWeather();
    }, function() {
        console.log('Failed to send config data!');
    });
  } else {
    console.log("No settings changed!");
  }

});

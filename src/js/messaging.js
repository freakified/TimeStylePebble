var CONFIG_VERSION = 8;
// var BASE_CONFIG_URL = 'http://localhost:4000/';
// var BASE_CONFIG_URL = 'http://192.168.0.108:40000/';
var BASE_CONFIG_URL = 'http://freakified.github.io/TimeStylePebble/';
// var BASE_CONFIG_URL = 'http://192.168.0.106:4000/';

// get new forecasts if 3 hours have elapsed
var FORECAST_MAX_AGE = 3 * 60 * 60 * 1000;

// icon codes for sending weather icons to pebble
var CLEAR_DAY           = 0;
var CLEAR_NIGHT         = 1;
var CLOUDY_DAY          = 2;
var HEAVY_RAIN          = 3;
var HEAVY_SNOW          = 4;
var LIGHT_RAIN          = 5;
var LIGHT_SNOW          = 6;
var PARTLY_CLOUDY_NIGHT = 7;
var PARTLY_CLOUDY       = 8;
var RAINING_AND_SNOWING = 9;
var THUNDERSTORM        = 10;
var WEATHER_GENERIC     = 11;

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
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&units=metric&appid=' + OWM_APP_ID;
  console.log(url);

  var forecastURL = 'http://api.openweathermap.org/data/2.5/forecast?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&cnt=8&units=metric&appid=' + OWM_APP_ID;

  getAndSendCurrentWeather(url);
  getForecastIfNeeded(forecastURL);
}

function getLocation() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function getForecastIfNeeded(url) {
  var enableForecast = window.localStorage.getItem('enable_forecast');
  var lastForecastTime = window.localStorage.getItem('last_forecast_time');
  var forecastAge = Date.now() - lastForecastTime;

  console.log("Forecast requested! Age is " + forecastAge);

  if(enableForecast === 'yes' && forecastAge > FORECAST_MAX_AGE) {
    setTimeout(function() { getAndSendWeatherForecast(url); }, 5000);
  }
}

function getWeather() {
  var weatherDisabled = window.localStorage.getItem('disable_weather');

  console.log("Get weather function called! DisableWeather is '" + weatherDisabled + "'");

  if(weatherDisabled !== "yes") {
    window.localStorage.setItem('disable_weather', 'no');

    var weatherLoc = window.localStorage.getItem('weather_loc');

    if(weatherLoc) {
      var url = 'http://api.openweathermap.org/data/2.5/weather?q=' +
          encodeURIComponent(weatherLoc) + '&units=metric&appid=' + OWM_APP_ID;

      var forecastURL = 'http://api.openweathermap.org/data/2.5/forecast?q=' +
          encodeURIComponent(weatherLoc) + '&cnt=8&units=metric&appid=' + OWM_APP_ID;

      getAndSendCurrentWeather(url);
      getForecastIfNeeded(forecastURL);
    } else {
      getLocation();
    }
  }
}

// accepts an openweathermap url, gets weather data from it, and sends it to the watch
function getAndSendCurrentWeather(url) {
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      if(json.cod == "200") {
        var temperature = Math.round(json.main.temp);
        console.log('Temperature is ' + temperature);

        // Conditions
        var conditionCode = json.weather[0].id;
        console.log('Condition code is ' + conditionCode);

        // night state
        var isNight = (json.weather[0].icon.slice(-1) == 'n') ? 1 : 0;

        var iconToLoad = getIconForConditionCode(conditionCode, isNight);

        // Assemble dictionary using our keys
        var dictionary = {
          'KEY_TEMPERATURE': temperature,
          'KEY_CONDITION_CODE': iconToLoad
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
              // call it again somewhere between 3 and 10 seconds
              setTimeout(getWeather, Math.floor(Math.random() * 10000) + 3000);

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

function getAndSendWeatherForecast(url) {
  console.log(url);
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      if(json.cod == "200") {
        var forecast = extractFakeDailyForecast(json);

        console.log('Forecast high/low temps are ' + forecast.highTemp + '/' + forecast.lowTemp);

        // Conditions
        var conditionCode = forecast.condition;
        console.log('Forecast condition is ' + conditionCode);

        var iconToLoad = getIconForConditionCode(conditionCode, false);

        // Assemble dictionary using our keys
        var dictionary = {
          'KEY_FORECAST_CONDITION': iconToLoad,
          'KEY_FORECAST_TEMP_HIGH': forecast.highTemp,
          'KEY_FORECAST_TEMP_LOW': forecast.lowTemp
        };

        console.log(JSON.stringify(dictionary));

        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log('Forecast info sent to Pebble successfully!');
            window.localStorage.setItem('last_forecast_time', Date.now());
          },
          function(e) {
            console.log('Failed to send forecast info! Oh well!');
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
      getWeather();
    }, function() {
        console.log('Failed to send config data!');
    });
  } else {
    console.log("No settings changed!");
  }

});

/*
 Attempts to approximate a daily forecast by interpolating the next
 24 hours worth of 3 hour forecasts :-O
*/
function extractFakeDailyForecast(json) {
  var todaysForecast = {};

  // find the max and min of those temperatures
  todaysForecast.highTemp = -Number.MAX_SAFE_INTEGER;
  todaysForecast.lowTemp  = Number.MAX_SAFE_INTEGER;

  console.log("List length" + json.list.length);

  for(var i = 0; i < json.list.length; i++) {
    if(todaysForecast.highTemp < json.list[i].main.temp_max) {
      todaysForecast.highTemp = json.list[i].main.temp_max;
    }

    if(todaysForecast.lowTemp > json.list[i].main.temp_min) {
      todaysForecast.lowTemp = json.list[i].main.temp_min;
    }
  }

  // we can't really "average" conditions, so we'll just cheat and use...one of them :-O
  todaysForecast.condition = json.list[3].weather[0].id;

  return todaysForecast;
}

function getIconForConditionCode(conditionCode, isNight) {
  var generalCondition = Math.floor(conditionCode / 100);

  // determine the correct icon
  switch(generalCondition) {
    case 2: //thunderstorm
      iconToLoad = THUNDERSTORM;
      break;
    case 3: //drizzle
      iconToLoad = LIGHT_RAIN;
      break;
    case 5: //rain
      if(conditionCode == 500) {
        iconToLoad = LIGHT_RAIN;
      } else if(conditionCode < 505) {
        iconToLoad = HEAVY_RAIN;
      } else if(conditionCode == 511) {
        iconToLoad = RAINING_AND_SNOWING;
      } else {
        iconToLoad = LIGHT_RAIN;
      }
      break;
    case 6: //snow
      if(conditionCode == 600 || conditionCode == 620) {
        iconToLoad = LIGHT_SNOW;
      } else if(conditionCode > 610 && conditionCode < 620) {
        iconToLoad = RAINING_AND_SNOWING;
      } else {
        iconToLoad = HEAVY_SNOW;
      }
      break;
    case 7: // fog, dust, etc
      iconToLoad = CLOUDY_DAY;
      break;
    case 8: // clouds
      if(conditionCode == 800) {
        iconToLoad = (!isNight) ? CLEAR_DAY : CLEAR_NIGHT;
      } else if(conditionCode < 803) {
        iconToLoad = (!isNight) ? PARTLY_CLOUDY : PARTLY_CLOUDY_NIGHT;
      } else {
        iconToLoad = CLOUDY_DAY;
      }
      break;
    default:
      iconToLoad = WEATHER_GENERIC;
      break;
  }

  return iconToLoad;

}

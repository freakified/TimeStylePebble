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
  //if we fail, try again!
  if(currentFailures <= failureRetryAmount) {
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
      pos.coords.latitude + '&lon=' + pos.coords.longitude;

  getAndSendWeatherData(url);
}

function getLocation() {
  navigator.geolocation.watchPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function getWeather() {
  var weatherLoc = localStorage.getItem('weather_loc');

  console.log('Getting Weather! WeatherLoc is: "' + weatherLoc + '"');

  if(weatherLoc) {
    var url = 'http://api.openweathermap.org/data/2.5/weather?q=' +
        encodeURIComponent(weatherLoc) + '&APPID=4bab067e4ab922f0c5dc8a963bcc9d1a';

    getAndSendWeatherData(url);
  } else {
    getLocation();
  }
}

// accepts an openweathermap url, gets weather data from it, and sends it to the watch
function getAndSendWeatherData(url) {
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      if(json.cod == "200") {
        // Temperature in Kelvin requires adjustment
        var temperature = Math.round(json.main.temp - 273.15);
        console.log('Temperature is ' + temperature);

        // Conditions
        var conditions = json.weather[0].id;
        console.log('Condition code is ' + conditions);

        // night state
        var isNight = (json.weather[0].icon.slice(-1) == 'n') ? 1 : 0;

        // Assemble dictionary using our keys
        var dictionary = {
          'KEY_TEMPERATURE': temperature,
          'KEY_CONDITION_CODE': conditions,
          'KEY_USE_NIGHT_ICON': isNight
        };

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

    // first thing to do: get our location
    getWeather();
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
  // Show config page
  // Pebble.openURL('http://freakified.github.io/TimeStylePebble/config_color.html');

  // var colorConfigURL = 'http://192.168.1.123:4000/config_color.html';
  // var bwConfigURL = 'http://192.168.1.123:4000/config_bw.html';
  var colorConfigURL = 'http://freakified.github.io/TimeStylePebble/config_color.html';
  var bwConfigURL = 'http://freakified.github.io/TimeStylePebble/config_bw.html';

  var watch;

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
    Pebble.openURL(bwConfigURL);
  } else {
    Pebble.openURL(colorConfigURL);
  }
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = decodeURIComponent(e.response);

  if(configData) {
    configData = JSON.parse(decodeURIComponent(e.response));

    console.log("Config data recieved!" + JSON.stringify(configData));

    // weather location can be placed into localstorage
    if(configData.weather_loc !== undefined) {
      localStorage.setItem('weather_loc', configData.weather_loc);
    }

    // prepare a structure to hold everything we'll send to the watch
    var dict = {};

    if(configData.color_time) {
      dict.KEY_SETTING_COLOR_TIME = parseInt(configData.color_time, 16);
    }

    if(configData.color_bg) {
      dict.KEY_SETTING_COLOR_BG = parseInt(configData.color_bg, 16);
    }

    if(configData.color_sidebar) {
      dict.KEY_SETTING_COLOR_SIDEBAR = parseInt(configData.color_sidebar, 16);
    }

    if(configData.sidebar_text_color) {
      dict.KEY_SETTING_SIDEBAR_TEXT_COLOR = parseInt(configData.sidebar_text_color, 16);
    }

    if(configData.sidebar_position) {
      if(configData.sidebar_position == 'right') {
        dict.KEY_SETTING_SIDEBAR_RIGHT = 1;
      } else {
        dict.KEY_SETTING_SIDEBAR_RIGHT = 0;
      }
    }

    if(configData.units) {
      if(configData.units == 'c') {
        dict.KEY_SETTING_USE_METRIC = 1;
      } else {
        dict.KEY_SETTING_USE_METRIC = 0;
      }
    }

    if(configData.bluetooth_vibe_setting) {
      if(configData.bluetooth_vibe_setting == 'yes') {
        dict.KEY_SETTING_BT_VIBE = 1;
      } else {
        dict.KEY_SETTING_BT_VIBE = 0;
      }
    }

    if(configData.battery_meter_setting) {
      if(configData.battery_meter_setting == 'yes-with-pct') {
        dict.KEY_SETTING_SHOW_BATTERY_METER = 1;
        dict.KEY_SETTING_SHOW_BATTERY_PCT = 1;
      } else if(configData.battery_meter_setting == 'yes') {
        dict.KEY_SETTING_SHOW_BATTERY_METER = 1;
        dict.KEY_SETTING_SHOW_BATTERY_PCT = 0;
      } else {
        dict.KEY_SETTING_SHOW_BATTERY_METER = 0;
      }
    }

    if(configData.leading_zero_setting) {
      if(configData.leading_zero_setting == 'yes') {
        dict.KEY_SETTING_SHOW_LEADING_ZERO = 1;
      } else {
        dict.KEY_SETTING_SHOW_LEADING_ZERO = 0;
      }
    }

    if(configData.language_id) {
      dict.KEY_SETTING_LANGUAGE_ID = configData.language_id;
    }

    console.log('Preparing message: ', JSON.stringify(dict));

    // Send settings to Pebble watchapp
    Pebble.sendAppMessage(dict, function(){
      console.log('Sent config data to Pebble');

      // after sending config data, force a weather refresh in case that changed
      getWeather();
    }, function() {
        console.log('Failed to send config data!');
    });
  } else {
    console.log("No settings changed!");
  }

});

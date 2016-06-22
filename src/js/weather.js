/* general utility stuff related to weather */

var weatherProviders = {
  'owm'          : require('weather_owm'),
  // 'forecast'     : require('weather_forecast'),
  'wunderground' : require('weather_wunderground')
};

var DEFAULT_WEATHER_PROVIDER = 'owm';

// get new forecasts if 3 hours have elapsed
var FORECAST_MAX_AGE = 3 * 60 * 60 * 1000;
var MAX_FAILURES = 3;
var currentFailures = 0;

// icon codes for sending weather icons to pebble
var WeatherIcons = {
  CLEAR_DAY           : 0,
  CLEAR_NIGHT         : 1,
  CLOUDY_DAY          : 2,
  HEAVY_RAIN          : 3,
  HEAVY_SNOW          : 4,
  LIGHT_RAIN          : 5,
  LIGHT_SNOW          : 6,
  PARTLY_CLOUDY_NIGHT : 7,
  PARTLY_CLOUDY       : 8,
  RAINING_AND_SNOWING : 9,
  THUNDERSTORM        : 10,
  WEATHER_GENERIC     : 11
};


function getCurrentWeatherProvider() {
  var currentWeatherProvider = window.localStorage.getItem('weather_datasource');

  if(weatherProviders[currentWeatherProvider] !== undefined ) {
    return weatherProviders[currentWeatherProvider];
  } else {
    return weatherProviders[DEFAULT_WEATHER_PROVIDER];
  }
}

function updateWeather(forceUpdate) {
  var weatherDisabled = window.localStorage.getItem('disable_weather');

  console.log("Get weather function called! DisableWeather is '" + weatherDisabled + "'");

  // if weather is not disabled...
  if(weatherDisabled !== "yes") {
    // in case "disable_weather" is empty or something weird, set it to "no"
    // since we already know it's not "yes"
    window.localStorage.setItem('disable_weather', 'no');

    var weatherLoc = window.localStorage.getItem('weather_loc');
    var storedLat = window.localStorage.getItem('weather_loc_lat');
    var storedLng = window.localStorage.getItem('weather_loc_lng');

    // console.log("Stored lat: " +  storedLat + ", stored lng: " + storedLng);

    if(weatherLoc) { // do we have a stored location?
      // if so, we should check if we have valid LAT and LNG coords
      hasLocationCoords = (storedLat != undefined && storedLng != undefined)
                       && (storedLat != '' && storedLng != '');
      if(hasLocationCoords) { // do we have valid stored coordinates?
        // if we have valid coords, use them
        var pos = {
                    coords : {
                      latitude : storedLat,
                      longitude : storedLng
                    }
                  };

        getCurrentWeatherProvider().getWeatherFromCoords(pos);

        if(forceUpdate || isForecastNeeded()) {
          getCurrentWeatherProvider().getForecastFromCoords(pos);
        }
      } else {
        // otherwise, use the stored string (legacy, or google was blocked from running)
        getCurrentWeatherProvider().getWeather(weatherLoc);

        if(forceUpdate || isForecastNeeded()) {
          getCurrentWeatherProvider().getForecast(weatherLoc);
        }
      }
    } else {
      // if we don't have a stored location, get the GPS location
      getLocation();
    }
  }
}

function getLocation() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function locationError(err) {
  console.log('location error on the JS side! Failure #' + currentFailures);
  //if we fail, try using the cached location
  if(currentFailures <= MAX_FAILURES) {
    // reset cache time
    window.localStorage.setItem('weather_loc_cache_time', (new Date().getTime() / 1000));

    // try again
    updateWeather();
    currentFailures++;
  } else {
    // until we get too many failures, at which point give up
    currentFailures = 0;
  }
}

function locationSuccess(pos) {
  getCurrentWeatherProvider().getWeatherFromCoords(pos);

  if(isForecastNeeded()) {
    setTimeout(function() { getCurrentWeatherProvider().getForecastFromCoords(pos); }, 5000);
  }
}

function isForecastNeeded() {
  var enableForecast = window.localStorage.getItem('enable_forecast');
  var lastForecastTime = window.localStorage.getItem('last_forecast_time');
  var forecastAge = Date.now() - lastForecastTime;

  console.log("Forecast requested! Age is " + forecastAge);

  if(enableForecast === 'yes' && forecastAge > FORECAST_MAX_AGE) {
    return true;
  } else {
    return false;
  }
}

function sendWeatherToPebble(dictionary) {
  // Send to Pebble
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Weather info sent to Pebble successfully!');
    },
    function(e) {
      // if we fail, wait a couple seconds, then try again
      if(currentFailures < failureRetryAmount) {
        // call it again somewhere between 3 and 10 seconds
        setTimeout(updateWeather, Math.floor(Math.random() * 10000) + 3000);

        currentFailures++;
      } else {
        currentFailures = 0;
      }

      console.log('Error sending weather info to Pebble! Count: #' + currentFailures);
    }
  );
}

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

// the individual weather providers need access to the weather icons
module.exports.icons = WeatherIcons;

// utility functions common to all weather providers
module.exports.xhrRequest = xhrRequest;
module.exports.sendWeatherToPebble = sendWeatherToPebble;

// called by app.js
// updates the weather if needed, respecting all provider settings in localStorage
module.exports.updateWeather = updateWeather;

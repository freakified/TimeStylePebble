/* general utility stuff related to weather */

var weatherProviders = {
  'openmeteo': require('./weather_openmeteo')
};

var DEFAULT_WEATHER_PROVIDER = 'openmeteo';

// how many times a failed location lookup or send is retried before
// giving up until the next scheduled update
var MAX_FAILURES = 3;
var currentFailures = 0;

// the watchface relaunches every time the user leaves an app, and both the
// launch and the watch's hourly ping ask for weather; anything fetched more
// recently than this is still fresh enough, unless the update is forced
var MIN_FETCH_INTERVAL_MS = 15 * 60 * 1000;
var LAST_FETCH_KEY = 'weather_last_fetch';

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

function fetchedRecently() {
  var lastFetch = Number(window.localStorage.getItem(LAST_FETCH_KEY));
  return lastFetch > 0 && (Date.now() - lastFetch) < MIN_FETCH_INTERVAL_MS;
}

function updateWeather(forceUpdate) {
  var weatherDisabled = window.localStorage.getItem('disable_weather');

  console.log("Get weather function called! DisableWeather is '" + weatherDisabled + "'");

  if(weatherDisabled === "yes") {
    return;
  }

  if(!forceUpdate && fetchedRecently()) {
    console.log('Weather is still fresh, skipping update');
    return;
  }

  var weatherLoc = window.localStorage.getItem('weather_loc');

  if(weatherLoc) { // do we have a stored location?
    var storedLat = window.localStorage.getItem('weather_loc_lat');
    var storedLng = window.localStorage.getItem('weather_loc_lng');

    // only use it if it has coordinates
    if(storedLat && storedLng) {
      getCurrentWeatherProvider().getWeatherFromCoords({
        coords: {
          latitude: storedLat,
          longitude: storedLng
        }
      });
    }
  } else {
    // if we don't have a stored location, get the GPS location
    getLocation();
  }
}

// retries a failed update after a few seconds, up to MAX_FAILURES times
function retryAfterFailure(what) {
  if(currentFailures < MAX_FAILURES) {
    currentFailures++;
    console.log(what + ' failed, retry #' + currentFailures);

    // somewhere between 3 and 13 seconds from now
    setTimeout(function () {
      updateWeather(true);
    }, Math.floor(Math.random() * 10000) + 3000);
  } else {
    console.log(what + ' failed, giving up');
    currentFailures = 0;
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
  retryAfterFailure('Location lookup');
}

function locationSuccess(pos) {
  getCurrentWeatherProvider().getWeatherFromCoords(pos);
}

function sendWeatherToPebble(dictionary) {
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Weather info sent to Pebble successfully!');
      currentFailures = 0;
      window.localStorage.setItem(LAST_FETCH_KEY, String(Date.now()));
    },
    function(e) {
      retryAfterFailure('Sending weather to Pebble');
    }
  );
}

// calls back with the response text, or null if the request failed
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.status >= 200 && this.status < 300 ? this.responseText : null);
  };
  xhr.onerror = function () {
    callback(null);
  };
  xhr.open(type, url);
  xhr.send();
};

// the individual weather providers need access to the weather icons
module.exports.icons = WeatherIcons;

// utility functions common to all weather providers
module.exports.xhrRequest = xhrRequest;
module.exports.sendWeatherToPebble = sendWeatherToPebble;
module.exports.retryAfterFailure = retryAfterFailure;

// called by index.js
// updates the weather if needed, respecting all provider settings in localStorage
module.exports.updateWeather = updateWeather;

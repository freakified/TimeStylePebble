/* general utility stuff related to weather */

var userLocation = require('./location');
var SunCalc = require('./suncalc');

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

// today's sunrise/sunset at pos, in minutes after the phone's local
// midnight (the watch's clock), or null where the sun doesn't rise or set
function getSunTimes(pos) {
  var times = SunCalc.getTimes(new Date(), pos.coords.latitude, pos.coords.longitude);
  if (isNaN(times.sunrise) || isNaN(times.sunset)) {
    return null;
  }

  return {
    'WeatherSunriseMinute': times.sunrise.getHours() * 60 + times.sunrise.getMinutes(),
    'WeatherSunsetMinute': times.sunset.getHours() * 60 + times.sunset.getMinutes()
  };
}

// sun times computed for the update in progress, sent along with the weather
var pendingSunTimes = null;

function updateWeather(forceUpdate) {
  var weatherDisabled = window.localStorage.getItem('disable_weather') === 'yes';
  var sunDisabled = window.localStorage.getItem('disable_sun') === 'yes';

  console.log('Get weather function called! Weather disabled: ' + weatherDisabled +
    ', sun disabled: ' + sunDisabled);

  if(weatherDisabled && sunDisabled) {
    return;
  }

  if(!forceUpdate && fetchedRecently()) {
    console.log('Weather is still fresh, skipping update');
    return;
  }

  userLocation.getPosition(function (pos) {
    pendingSunTimes = sunDisabled ? null : getSunTimes(pos);
    console.log('Sun times: ' + JSON.stringify(pendingSunTimes));

    if(weatherDisabled) {
      // sunrise/sunset alone don't need the weather service
      if(pendingSunTimes) {
        sendWeatherToPebble({});
      }
    } else {
      getCurrentWeatherProvider().getWeatherFromCoords(pos);
    }
  }, locationError);
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

function locationError(err) {
  retryAfterFailure('Location lookup');
}

function sendWeatherToPebble(dictionary) {
  for (var key in pendingSunTimes) {
    dictionary[key] = pendingSunTimes[key];
  }

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

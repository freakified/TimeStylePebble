// this contains our offical OWM weather key, hidden from prying eyes
var secrets = require('./secrets');
var weatherCommon = require('./weather');

// "public" functions

module.exports.getWeather = getWeather;
module.exports.getWeatherFromCoords = getWeatherFromCoords;
module.exports.getForecast = getForecast;
module.exports.getForecastFromCoords = getForecastFromCoords;

function getWeather(weatherLoc) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?q=' +
      encodeURIComponent(weatherLoc) + '&units=metric&appid=' + secrets.OWM_APP_ID;

  getAndSendCurrentWeather(url);
}

function getWeatherFromCoords(pos) {
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&units=metric&appid=' + secrets.OWM_APP_ID;
  console.log(url);

  getAndSendCurrentWeather(url);
}

function getForecast(weatherLoc) {
  var forecastURL = 'http://api.openweathermap.org/data/2.5/forecast?q=' +
      encodeURIComponent(weatherLoc) + '&cnt=8&units=metric&appid=' + secrets.OWM_APP_ID;

  getAndSendWeatherForecast(forecastURL);
}

function getForecastFromCoords(pos) {
  var forecastURL = 'http://api.openweathermap.org/data/2.5/forecast?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&cnt=8&units=metric&appid=' + secrets.OWM_APP_ID;

  getAndSendWeatherForecast(forecastURL);
}

// "private" functions

// accepts an openweathermap url, gets weather data from it, and sends it to the watch
function getAndSendCurrentWeather(url) {
  weatherCommon.xhrRequest(url, 'GET',
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
          'WeatherTemperature': temperature,
          'WeatherCondition': iconToLoad
        };

        console.log(JSON.stringify(dictionary));

        weatherCommon.sendWeatherToPebble(dictionary);
      }
  });
}

function getAndSendWeatherForecast(url) {
  console.log(url);
  weatherCommon.xhrRequest(url, 'GET',
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
          'WeatherForecastCondition': iconToLoad,
          'WeatherForecastHighTemp': forecast.highTemp,
          'WeatherForecastLowTemp': forecast.lowTemp
        };

        console.log(JSON.stringify(dictionary));

        weatherCommon.sendWeatherToPebble(dictionary);
    }
  });
}

function getIconForConditionCode(conditionCode, isNight) {
  var generalCondition = Math.floor(conditionCode / 100);

  // determine the correct icon
  switch(generalCondition) {
    case 2: //thunderstorm
      iconToLoad = weatherCommon.icons.THUNDERSTORM;
      break;
    case 3: //drizzle
      iconToLoad = weatherCommon.icons.LIGHT_RAIN;
      break;
    case 5: //rain
      if(conditionCode == 500) {
        iconToLoad = weatherCommon.icons.LIGHT_RAIN;
      } else if(conditionCode < 505) {
        iconToLoad = weatherCommon.icons.HEAVY_RAIN;
      } else if(conditionCode == 511) {
        iconToLoad = weatherCommon.icons.RAINING_AND_SNOWING;
      } else {
        iconToLoad = weatherCommon.icons.LIGHT_RAIN;
      }
      break;
    case 6: //snow
      if(conditionCode == 600 || conditionCode == 620) {
        iconToLoad = weatherCommon.icons.LIGHT_SNOW;
      } else if(conditionCode > 610 && conditionCode < 620) {
        iconToLoad = weatherCommon.icons.RAINING_AND_SNOWING;
      } else {
        iconToLoad = weatherCommon.icons.HEAVY_SNOW;
      }
      break;
    case 7: // fog, dust, etc
      iconToLoad = weatherCommon.icons.CLOUDY_DAY;
      break;
    case 8: // clouds
      if(conditionCode == 800) {
        iconToLoad = (!isNight) ? weatherCommon.icons.CLEAR_DAY : weatherCommon.icons.CLEAR_NIGHT;
      } else if(conditionCode < 803) {
        iconToLoad = (!isNight) ? weatherCommon.icons.PARTLY_CLOUDY : weatherCommon.icons.PARTLY_CLOUDY_NIGHT;
      } else {
        iconToLoad = weatherCommon.icons.CLOUDY_DAY;
      }
      break;
    default:
      iconToLoad = weatherCommon.icons.WEATHER_GENERIC;
      break;
  }

  return iconToLoad;
}

/*
 Attempts to approximate a daily forecast by interpolating the next
 24 hours worth of 3 hour forecasts :-O
*/
function extractFakeDailyForecast(json) {
  var todaysForecast = {};

 // Set the high and low temp to the first interval's values. Avoids returning an invalid number as the temperature.
  todaysForecast.highTemp = json.list[0].main.temp_max;
  todaysForecast.lowTemp  = json.list[0].main.temp_min;

  //Iterates from 1 instead of 0 because we already stored those values
  for(var i = 1; i < json.list.length; i++) {
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

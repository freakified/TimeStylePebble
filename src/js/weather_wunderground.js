var weatherCommon = require('weather');

// "public" functions

module.exports.getWeather = getWeather;
module.exports.getWeatherFromCoords = getWeatherFromCoords;
module.exports.getForecast = getForecast;
module.exports.getForecastFromCoords = getForecastFromCoords;

function getWeather(weatherLoc) {
  var apiKey = window.localStorage.getItem('weather_api_key');

  var url = 'http://api.wunderground.com/api/' + apiKey +
            '/conditions/q/' + encodeURIComponent(weatherLoc) + '.json';

  getAndSendCurrentWeather(url);
}

function getWeatherFromCoords(pos) {
  var apiKey = window.localStorage.getItem('weather_api_key');

  // Construct URL
  var url = 'http://api.wunderground.com/api/' + apiKey +
            '/conditions/q/' + pos.coords.latitude + ',' + pos.coords.longitude + '.json';

  getAndSendCurrentWeather(url);
}

function getForecast(weatherLoc) {
  var apiKey = window.localStorage.getItem('weather_api_key');

  var forecastURL = 'http://api.wunderground.com/api/' + apiKey +
            '/forecast/q/' + encodeURIComponent(weatherLoc) + '.json';

  getAndSendWeatherForecast(forecastURL);
}

function getForecastFromCoords(pos) {
  var apiKey = window.localStorage.getItem('weather_api_key');

  var forecastURL = 'http://api.wunderground.com/api/' + apiKey +
            '/forecast/q/' + pos.coords.latitude + ',' + pos.coords.longitude + '.json';

  getAndSendWeatherForecast(forecastURL);
}

// "private" functions

// accepts a wunderground conditions url, gets weather data from it, and sends it to the watch
function getAndSendCurrentWeather(url) {
  weatherCommon.xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      if(json.response.features.conditions == 1) {
        var temperature = Math.round(json.current_observation.temp_c);
        console.log('Temperature is ' + temperature);

        // Conditions
        var conditionCode = json.current_observation.icon;
        console.log('Condition icon is ' + conditionCode);

        // night state
        var isNight = false;

        if(json.current_observation.icon_url.indexOf('nt_') != -1) {
          isNight = true;
        }

        var iconToLoad = getIconForConditionCode(conditionCode, isNight);
        console.log('were loading this icon:' + iconToLoad);

        // Assemble dictionary using our keys
        var dictionary = {
          'KEY_TEMPERATURE': temperature,
          'KEY_CONDITION_CODE': iconToLoad
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

      if(json.response.features.forecast == 1) {
        var todaysForecast = json.forecast.simpleforecast.forecastday[0];

        var highTemp = parseInt(todaysForecast.high.celsius, 10);
        var lowTemp = parseInt(todaysForecast.low.celsius, 10);

        console.log('Forecast high/low temps are ' + highTemp + '/' + lowTemp);

        // Conditions
        var conditionCode = todaysForecast.icon;
        console.log('Forecast icon is ' + conditionCode);

        var iconToLoad = getIconForConditionCode(conditionCode, false);

        // Assemble dictionary using our keys
        var dictionary = {
          'KEY_FORECAST_CONDITION': iconToLoad,
          'KEY_FORECAST_TEMP_HIGH': highTemp,
          'KEY_FORECAST_TEMP_LOW': lowTemp
        };

        console.log(JSON.stringify(dictionary));

        weatherCommon.sendWeatherToPebble(dictionary);
    }
  });
}

function getIconForConditionCode(conditionCode, isNight) {

  // determine the correct icon
  switch(conditionCode) {
    case 'chanceflurries':
      iconToLoad = weatherCommon.icons.LIGHT_SNOW;
      break;
    case 'chancerain':
      iconToLoad = weatherCommon.icons.LIGHT_RAIN;
      break;
    case 'chancesleet':
      iconToLoad = weatherCommon.icons.RAINING_AND_SNOWING;
      break;
    case 'chancesnow':
      iconToLoad = weatherCommon.icons.LIGHT_SNOW;
      break;
    case 'chancetstorms':
      iconToLoad = weatherCommon.icons.THUNDERSTORM;
      break;
    case 'clear':
      iconToLoad = (!isNight) ? weatherCommon.icons.CLEAR_DAY : weatherCommon.icons.CLEAR_NIGHT;
      break;
    case 'cloudy':
      iconToLoad = weatherCommon.icons.CLOUDY_DAY;
      break;
    case 'flurries':
      iconToLoad = weatherCommon.icons.LIGHT_SNOW;
      break;
    case 'fog':
    case 'hazy':
    case 'mostlycloudy':
      iconToLoad = weatherCommon.icons.CLOUDY_DAY;
      break;
    case 'mostlysunny':
      iconToLoad = (!isNight) ? weatherCommon.icons.CLEAR_DAY : weatherCommon.icons.CLEAR_NIGHT;
      break;
    case 'partlycloudy':
      iconToLoad = (!isNight) ? weatherCommon.icons.PARTLY_CLOUDY : weatherCommon.icons.PARTLY_CLOUDY_NIGHT;
      break;
    case 'partlysunny':
      iconToLoad = (!isNight) ? weatherCommon.icons.PARTLY_CLOUDY : weatherCommon.icons.PARTLY_CLOUDY_NIGHT;
      break;
    case 'sleet':
      iconToLoad = weatherCommon.icons.RAINING_AND_SNOWING;
      break;
    case 'rain':
      iconToLoad = weatherCommon.icons.HEAVY_RAIN;
      break;
    case 'snow':
      iconToLoad = weatherCommon.icons.HEAVY_SNOW;
      break;
    case 'sunny':
      iconToLoad = (!isNight) ? weatherCommon.icons.CLEAR_DAY : weatherCommon.icons.CLEAR_NIGHT;
      break;
    case 'tstorms':
      iconToLoad = weatherCommon.icons.THUNDERSTORM;
      break;
    default:
      iconToLoad = weatherCommon.icons.WEATHER_GENERIC;
      console.log("Warning: No icon found for " + conditionCode);
      break;
  }

  return iconToLoad;
}

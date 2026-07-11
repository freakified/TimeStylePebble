var weatherCommon = require('./weather');

module.exports.getWeatherFromCoords = getWeatherFromCoords;

function getWeatherFromCoords(pos) {
  console.log('getting that weather');
  var lat = pos.coords.latitude;
  var lon = pos.coords.longitude;

  var url = 'https://api.open-meteo.com/v1/forecast?latitude=' +
    lat + '&longitude=' + lon +
    '&current_weather=true' +
    '&daily=temperature_2m_max,temperature_2m_min,uv_index_max,weathercode' +
      ',sunrise,sunset,precipitation_probability_max' +
    '&hourly=uv_index' +
    '&timezone=auto';

  console.log(url);

  getAndSendWeather(url);
}

function getAndSendWeather(url) {
  weatherCommon.xhrRequest(url, 'GET', function(responseText) {
    var json = JSON.parse(responseText);

    // Handle current weather
    var temperature = Math.round(json.current_weather.temperature);
    var conditionCode = json.current_weather.weathercode;
    var isNight = json.current_weather.is_day !== 1;

    console.log('Current temperature is ' + temperature);
    console.log('Current condition code is ' + conditionCode);

    var iconToLoad = getIconForWeatherCode(conditionCode, isNight);

    // Handle forecast
    var forecastHigh = Math.round(json.daily.temperature_2m_max[0]);
    var forecastLow = Math.round(json.daily.temperature_2m_min[0]);
    var forecastCode = json.daily.weathercode[0];

    function getCurrentUVFromHourly(json) {
      var now = new Date();
      var times = json.hourly.time;
      var uvValues = json.hourly.uv_index;

      var closestIndex = 0;
      var smallestDiff = Infinity;

      for (var i = 0; i < times.length; i++) {
        var t = new Date(times[i]);
        var diff = Math.abs(t - now);
        if (diff < smallestDiff) {
          smallestDiff = diff;
          closestIndex = i;
        }
      }
      return Math.round(uvValues[closestIndex]);
    }

    var uvIndex = getCurrentUVFromHourly(json);

    // sunrise/sunset arrive as ISO local time strings like "2026-07-11T05:42";
    // parse the wall-clock time directly to avoid timezone reinterpretation
    function isoToMinutes(isoString) {
      var timePart = String(isoString).split('T')[1];
      if (!timePart) {
        return null;
      }
      var pieces = timePart.split(':');
      return parseInt(pieces[0], 10) * 60 + parseInt(pieces[1], 10);
    }

    var sunriseMinute = isoToMinutes(json.daily.sunrise[0]);
    var sunsetMinute = isoToMinutes(json.daily.sunset[0]);

    var windSpeed = Math.round(json.current_weather.windspeed); // km/h
    var precipChance = null;
    if (json.daily.precipitation_probability_max) {
      precipChance = Math.round(json.daily.precipitation_probability_max[0]);
    }

    console.log('Forecast high/low: ' + forecastHigh + '/' + forecastLow);
    console.log('Forecast condition code: ' + forecastCode);
    console.log('UV Index: ' + uvIndex);
    console.log('Is Night ' + isNight);
    console.log('Wind: ' + windSpeed + 'km/h, precip chance: ' + precipChance +
      '%, sunrise: ' + sunriseMinute + ', sunset: ' + sunsetMinute);

    var forecastIcon = getIconForWeatherCode(forecastCode);

    var dictionary = {
      'WeatherTemperature': temperature,
      'WeatherCondition': iconToLoad,
      'WeatherForecastHighTemp': forecastHigh,
      'WeatherForecastLowTemp': forecastLow,
      'WeatherForecastCondition': forecastIcon,
      'WeatherUVIndex': uvIndex,
      'WeatherWindSpeed': windSpeed
    };

    if (sunriseMinute !== null && sunsetMinute !== null) {
      dictionary.WeatherSunriseMinute = sunriseMinute;
      dictionary.WeatherSunsetMinute = sunsetMinute;
    }

    if (precipChance !== null && !isNaN(precipChance)) {
      dictionary.WeatherPrecipChance = precipChance;
    }

    console.log(JSON.stringify(dictionary));

    weatherCommon.sendWeatherToPebble(dictionary);
  });
}

function getIconForWeatherCode(code, isNight) {
  switch (code) {
    case 0:
        return isNight ? weatherCommon.icons.CLEAR_NIGHT : weatherCommon.icons.CLEAR_DAY;
    case 1:
    case 2:
        return isNight ? weatherCommon.icons.PARTLY_CLOUDY_NIGHT : weatherCommon.icons.PARTLY_CLOUDY;
    case 3:
    case 45:
    case 48:
        return weatherCommon.icons.CLOUDY_DAY;
    case 51:
    case 53:
    case 55:
    case 61:
    case 80:
        return weatherCommon.icons.LIGHT_RAIN;
    case 63:
    case 65:
    case 81:
    case 82:
        return weatherCommon.icons.HEAVY_RAIN;
    case 56:
    case 57:
    case 66:
    case 67:
        return weatherCommon.icons.RAINING_AND_SNOWING;
    case 71:
    case 77:
    case 85:
        return weatherCommon.icons.LIGHT_SNOW;
    case 73:
    case 75:
    case 86:
        return weatherCommon.icons.HEAVY_SNOW;
    case 95:
    case 96:
    case 99:
        return weatherCommon.icons.THUNDERSTORM;
    default:
        return weatherCommon.icons.WEATHER_GENERIC;
  }
}

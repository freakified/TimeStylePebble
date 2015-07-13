var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationError(err) {
  console.log('location error on the JS side :-(' + err.message);
  //if we fail, try again!
  getLocation();
}

function locationSuccess(pos) {
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].id;      
      console.log('Condition code is ' + conditions);
      
      // night state
      var isNight = (json.weather[0].icon.slice(-1) == 'n') ? 1 : 0;
      
      // General Location Data
      var tzOffset = new Date().getTimezoneOffset() * -1;
      
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_LOCATION_LAT': Math.round(pos.coords.latitude * 1000000),
        'KEY_LOCATION_LNG': Math.round(pos.coords.longitude * 1000000),
        'KEY_GMT_OFFSET': tzOffset,
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
          console.log('Error sending weather info to Pebble!');
        }
      );
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

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('i guess the JS component starterd');

    // first thing to do: send the watchface our location
    getLocation();
  }
);
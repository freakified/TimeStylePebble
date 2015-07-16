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
    console.log('JS component is now READY');

    // first thing to do: get our location
    getLocation();
  }
);

// Listen for incoming messages
// When we get one, just assume that the watch wants new weather
// data because OF COURSE it wants some weather data!
Pebble.addEventListener('appmessage',
  function(msg) {
    console.log('Recieved message: ' + JSON.stringify(msg.payload));
    
    getLocation();
  }                     
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
//   Pebble.openURL('http://freakified.github.io/TimeStylePebble/config_color.html');
  Pebble.openURL('http://localhost/~Dan/p2/TimeStylePebble/config_color.html');
});

Pebble.addEventListener('webviewclosed', function(e) {  
  var configData = decodeURIComponent(e.response);
  if(configData) {
    configData = JSON.parse(decodeURIComponent(e.response));
    
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
    
    console.log('Preparing message: ', JSON.stringify(dict));
    
    // Send settings to Pebble watchapp
    Pebble.sendAppMessage(dict, function(){
      console.log('Sent config data to Pebble');  
    }, function() {
        console.log('Failed to send config data!');
    });
  } else {
    console.log("No settings changed!");
  }

});
function locationError(err) {
  console.log('location error on the JS side :-(');
}

function locationSuccess(pos) {
  console.log('location accquired on the JS side!');
  // now that we have the location, get the timezone offset 
  var tzOffset = new Date().getTimezoneOffset() * -1;
  
  // collect everything to send
  var message = {
        'KEY_LOCATION_LAT': Math.round(pos.coords.latitude * 1000000),
        'KEY_LOCATION_LNG': Math.round(pos.coords.longitude * 1000000),
        'KEY_GMT_OFFSET': tzOffset
      };
  
  console.log(message.KEY_LOCATION_LAT);
  
  // send the message to the watch
  Pebble.sendAppMessage(message,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
  console.log('i sent the message!');
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
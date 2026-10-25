/*
 * The location behind weather and sunrise/sunset: either the phone's GPS or
 * a city chosen on the config page. Lives only on the phone; the watch
 * never sees it.
 */

var USE_GPS_KEY = 'location_use_gps';
var NAME_KEY = 'location_name';
var LAT_KEY = 'location_lat';
var LNG_KEY = 'location_lng';

// remembers the location choice from a config page result
function storeConfig(configData) {
  if (configData.useGpsLocation === undefined) {
    return;
  }

  window.localStorage.setItem(USE_GPS_KEY, configData.useGpsLocation ? 'yes' : 'no');
  window.localStorage.setItem(NAME_KEY, configData.locationName || '');
  window.localStorage.setItem(LAT_KEY, String(configData.locationLat));
  window.localStorage.setItem(LNG_KEY, String(configData.locationLng));
}

// current choice, for prefilling the config page
function getConfigSettings() {
  var useGps = window.localStorage.getItem(USE_GPS_KEY);
  if (useGps === null) {
    return {};
  }

  return {
    useGpsLocation: useGps === 'yes',
    locationName: window.localStorage.getItem(NAME_KEY) || '',
    locationLat: Number(window.localStorage.getItem(LAT_KEY)) || 0,
    locationLng: Number(window.localStorage.getItem(LNG_KEY)) || 0
  };
}

// the chosen city's coordinates, or null if GPS should be used (which is
// also the case when GPS is off but no city has been chosen yet)
function getManualCoords() {
  if (window.localStorage.getItem(USE_GPS_KEY) !== 'no' ||
      !window.localStorage.getItem(NAME_KEY)) {
    return null;
  }

  var lat = Number(window.localStorage.getItem(LAT_KEY));
  var lng = Number(window.localStorage.getItem(LNG_KEY));
  if (isNaN(lat) || isNaN(lng)) {
    return null;
  }

  return { latitude: lat, longitude: lng };
}

// calls back with a geolocation-style position ({ coords: { latitude,
// longitude } }), or calls onError if GPS fails
function getPosition(onSuccess, onError) {
  var manual = getManualCoords();
  if (manual) {
    console.log('Using chosen location: ' + manual.latitude + ',' + manual.longitude);
    onSuccess({ coords: manual });
    return;
  }

  navigator.geolocation.getCurrentPosition(
    onSuccess,
    onError,
    {timeout: 15000, maximumAge: 60000}
  );
}

module.exports.storeConfig = storeConfig;
module.exports.getConfigSettings = getConfigSettings;
module.exports.getPosition = getPosition;

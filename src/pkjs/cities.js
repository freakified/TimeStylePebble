/* world time cities
 *
 * Each city names its IANA zone, and the phone's own tz database (via Intl)
 * says what that zone's offset is right now, DST included. `std` is the
 * standard-time offset in minutes, used only when Intl isn't usable (so far
 * just the emulator; see docs/emulator-timezones.md). Shared
 * with pebble-time-traveler, and the names must match
 * config-page/src/data/cities.ts.
 */

var CITIES = [
  { name: "PAGO PAGO",      label: "PPG", tz: "Pacific/Pago_Pago",   std: -660 },
  { name: "HONOLULU",       label: "HNL", tz: "Pacific/Honolulu",    std: -600 },
  { name: "ANCHORAGE",      label: "ANC", tz: "America/Anchorage",   std: -540 },
  { name: "VANCOUVER",      label: "YVR", tz: "America/Vancouver",   std: -480 },
  { name: "SAN FRANCISCO",  label: "SFO", tz: "America/Los_Angeles", std: -480 },
  { name: "EDMONTON",       label: "YEG", tz: "America/Edmonton",    std: -420 },
  { name: "DENVER",         label: "DEN", tz: "America/Denver",      std: -420 },
  { name: "MEXICO CITY",    label: "MEX", tz: "America/Mexico_City", std: -360 },
  { name: "CHICAGO",        label: "CHI", tz: "America/Chicago",     std: -360 },
  { name: "NEW YORK",       label: "NYC", tz: "America/New_York",    std: -300 },
  { name: "SANTIAGO",       label: "SCL", tz: "America/Santiago",    std: -240 },
  { name: "HALIFAX",        label: "YHZ", tz: "America/Halifax",     std: -240 },
  { name: "ST. JOHNS",      label: "YYT", tz: "America/St_Johns",    std: -210 },
  { name: "RIO DE JANEIRO", label: "RIO", tz: "America/Sao_Paulo",   std: -180 },
  { name: "F. DE NORONHA",  label: "FDN", tz: "America/Noronha",     std: -120 },
  { name: "PRAIA",          label: "RAI", tz: "Atlantic/Cape_Verde", std: -60 },
  { name: "UTC",            label: "UTC", tz: "UTC",                 std: 0 },
  { name: "LISBON",         label: "LIS", tz: "Europe/Lisbon",       std: 0 },
  { name: "LONDON",         label: "LON", tz: "Europe/London",       std: 0 },
  { name: "MADRID",         label: "MAD", tz: "Europe/Madrid",       std: 60 },
  { name: "PARIS",          label: "PAR", tz: "Europe/Paris",        std: 60 },
  { name: "ROME",           label: "ROM", tz: "Europe/Rome",         std: 60 },
  { name: "BERLIN",         label: "BER", tz: "Europe/Berlin",       std: 60 },
  { name: "STOCKHOLM",      label: "STO", tz: "Europe/Stockholm",    std: 60 },
  { name: "ATHENS",         label: "ATH", tz: "Europe/Athens",       std: 120 },
  { name: "CAIRO",          label: "CAI", tz: "Africa/Cairo",        std: 120 },
  { name: "JERUSALEM",      label: "JRS", tz: "Asia/Jerusalem",      std: 120 },
  { name: "MOSCOW",         label: "MOW", tz: "Europe/Moscow",       std: 180 },
  { name: "JEDDAH",         label: "JED", tz: "Asia/Riyadh",         std: 180 },
  { name: "TEHRAN",         label: "THR", tz: "Asia/Tehran",         std: 210 },
  { name: "DUBAI",          label: "DXB", tz: "Asia/Dubai",          std: 240 },
  { name: "KABUL",          label: "KBL", tz: "Asia/Kabul",          std: 270 },
  { name: "KARACHI",        label: "KHI", tz: "Asia/Karachi",        std: 300 },
  { name: "DELHI",          label: "DEL", tz: "Asia/Kolkata",        std: 330 },
  { name: "KATHMANDU",      label: "KTM", tz: "Asia/Kathmandu",      std: 345 },
  { name: "DHAKA",          label: "DAC", tz: "Asia/Dhaka",          std: 360 },
  { name: "YANGON",         label: "RGN", tz: "Asia/Yangon",         std: 390 },
  { name: "BANGKOK",        label: "BKK", tz: "Asia/Bangkok",        std: 420 },
  { name: "SINGAPORE",      label: "SIN", tz: "Asia/Singapore",      std: 480 },
  { name: "HONG KONG",      label: "HKG", tz: "Asia/Hong_Kong",      std: 480 },
  { name: "BEIJING",        label: "BJS", tz: "Asia/Shanghai",       std: 480 },
  { name: "TAIPEI",         label: "TPE", tz: "Asia/Taipei",         std: 480 },
  { name: "SEOUL",          label: "SEL", tz: "Asia/Seoul",          std: 540 },
  { name: "TOKYO",          label: "TYO", tz: "Asia/Tokyo",          std: 540 },
  { name: "ADELAIDE",       label: "ADL", tz: "Australia/Adelaide",  std: 570 },
  { name: "GUAM",           label: "GUM", tz: "Pacific/Guam",        std: 600 },
  { name: "SYDNEY",         label: "SYD", tz: "Australia/Sydney",    std: 600 },
  { name: "NOUMEA",         label: "NOU", tz: "Pacific/Noumea",      std: 660 },
  { name: "WELLINGTON",     label: "WLG", tz: "Pacific/Auckland",    std: 720 }
];

// ---- Real IANA timezone data via Intl ----
// The phone apps (JavaScriptCore on iOS, a WebView on Android) expose the OS's
// tz database through Intl. pypkjs's V8 has an Intl object but no ICU locale
// data, and constructing any DateTimeFormat there is a fatal, uncatchable
// crash. supportedLocalesOf is safe to call, so probe with it first.

var intlAvailable = null;
var intlFormatters = {};

function intlFormatter(tz) {
  if (!intlFormatters[tz]) {
    intlFormatters[tz] = new Intl.DateTimeFormat('en-US', {
      timeZone: tz,
      hour12: false,
      year: 'numeric', month: 'numeric', day: 'numeric',
      hour: 'numeric', minute: 'numeric'
    });
  }
  return intlFormatters[tz];
}

function intlOffsetMinutes(tz, now) {
  var p = {};
  intlFormatter(tz).formatToParts(now).forEach(function (part) {
    p[part.type] = part.value;
  });
  // Some engines render midnight as "24" with hour12:false.
  var wallAsUtc = Date.UTC(+p.year, +p.month - 1, +p.day, +p.hour % 24, +p.minute);
  var subMinuteMs = now.getUTCSeconds() * 1000 + now.getUTCMilliseconds();
  return Math.round((wallAsUtc - (now.getTime() - subMinuteMs)) / 60000);
}

function hasIntlTimezones() {
  if (intlAvailable !== null) return intlAvailable;

  intlAvailable = false;
  try {
    if (typeof Intl !== 'undefined' && Intl.DateTimeFormat &&
        Intl.DateTimeFormat.supportedLocalesOf(['en-US']).length > 0) {
      // Sanity check against a known DST zone before trusting the engine.
      intlAvailable =
        intlOffsetMinutes('America/New_York', new Date(Date.UTC(2026, 0, 15, 12))) === -300 &&
        intlOffsetMinutes('America/New_York', new Date(Date.UTC(2026, 6, 15, 12))) === -240;
    }
  } catch (e) {
    console.log('Intl timezone probe failed: ' + e);
  }
  console.log('Timezone source: ' + (intlAvailable ? 'Intl' : 'standard offsets (no DST)'));
  return intlAvailable;
}

function cityOffsetMinutes(city, now) {
  if (hasIntlTimezones()) {
    try {
      return intlOffsetMinutes(city.tz, now);
    } catch (e) {
      console.log('Intl offset failed for ' + city.tz + ': ' + e);
    }
  }
  // No usable Intl (only the emulator so far): standard time, ignoring DST.
  return city.std;
}

function findCity(name) {
  for (var i = 0; i < CITIES.length; i++) {
    if (CITIES[i].name === name) return CITIES[i];
  }
  return null;
}

module.exports = {
  CITIES: CITIES,
  cityOffsetMinutes: cityOffsetMinutes,
  findCity: findCity
};

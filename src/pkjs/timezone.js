/* IANA timezone offsets via Intl
 *
 * The phone apps (JavaScriptCore on iOS, a WebView on Android) expose the
 * OS's tz database through Intl. pypkjs's V8 has an Intl object but no ICU
 * locale data, and constructing any DateTimeFormat there is a fatal,
 * uncatchable crash. supportedLocalesOf is safe to call, so probe with it
 * first. See docs/emulator-timezones.md.
 */

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
  console.log('Timezone source: ' + (intlAvailable ? 'Intl' : 'none (offsets not sent)'));
  return intlAvailable;
}

// a zone's UTC offset in minutes at `now`, or null if it can't be known
// (no usable Intl, which so far means the emulator, or an unknown zone)
function offsetMinutes(tz, now) {
  if (!hasIntlTimezones()) {
    return null;
  }
  try {
    return intlOffsetMinutes(tz, now);
  } catch (e) {
    console.log('Intl offset failed for ' + tz + ': ' + e);
    return null;
  }
}

module.exports.offsetMinutes = offsetMinutes;

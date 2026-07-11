var CITIES = [
  { name: "PAGO PAGO",         label: "PPG",             lat: -14.27, lon: -170.70, std: -660,  dst: null },
  { name: "HONOLULU",          label: "HNL",             lat: 21.31,  lon: -157.86, std: -600,  dst: null },
  { name: "ANCHORAGE",         label: "ANC",             lat: 61.22,  lon: -149.90, std: -540,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "VANCOUVER",         label: "YVR",             lat: 49.28,  lon: -123.12, std: -480,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "SAN FRANCISCO",     label: "SFO",             lat: 37.77,  lon: -122.42, std: -480,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "EDMONTON",          label: "YEG",             lat: 53.54,  lon: -113.49, std: -420,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "DENVER",            label: "DEN",             lat: 39.74,  lon: -104.99, std: -420,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "MEXICO CITY",       label: "MEX",             lat: 19.43,  lon: -99.13,  std: -360,  dst: null },
  { name: "CHICAGO",           label: "CHI",             lat: 41.88,  lon: -87.63,  std: -360,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "NEW YORK",          label: "NYC",             lat: 40.71,  lon: -74.01,  std: -300,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "SANTIAGO",          label: "SCL",             lat: -33.45, lon: -70.67,  std: -240,  dst: { off: 60,  sm: 9, sw: 1, sd: 6, em: 4,  ew: 1, ed: 6 } },
  { name: "HALIFAX",           label: "YHZ",             lat: 44.65,  lon: -63.57,  std: -240,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "ST. JOHNS",         label: "YYT",             lat: 47.56,  lon: -52.71,  std: -210,  dst: { off: 60,  sm: 3, sw: 2, sd: 0, em: 11, ew: 1, ed: 0 } },
  { name: "RIO DE JANEIRO",    label: "RIO",             lat: -22.91, lon: -43.17,  std: -180,  dst: null },
  { name: "F. DE NORONHA",     label: "FDN",             lat: -3.86,  lon: -32.42,  std: -120,  dst: null },
  { name: "PRAIA",             label: "RAI",             lat: 14.92,  lon: -23.51,  std: -60,   dst: null },
  { name: "UTC",               label: "UTC",             lat: 0.00,   lon: 0.00,    std: 0,     dst: null },
  { name: "LISBON",            label: "LIS",             lat: 38.72,  lon: -9.14,   std: 0,     dst: { off: 60,  sm: 3, sw: 0, sd: 0, em: 10, ew: 0, ed: 0 } },
  { name: "LONDON",            label: "LON",             lat: 51.51,  lon: -0.13,   std: 0,     dst: { off: 60,  sm: 3, sw: 0, sd: 0, em: 10, ew: 0, ed: 0 } },
  { name: "MADRID",            label: "MAD",             lat: 40.42,  lon: -3.70,   std: 60,    dst: { off: 60,  sm: 3, sw: 0, sd: 0, em: 10, ew: 0, ed: 0 } },
  { name: "PARIS",             label: "PAR",             lat: 48.86,  lon: 2.35,    std: 60,    dst: { off: 60,  sm: 3, sw: 0, sd: 0, em: 10, ew: 0, ed: 0 } },
  { name: "ROME",              label: "ROM",             lat: 41.90,  lon: 12.50,   std: 60,    dst: { off: 60,  sm: 3, sw: 0, sd: 0, em: 10, ew: 0, ed: 0 } },
  { name: "BERLIN",            label: "BER",             lat: 52.52,  lon: 13.41,   std: 60,    dst: { off: 60,  sm: 3, sw: 0, sd: 0, em: 10, ew: 0, ed: 0 } },
  { name: "STOCKHOLM",         label: "STO",             lat: 59.33,  lon: 18.07,   std: 60,    dst: { off: 60,  sm: 3, sw: 0, sd: 0, em: 10, ew: 0, ed: 0 } },
  { name: "ATHENS",            label: "ATH",             lat: 37.98,  lon: 23.73,   std: 120,   dst: { off: 60,  sm: 3, sw: 0, sd: 0, em: 10, ew: 0, ed: 0 } },
  { name: "CAIRO",             label: "CAI",             lat: 30.04,  lon: 31.24,   std: 120,   dst: null },
  { name: "JERUSALEM",         label: "JRS",             lat: 31.77,  lon: 35.23,   std: 120,   dst: { off: 60,  sm: 3, sw: 0, sd: 5, em: 10, ew: 1, ed: 0 } },
  { name: "MOSCOW",            label: "MOW",             lat: 55.76,  lon: 37.62,   std: 180,   dst: null },
  { name: "JEDDAH",            label: "JED",             lat: 21.49,  lon: 39.19,   std: 180,   dst: null },
  { name: "TEHRAN",            label: "THR",             lat: 35.69,  lon: 51.39,   std: 210,   dst: { off: 60,  sm: 3, sd: 21, em: 9, ed: 21 } },
  { name: "DUBAI",             label: "DXB",             lat: 25.20,  lon: 55.27,   std: 240,   dst: null },
  { name: "KABUL",             label: "KBL",             lat: 34.53,  lon: 69.17,   std: 270,   dst: null },
  { name: "KARACHI",           label: "KHI",             lat: 24.86,  lon: 67.01,   std: 300,   dst: null },
  { name: "DELHI",             label: "DEL",             lat: 28.61,  lon: 77.21,   std: 330,   dst: null },
  { name: "KATHMANDU",         label: "KTM",             lat: 27.72,  lon: 85.32,   std: 345,   dst: null },
  { name: "DHAKA",             label: "DAC",             lat: 23.81,  lon: 90.41,   std: 360,   dst: null },
  { name: "YANGON",            label: "RGN",             lat: 16.87,  lon: 96.20,   std: 390,   dst: null },
  { name: "BANGKOK",           label: "BKK",             lat: 13.76,  lon: 100.50,  std: 420,   dst: null },
  { name: "SINGAPORE",         label: "SIN",             lat: 1.35,   lon: 103.82,  std: 480,   dst: null },
  { name: "HONG KONG",         label: "HKG",             lat: 22.32,  lon: 114.17,  std: 480,   dst: null },
  { name: "BEIJING",           label: "BJS",             lat: 39.90,  lon: 116.40,  std: 480,   dst: null },
  { name: "TAIPEI",            label: "TPE",             lat: 25.03,  lon: 121.57,  std: 480,   dst: null },
  { name: "SEOUL",             label: "SEL",             lat: 37.57,  lon: 126.98,  std: 540,   dst: null },
  { name: "TOKYO",             label: "TYO",             lat: 35.68,  lon: 139.69,  std: 540,   dst: null },
  { name: "ADELAIDE",          label: "ADL",             lat: -34.93, lon: 138.60,  std: 570,   dst: { off: 60,  sm: 10, sw: 1, sd: 0, em: 4, ew: 1, ed: 0 } },
  { name: "GUAM",              label: "GUM",             lat: 13.44,  lon: 144.79,  std: 600,   dst: null },
  { name: "SYDNEY",            label: "SYD",             lat: -33.87, lon: 151.21,  std: 600,   dst: { off: 60,  sm: 10, sw: 1, sd: 0, em: 4, ew: 1, ed: 0 } },
  { name: "NOUMEA",            label: "NOU",             lat: -22.28, lon: 166.46,  std: 660,   dst: null },
  { name: "WELLINGTON",        label: "WLG",             lat: -41.29, lon: 174.78,  std: 720,   dst: { off: 60,  sm: 9, sw: 0, sd: 0, em: 4, ew: 1, ed: 0 } }
];

function nthWeekdayOfMonth(year, month, n, dayOfWeek) {
  if (n === 0) {
    var nextMonth = new Date(Date.UTC(year, month, 1));
    nextMonth.setUTCDate(0);
    var lastDay = nextMonth.getUTCDate();
    var lastDow = nextMonth.getUTCDay();
    var diff = lastDow - dayOfWeek;
    if (diff < 0) diff += 7;
    return lastDay - diff;
  }
  var first = new Date(Date.UTC(year, month - 1, 1));
  var firstDow = first.getUTCDay();
  var diff = dayOfWeek - firstDow;
  if (diff < 0) diff += 7;
  return 1 + diff + (n - 1) * 7;
}

function isDstActive(city, now) {
  var rule = city.dst;
  if (!rule) return false;

  var year = now.getUTCFullYear();

  var startDay, endDay;
  if (rule.sd !== undefined && rule.sw === undefined) {
    startDay = new Date(Date.UTC(year, rule.sm - 1, rule.sd));
    endDay = new Date(Date.UTC(year, rule.em - 1, rule.ed));
  } else {
    startDay = new Date(Date.UTC(year, rule.sm - 1, nthWeekdayOfMonth(year, rule.sm, rule.sw, rule.sd)));
    endDay = new Date(Date.UTC(year, rule.em - 1, nthWeekdayOfMonth(year, rule.em, rule.ew, rule.ed)));
  }

  return now >= startDay && now < endDay;
}

function cityOffsetMinutes(city, now) {
  if (isDstActive(city, now)) {
    return city.std + city.dst.off;
  }
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

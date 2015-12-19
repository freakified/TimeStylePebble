#pragma once

#define LANGUAGE_EN 0
#define LANGUAGE_FR 1
#define LANGUAGE_DE 2
#define LANGUAGE_ES 3
#define LANGUAGE_IT 4
#define LANGUAGE_NL 5
#define LANGUAGE_TR 6
#define LANGUAGE_CZ 7
#define LANGUAGE_PT 8
#define LANGUAGE_GK 9
#define LANGUAGE_SE 10
#define LANGUAGE_PL 11
#define LANGUAGE_SK 12
#define LANGUAGE_VN 13
#define LANGUAGE_RO 14
#define LANGUAGE_CA 15
#define LANGUAGE_NO 16
#define LANGUAGE_RU 17
#define LANGUAGE_EE 18
#define LANGUAGE_EU 19
#define LANGUAGE_FI 20
#define LANGUAGE_DA 21
#define LANGUAGE_LT 22
#define LANGUAGE_SL 23
#define LANGUAGE_HU 24

/* day names in many different languages! */
extern const char dayNames[25][7][8];

/* month names in many different languages! */
extern const char monthNames[25][12][8];

// all of these are taken from:
// http://www.unicode.org/cldr/charts/28/by_type/date_&_time.fields.html#521165cf49647551
extern const char wordForWeek[25][12];

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

/* day names in many different languages! */
char dayNames[10][7][8] = {
	{"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"},
	{"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"},
	{"SO", "MO", "DI", "MI", "DO", "FR", "SA"},
	{"DOM", "LUN", "MAR", "MIÉ", "JUE", "VIE", "SÁB"},
	{"DOM", "LUN", "MAR", "MER", "GIO", "VEN", "SAB"},
	{"ZO", "MA", "DI", "WO", "DO", "VR", "ZA"},
	{"PAZ", "PTS", "SAL", "ÇAR", "PER", "CUM", "CTS"},
	{"NE", "PO", "ÚT", "ST", "ČT", "PÁ", "SO"},
	{"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SÁB"},
	{"ΚΥΡ", "ΔΕΥ", "ΤΡΙ", "ΤΕΤ", "ΠΕΜ", "ΠΑΡ", "ΣΑΒ"}
	{"SÖN", "MÅN", "TI", "ONS", "TO", "FRE", "LÖR"}
};

/* month names in many different languages! */
char monthNames[10][12][8] = {
	{"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"},
	{"JAN", "FÉV", "MAR", "AVR", "MAI", "JUI", "JUL", "AOÛ", "SEP", "OCT", "NOV", "DÉC"},
	{"JAN", "FEB", "MÄR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ"},
	{"ENE", "FEB", "MAR", "ABR", "MAY", "JUN", "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"},
	{"GEN", "FEB", "MAR", "APR", "MAG", "GIU", "LUG", "AGO", "SET", "OTT", "NOV", "DIC"},
	{"JAN", "FEB", "MRT", "APR", "MEI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEC"},
	{"OCA", "ŞUB", "MAR", "NİS", "MAY", "HAZ", "TEM", "AĞU", "EYL", "EKİ", "KAS", "ARA"},
	{"LED", "ÚNO", "BŘE", "DUB", "KVĚ", "ČRV", "ČVC", "SRP", "ZÁŘ", "ŘÍJ", "LIS", "PRO"},
	{"JAN", "FEV", "MAR", "ABR", "MAI", "JUN", "JUL", "AGO", "SET", "OUT", "NOV", "DEZ"},
	{"ΙΑΝ", "ΦΕΒ", "ΜΑΡ", "ΑΠΡ", "ΜΑΪ", "ΙΟΝ", "ΙΟΛ", "ΑΥΓ", "ΣΕΠ", "ΟΚΤ", "ΝΟΕ", "ΔΕΚ"}
	{"JAN", "FEB", "MAR", "APR", "MAJ", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEC"}
};

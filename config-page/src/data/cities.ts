// World time city list. Must stay in sync with src/pkjs/cities.js, which
// owns the DST rules and computes each city's current UTC offset whenever
// the phone talks to the watch. `std` (standard-time offset from UTC, in
// minutes) is only used here to sort and annotate the dropdown.

export interface WorldTimeCity {
    name: string;   // stable identifier, matches pkjs cities.js
    label: string;  // default 3-letter label shown on the watch
    std: number;    // standard-time UTC offset in minutes (display only)
    hasDst: boolean;
}

export const WORLD_TIME_CITIES: WorldTimeCity[] = [
    { name: 'PAGO PAGO',      label: 'PPG', std: -660, hasDst: false },
    { name: 'HONOLULU',       label: 'HNL', std: -600, hasDst: false },
    { name: 'ANCHORAGE',      label: 'ANC', std: -540, hasDst: true },
    { name: 'VANCOUVER',      label: 'YVR', std: -480, hasDst: true },
    { name: 'SAN FRANCISCO',  label: 'SFO', std: -480, hasDst: true },
    { name: 'EDMONTON',       label: 'YEG', std: -420, hasDst: true },
    { name: 'DENVER',         label: 'DEN', std: -420, hasDst: true },
    { name: 'MEXICO CITY',    label: 'MEX', std: -360, hasDst: false },
    { name: 'CHICAGO',        label: 'CHI', std: -360, hasDst: true },
    { name: 'NEW YORK',       label: 'NYC', std: -300, hasDst: true },
    { name: 'SANTIAGO',       label: 'SCL', std: -240, hasDst: true },
    { name: 'HALIFAX',        label: 'YHZ', std: -240, hasDst: true },
    { name: 'ST. JOHNS',      label: 'YYT', std: -210, hasDst: true },
    { name: 'RIO DE JANEIRO', label: 'RIO', std: -180, hasDst: false },
    { name: 'F. DE NORONHA',  label: 'FDN', std: -120, hasDst: false },
    { name: 'PRAIA',          label: 'RAI', std: -60,  hasDst: false },
    { name: 'UTC',            label: 'UTC', std: 0,    hasDst: false },
    { name: 'LISBON',         label: 'LIS', std: 0,    hasDst: true },
    { name: 'LONDON',         label: 'LON', std: 0,    hasDst: true },
    { name: 'MADRID',         label: 'MAD', std: 60,   hasDst: true },
    { name: 'PARIS',          label: 'PAR', std: 60,   hasDst: true },
    { name: 'ROME',           label: 'ROM', std: 60,   hasDst: true },
    { name: 'BERLIN',         label: 'BER', std: 60,   hasDst: true },
    { name: 'STOCKHOLM',      label: 'STO', std: 60,   hasDst: true },
    { name: 'ATHENS',         label: 'ATH', std: 120,  hasDst: true },
    { name: 'CAIRO',          label: 'CAI', std: 120,  hasDst: false },
    { name: 'JERUSALEM',      label: 'JRS', std: 120,  hasDst: true },
    { name: 'MOSCOW',         label: 'MOW', std: 180,  hasDst: false },
    { name: 'JEDDAH',         label: 'JED', std: 180,  hasDst: false },
    { name: 'TEHRAN',         label: 'THR', std: 210,  hasDst: true },
    { name: 'DUBAI',          label: 'DXB', std: 240,  hasDst: false },
    { name: 'KABUL',          label: 'KBL', std: 270,  hasDst: false },
    { name: 'KARACHI',        label: 'KHI', std: 300,  hasDst: false },
    { name: 'DELHI',          label: 'DEL', std: 330,  hasDst: false },
    { name: 'KATHMANDU',      label: 'KTM', std: 345,  hasDst: false },
    { name: 'DHAKA',          label: 'DAC', std: 360,  hasDst: false },
    { name: 'YANGON',         label: 'RGN', std: 390,  hasDst: false },
    { name: 'BANGKOK',        label: 'BKK', std: 420,  hasDst: false },
    { name: 'SINGAPORE',      label: 'SIN', std: 480,  hasDst: false },
    { name: 'HONG KONG',      label: 'HKG', std: 480,  hasDst: false },
    { name: 'BEIJING',        label: 'BJS', std: 480,  hasDst: false },
    { name: 'TAIPEI',         label: 'TPE', std: 480,  hasDst: false },
    { name: 'SEOUL',          label: 'SEL', std: 540,  hasDst: false },
    { name: 'TOKYO',          label: 'TYO', std: 540,  hasDst: false },
    { name: 'ADELAIDE',       label: 'ADL', std: 570,  hasDst: true },
    { name: 'GUAM',           label: 'GUM', std: 600,  hasDst: false },
    { name: 'SYDNEY',         label: 'SYD', std: 600,  hasDst: true },
    { name: 'NOUMEA',         label: 'NOU', std: 660,  hasDst: false },
    { name: 'WELLINGTON',     label: 'WLG', std: 720,  hasDst: true },
];

const formatOffset = (minutes: number): string => {
    const sign = minutes < 0 ? '-' : '+';
    const abs = Math.abs(minutes);
    const hours = Math.floor(abs / 60);
    const mins = abs % 60;
    return `UTC${sign}${hours}${mins ? ':' + String(mins).padStart(2, '0') : ''}`;
};

export const WORLD_TIME_CITY_OPTIONS = WORLD_TIME_CITIES.map((city) => ({
    label: `${city.name} (${formatOffset(city.std)}${city.hasDst ? ', DST' : ''})`,
    value: city.name,
}));

export const findWorldTimeCity = (name: string): WorldTimeCity | undefined =>
    WORLD_TIME_CITIES.find((city) => city.name === name);

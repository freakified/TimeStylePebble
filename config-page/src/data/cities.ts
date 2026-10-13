import { offsetMinutes } from '../utils/timezone';

// World time city list. Must stay in sync with src/pkjs/cities.js, which
// computes each city's current UTC offset from its IANA zone whenever the
// phone talks to the watch. `std` (standard-time offset from UTC, in
// minutes) is the order the dropdown lists them in.

export interface WorldTimeCity {
    name: string;   // stable identifier, matches pkjs cities.js
    label: string;  // default 3-letter label shown on the watch
    tz: string;     // IANA zone
    std: number;    // standard-time UTC offset in minutes
}

export const WORLD_TIME_CITIES: WorldTimeCity[] = [
    { name: 'PAGO PAGO',     label: 'PPG', tz: 'Pacific/Pago_Pago',   std: -660 },
    { name: 'HONOLULU',      label: 'HNL', tz: 'Pacific/Honolulu',    std: -600 },
    { name: 'ANCHORAGE',     label: 'ANC', tz: 'America/Anchorage',   std: -540 },
    { name: 'VANCOUVER',     label: 'YVR', tz: 'America/Vancouver',   std: -480 },
    { name: 'SAN FRANCISCO', label: 'SFO', tz: 'America/Los_Angeles', std: -480 },
    { name: 'EDMONTON',      label: 'YEG', tz: 'America/Edmonton',    std: -420 },
    { name: 'DENVER',        label: 'DEN', tz: 'America/Denver',      std: -420 },
    { name: 'MEXICO CITY',   label: 'MEX', tz: 'America/Mexico_City', std: -360 },
    { name: 'CHICAGO',       label: 'CHI', tz: 'America/Chicago',     std: -360 },
    { name: 'NEW YORK',      label: 'NYC', tz: 'America/New_York',    std: -300 },
    { name: 'SANTIAGO',      label: 'SCL', tz: 'America/Santiago',    std: -240 },
    { name: 'HALIFAX',       label: 'YHZ', tz: 'America/Halifax',     std: -240 },
    { name: 'ST. JOHNS',     label: 'YYT', tz: 'America/St_Johns',    std: -210 },
    { name: 'RIO DE JANEIRO',label: 'RIO', tz: 'America/Sao_Paulo',   std: -180 },
    { name: 'F. DE NORONHA', label: 'FDN', tz: 'America/Noronha',     std: -120 },
    { name: 'PRAIA',         label: 'RAI', tz: 'Atlantic/Cape_Verde', std: -60 },
    { name: 'UTC',           label: 'UTC', tz: 'UTC',                 std: 0 },
    { name: 'LISBON',        label: 'LIS', tz: 'Europe/Lisbon',       std: 0 },
    { name: 'LONDON',        label: 'LON', tz: 'Europe/London',       std: 0 },
    { name: 'MADRID',        label: 'MAD', tz: 'Europe/Madrid',       std: 60 },
    { name: 'PARIS',         label: 'PAR', tz: 'Europe/Paris',        std: 60 },
    { name: 'ROME',          label: 'ROM', tz: 'Europe/Rome',         std: 60 },
    { name: 'BERLIN',        label: 'BER', tz: 'Europe/Berlin',       std: 60 },
    { name: 'STOCKHOLM',     label: 'STO', tz: 'Europe/Stockholm',    std: 60 },
    { name: 'ATHENS',        label: 'ATH', tz: 'Europe/Athens',       std: 120 },
    { name: 'CAIRO',         label: 'CAI', tz: 'Africa/Cairo',        std: 120 },
    { name: 'JERUSALEM',     label: 'JRS', tz: 'Asia/Jerusalem',      std: 120 },
    { name: 'MOSCOW',        label: 'MOW', tz: 'Europe/Moscow',       std: 180 },
    { name: 'JEDDAH',        label: 'JED', tz: 'Asia/Riyadh',         std: 180 },
    { name: 'TEHRAN',        label: 'THR', tz: 'Asia/Tehran',         std: 210 },
    { name: 'DUBAI',         label: 'DXB', tz: 'Asia/Dubai',          std: 240 },
    { name: 'KABUL',         label: 'KBL', tz: 'Asia/Kabul',          std: 270 },
    { name: 'KARACHI',       label: 'KHI', tz: 'Asia/Karachi',        std: 300 },
    { name: 'DELHI',         label: 'DEL', tz: 'Asia/Kolkata',        std: 330 },
    { name: 'KATHMANDU',     label: 'KTM', tz: 'Asia/Kathmandu',      std: 345 },
    { name: 'DHAKA',         label: 'DAC', tz: 'Asia/Dhaka',          std: 360 },
    { name: 'YANGON',        label: 'RGN', tz: 'Asia/Yangon',         std: 390 },
    { name: 'BANGKOK',       label: 'BKK', tz: 'Asia/Bangkok',        std: 420 },
    { name: 'SINGAPORE',     label: 'SIN', tz: 'Asia/Singapore',      std: 480 },
    { name: 'HONG KONG',     label: 'HKG', tz: 'Asia/Hong_Kong',      std: 480 },
    { name: 'BEIJING',       label: 'BJS', tz: 'Asia/Shanghai',       std: 480 },
    { name: 'TAIPEI',        label: 'TPE', tz: 'Asia/Taipei',         std: 480 },
    { name: 'SEOUL',         label: 'SEL', tz: 'Asia/Seoul',          std: 540 },
    { name: 'TOKYO',         label: 'TYO', tz: 'Asia/Tokyo',          std: 540 },
    { name: 'ADELAIDE',      label: 'ADL', tz: 'Australia/Adelaide',  std: 570 },
    { name: 'GUAM',          label: 'GUM', tz: 'Pacific/Guam',        std: 600 },
    { name: 'SYDNEY',        label: 'SYD', tz: 'Australia/Sydney',    std: 600 },
    { name: 'NOUMEA',        label: 'NOU', tz: 'Pacific/Noumea',      std: 660 },
    { name: 'WELLINGTON',    label: 'WLG', tz: 'Pacific/Auckland',    std: 720 },
];

const formatOffset = (minutes: number): string => {
    const sign = minutes < 0 ? '-' : '+';
    const abs = Math.abs(minutes);
    const hours = Math.floor(abs / 60);
    const mins = abs % 60;
    return `UTC${sign}${hours}${mins ? ':' + String(mins).padStart(2, '0') : ''}`;
};

// labelled with each city's offset right now, so a city in DST says so
export const WORLD_TIME_CITY_OPTIONS = WORLD_TIME_CITIES.map((city) => ({
    label: `${city.name} (${formatOffset(offsetMinutes(city.tz, city.std))})`,
    value: city.name,
}));

export const findWorldTimeCity = (name: string): WorldTimeCity | undefined =>
    WORLD_TIME_CITIES.find((city) => city.name === name);

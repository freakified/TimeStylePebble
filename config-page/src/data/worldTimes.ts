import { allTimeZones, offsetMinutes } from '../utils/timezone';

/*
 * A world time is any IANA zone, stored by its id ("America/New_York").
 * The phone (src/pkjs/worldtime.js) works out its current offset whenever
 * it talks to the watch.
 */

export interface WorldTimeOption {
    label: string;
    value: string;
    category?: string;
}

const formatOffset = (minutes: number): string => {
    const sign = minutes < 0 ? '-' : '+';
    const abs = Math.abs(minutes);
    const hours = Math.floor(abs / 60);
    const mins = abs % 60;
    return `UTC${sign}${hours}${mins ? ':' + String(mins).padStart(2, '0') : ''}`;
};

// "America/Argentina/Buenos_Aires" -> "Argentina / Buenos Aires"
const zonePlace = (tz: string) =>
    (tz.includes('/') ? tz.split('/').slice(1).join(' / ') : tz).replace(/_/g, ' ');

/** The label a world time gets until the user types their own: the first
 *  three letters of its city. Must match defaultLabel() in worldtime.js. */
export const worldTimeDefaultLabel = (tz: string): string =>
    (tz.split('/').pop() ?? '').replace(/[^A-Za-z]/g, '').slice(0, 3).toUpperCase();

/** The dropdown: UTC, then every zone by region, each labelled with its
 *  offset right now, so a place in DST says so. */
export const worldTimeOptions = (): WorldTimeOption[] => {
    const zones = allTimeZones()
        .filter((tz) => tz.includes('/'))
        .map((tz) => ({
            label: `${zonePlace(tz)} (${formatOffset(offsetMinutes(tz, 0))})`,
            value: tz,
            category: tz.split('/')[0],
        }))
        .sort((a, b) => a.category.localeCompare(b.category) || a.label.localeCompare(b.label));
    return [{ label: 'UTC', value: 'UTC' }, ...zones];
};

/** The options, plus `value` if this browser doesn't list it (a zone saved
 *  under another spelling, e.g. Asia/Kolkata where Chrome says Calcutta),
 *  so the select shows what's saved rather than its first option. */
export const withWorldTimeValue = (options: WorldTimeOption[], value: string): WorldTimeOption[] =>
    !value || options.some((option) => option.value === value)
        ? options
        : [{ label: zonePlace(value), value, category: 'Saved' }, ...options];

const formatters: Record<string, Intl.DateTimeFormat> = {};

const formatterFor = (tz: string): Intl.DateTimeFormat => {
    if (!formatters[tz]) {
        formatters[tz] = new Intl.DateTimeFormat('en-US', {
            timeZone: tz,
            hour12: false,
            year: 'numeric', month: 'numeric', day: 'numeric',
            hour: 'numeric', minute: 'numeric',
        });
    }
    return formatters[tz];
};

/**
 * A zone's UTC offset in minutes at `date`, from the browser's tz database.
 * Same approach as intlOffsetMinutes in src/pkjs/timezone.js. Falls back to
 * `fallback` if the browser doesn't know the zone.
 */
export const offsetMinutes = (tz: string, fallback: number, date: Date = new Date()): number => {
    try {
        const p: Record<string, string> = {};
        for (const part of formatterFor(tz).formatToParts(date)) p[part.type] = part.value;
        // Some engines render midnight as "24" with hour12:false.
        const wallAsUtc = Date.UTC(+p.year, +p.month - 1, +p.day, +p.hour % 24, +p.minute);
        const subMinuteMs = date.getUTCSeconds() * 1000 + date.getUTCMilliseconds();
        return Math.round((wallAsUtc - (date.getTime() - subMinuteMs)) / 60000);
    } catch {
        return fallback;
    }
};

const isUsableZone = (tz: string): boolean => {
    try {
        formatterFor(tz);
        return true;
    } catch {
        return false;
    }
};

/** Every zone the browser knows (empty if it can't list them). */
export const allTimeZones = (): string[] => {
    const supportedValuesOf = (Intl as { supportedValuesOf?: (key: string) => string[] })
        .supportedValuesOf;
    if (!supportedValuesOf) return [];
    try {
        return supportedValuesOf('timeZone').filter(isUsableZone);
    } catch {
        return [];
    }
};

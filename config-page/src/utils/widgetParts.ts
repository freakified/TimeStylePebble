import { Settings } from '../context/types';

/*
 * Which parts of each widget the user can hide, mirroring hidden_parts()
 * in sidebar_widgets.c.
 *
 * A widget has at most one "marker" -- the part that names it: an icon,
 * a label, or the date's weekday -- and a few also have a "detail" that
 * can go instead: the battery's percentage, the weather's temperatures,
 * the date's month. The settings hold one bitmask per kind of part, one
 * bit per widget id.
 *
 * A widget never loses both parts, since that would leave it blank: if
 * both bits are set the watch hides the marker and keeps the detail. The
 * date is the exception -- its calendar remains either way.
 *
 * Only widgets listed here offer a choice.
 */

export interface WidgetParts {
    /** what the marker is called in the "Show" options */
    marker: string;
    /** the rest of the widget, what's left when the marker is hidden:
     *  "value", unless a plainer word names it (percentage, temperature,
     *  time) */
    value: string;
    /** whether the value can be hidden instead, leaving just the marker */
    detailHideable?: boolean;
    /** the date: the marker and detail go independently, never blanking it */
    independent?: boolean;
}

export const WIDGET_PARTS: Record<number, WidgetParts> = {
    2:  { marker: 'icon', value: 'percentage', detailHideable: true },   // Battery
    7:  { marker: 'icon', value: 'temperature', detailHideable: true },  // Current Weather
    8:  { marker: 'icon', value: 'temperature', detailHideable: true },  // Today's Forecast
    6:  { marker: 'label', value: 'value' },                             // Week Number
    11: { marker: 'label', value: 'value' },                             // Swatch Internet Time
    13: { marker: 'label', value: 'value' },                             // UV Index
    3:  { marker: 'label', value: 'time' },                              // World Time 1
    14: { marker: 'label', value: 'time' },                              // World Time 2
    9:  { marker: 'icon', value: 'value' },                              // Sleep
    10: { marker: 'icon', value: 'value' },                              // Steps
    12: { marker: 'icon', value: 'value' },                              // Heart Rate
    15: { marker: 'icon', value: 'value' },                              // Sunrise & Sunset
    17: { marker: 'icon', value: 'value' },                              // Chance of Rain
    18: { marker: 'icon', value: 'value' },                              // Distance Walked
    19: { marker: 'icon', value: 'value' },                              // Active Calories
    20: { marker: 'icon', value: 'time' },                               // Next Alarm
    4:  { marker: 'weekday', value: 'month', independent: true },        // Today's Date
};

/** anchor for the Widget Display section, which the fit warning links to */
export const WIDGET_DISPLAY_SECTION_ID = 'widget-display';

export interface HiddenParts {
    marker: boolean;
    detail: boolean;
}

const bit = (mask: number, widgetId: number) => ((mask >>> widgetId) & 1) === 1;

/** The parts the watch will actually hide for this widget: hidden_parts(). */
export const hiddenParts = (widgetId: number, settings: Settings): HiddenParts => {
    const marker = bit(settings.widgetHideMarker ?? 0, widgetId);
    let detail = bit(settings.widgetHideDetail ?? 0, widgetId);
    const parts = WIDGET_PARTS[widgetId];

    if (!parts || !(parts.detailHideable || parts.independent)) {
        detail = false;
    } else if (!parts.independent && marker) {
        detail = false;
    }

    return { marker, detail };
};

/* ── The "Show" select ── */

/** Which parts are hidden, as one select value: bit 0 marker, bit 1 detail */
export type ShowChoice = 0 | 1 | 2 | 3;

export interface ShowOption {
    value: ShowChoice;
    label: string;
}

const capitalize = (str: string) => str.charAt(0).toUpperCase() + str.slice(1);

/** The choices a widget's "Show" select offers, everything shown first. */
export const showOptions = (widgetId: number): ShowOption[] => {
    const parts = WIDGET_PARTS[widgetId];
    if (!parts) return [];

    if (parts.independent) {
        // the date reads as a sliding scale from full to just the calendar
        return [
            { value: 0, label: 'Weekday, day & month' },
            { value: 2, label: 'Weekday & day' },
            { value: 1, label: 'Day & month' },
            { value: 3, label: 'Day only' },
        ];
    }

    const options: ShowOption[] = [
        { value: 0, label: `${capitalize(parts.marker)} & ${parts.value}` },
    ];
    if (parts.detailHideable) {
        options.push({ value: 2, label: `${capitalize(parts.marker)} only` });
    }
    options.push({ value: 1, label: `${capitalize(parts.value)} only` });
    return options;
};

export const showChoice = (widgetId: number, settings: Settings): ShowChoice => {
    const hidden = hiddenParts(widgetId, settings);
    return ((hidden.marker ? 1 : 0) | (hidden.detail ? 2 : 0)) as ShowChoice;
};

/** The settings change that makes a widget show `choice`. */
export const setShowChoice = (
    widgetId: number,
    choice: ShowChoice,
    settings: Settings
): Pick<Settings, 'widgetHideMarker' | 'widgetHideDetail'> => {
    const flag = 1 << widgetId;
    const apply = (mask: number, hide: boolean) => (hide ? mask | flag : mask & ~flag);
    return {
        widgetHideMarker: apply(settings.widgetHideMarker ?? 0, (choice & 1) !== 0),
        widgetHideDetail: apply(settings.widgetHideDetail ?? 0, (choice & 2) !== 0),
    };
};

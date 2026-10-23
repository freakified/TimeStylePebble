// Widget theme presets. Each one is written once for every watch: a
// region lists its candidate widgets most important first, and takes as
// many of them as the watch has room for (see utils/widgetThemes.ts), so
// bigger watches naturally show more. A short list simply stops early, on
// every size.
//
// The first preset is the default and must resolve to exactly the
// default widgets (utils/defaultSettings.ts) on every watch, so a fresh
// install shows it selected.

import { Capabilities } from '../context/types';

// widget ids, matching ALL_WIDGET_OPTIONS in widgetTypes.ts
const BATTERY = 2;
const WORLD_TIME_1 = 3;
const DATE = 4;
const CURRENT_WEATHER = 7;
const FORECAST = 8;
const SLEEP = 9;
const STEPS = 10;
const HEART_RATE = 12;
const UV_INDEX = 13;
const WORLD_TIME_2 = 14;
const SUNRISE_SUNSET = 15;
const CHANCE_OF_RAIN = 17;
const DISTANCE = 18;
const CALORIES = 19;

export interface RectWidgets {
    sidebar: number[];
    bar?: number[];
}

export interface RoundWidgets {
    left: number[];
    right: number[];
    bar?: number[];
}

export type FormFactor = 'small-rect' | 'large-rect' | 'small-round' | 'large-round';

export interface WidgetPreset {
    name: string;
    /** hidden on watches missing any of these */
    requires?: (keyof Capabilities)[];
    rect: RectWidgets;
    // a rect sidebar doesn't split onto two arcs by itself, so round
    // presets name their arcs
    round: RoundWidgets;
    /**
     * Escape hatch: the exact widgets for one form factor, placed as
     * listed rather than filled by fit (still capped, and still dropping
     * widgets the watch can't show).
     */
    exact?: {
        'small-rect'?: RectWidgets;
        'large-rect'?: RectWidgets;
        'small-round'?: RoundWidgets;
        'large-round'?: RoundWidgets;
    };
}

export const WIDGET_PRESETS: WidgetPreset[] = [
    {
        name: 'Classic',
        rect: { sidebar: [CURRENT_WEATHER, DATE] },
        round: { left: [CURRENT_WEATHER], right: [DATE] },
    },
    {
        name: 'Minimal',
        rect: { sidebar: [DATE] },
        round: { left: [], right: [DATE] },
    },
    {
        name: "Lots o' Fitness",
        requires: ['HEALTH'],
        rect: {
            sidebar: [STEPS, HEART_RATE, DISTANCE, CALORIES, SLEEP],
            bar: [DATE, BATTERY],
        },
        round: {
            left: [STEPS, DISTANCE, SLEEP],
            right: [HEART_RATE, CALORIES, BATTERY],
            bar: [DATE],
        },
    },
    {
        name: "Lots o' Weather",
        rect: {
            sidebar: [CURRENT_WEATHER, FORECAST, CHANCE_OF_RAIN, SUNRISE_SUNSET, UV_INDEX],
            bar: [DATE, BATTERY],
        },
        round: {
            left: [CURRENT_WEATHER, CHANCE_OF_RAIN, UV_INDEX],
            right: [FORECAST, SUNRISE_SUNSET],
            bar: [DATE],
        },
    },
    {
        name: 'Traveler',
        rect: { sidebar: [WORLD_TIME_1, WORLD_TIME_2, DATE, BATTERY] },
        round: { left: [WORLD_TIME_1], right: [WORLD_TIME_2], bar: [DATE] },
    },
];

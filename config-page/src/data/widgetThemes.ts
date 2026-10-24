// Widget theme presets. Each one is written once for every watch: a
// region lists its candidate widgets most important first, and takes as
// many of them as the watch has room for (see utils/widgetThemes.ts), so
// bigger watches naturally show more. A short list simply stops early, on
// every size. Every candidate should make it onto at least one watch; one
// that never fits anywhere is just noise in the list.
//
// The first preset is the default and must resolve to exactly the
// default widgets (utils/defaultSettings.ts) on every watch, so a fresh
// install shows it selected.

import { Capabilities } from '../context/types';

// widget ids, matching ALL_WIDGET_OPTIONS in widgetTypes.ts
const BATTERY = 2;
const WORLD_TIME_1 = 3;
const DATE = 4;
const SECONDS = 5;
const WEEK_NUMBER = 6;
const CURRENT_WEATHER = 7;
const FORECAST = 8;
const SLEEP = 9;
const STEPS = 10;
const SWATCH_TIME = 11;
const HEART_RATE = 12;
const UV_INDEX = 13;
const WORLD_TIME_2 = 14;
const SUNRISE_SUNSET = 15;
const CHANCE_OF_RAIN = 17;
const DISTANCE = 18;
const CALORIES = 19;
const NEXT_ALARM = 20;

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
        name: 'Everyday',
        rect: { sidebar: [CURRENT_WEATHER, DATE, STEPS, BATTERY] },
        round: { left: [CURRENT_WEATHER, STEPS], right: [DATE], bar: [BATTERY] },
    },
    {
        name: "Lots o' Fitness",
        requires: ['HEALTH'],
        rect: {
            sidebar: [STEPS, HEART_RATE, DISTANCE, CALORIES],
            bar: [DATE],
        },
        round: {
            left: [STEPS, DISTANCE],
            right: [CALORIES, BATTERY],
            bar: [DATE],
        },
    },
    {
        name: 'Well Rested',
        requires: ['HEALTH'],
        rect: { sidebar: [SLEEP, HEART_RATE, STEPS, BATTERY] },
        round: { left: [SLEEP], right: [STEPS], bar: [BATTERY] },
    },
    {
        name: "Lots o' Weather",
        rect: {
            sidebar: [CURRENT_WEATHER, FORECAST, CHANCE_OF_RAIN, UV_INDEX],
            bar: [DATE],
        },
        round: {
            left: [CURRENT_WEATHER, CHANCE_OF_RAIN, UV_INDEX],
            right: [FORECAST, SUNRISE_SUNSET],
            bar: [DATE],
        },
    },
    {
        name: 'Great Outdoors',
        rect: {
            sidebar: [CURRENT_WEATHER, SUNRISE_SUNSET, STEPS, DISTANCE],
            bar: [DATE],
        },
        round: {
            left: [CURRENT_WEATHER, STEPS],
            right: [SUNRISE_SUNSET, DISTANCE],
            bar: [DATE],
        },
    },
    {
        name: 'Commuter',
        rect: {
            sidebar: [NEXT_ALARM, CURRENT_WEATHER, CHANCE_OF_RAIN, DATE],
            bar: [BATTERY],
        },
        round: {
            left: [NEXT_ALARM, CHANCE_OF_RAIN],
            right: [CURRENT_WEATHER],
            bar: [DATE],
        },
    },
    {
        name: 'Traveler',
        rect: { sidebar: [WORLD_TIME_1, WORLD_TIME_2, DATE, BATTERY] },
        round: { left: [WORLD_TIME_1], right: [WORLD_TIME_2], bar: [DATE] },
    },
    {
        name: 'Time Nerd',
        rect: { sidebar: [DATE, WEEK_NUMBER, SWATCH_TIME, SECONDS, WORLD_TIME_1] },
        round: { left: [WEEK_NUMBER, SWATCH_TIME], right: [DATE], bar: [SECONDS] },
    },
];

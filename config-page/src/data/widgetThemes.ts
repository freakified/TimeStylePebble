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
   * Widgets placed before the rest of their region's list when room is
   * short, though still shown where the list puts them: a date listed
   * last, at the bottom of the sidebar, is the last thing to drop.
   */
  essential?: number[];
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
    name: 'Extra',
    rect: { sidebar: [CURRENT_WEATHER, STEPS, BATTERY, DATE] },
    round: { left: [CURRENT_WEATHER, BATTERY], right: [DATE] },
    exact: {
      'small-rect': {
        sidebar: [CURRENT_WEATHER, BATTERY, DATE]
      }
    }
  },
  {
    name: 'Extra 2',
    rect: { sidebar: [CURRENT_WEATHER, UV_INDEX, BATTERY, DATE], bar: [STEPS] },
    round: { left: [CURRENT_WEATHER, UV_INDEX, BATTERY], right: [DATE], bar: [STEPS] },
    exact: {
      'small-rect': {
        sidebar: [CURRENT_WEATHER, UV_INDEX, DATE],
        bar: [STEPS]
      }
    }
  },
  {
    name: 'Extra 3',
    rect: {
      sidebar: [FORECAST, CHANCE_OF_RAIN, DATE],
      bar: [SUNRISE_SUNSET, STEPS],
    },
    round: {
      left: [FORECAST, CHANCE_OF_RAIN],
      right: [STEPS, DATE],
      bar: [SUNRISE_SUNSET],
    },
    essential: [DATE],
  },
  {
    name: 'Commute',
    requires: ['ALARMS'],
    rect: {
      sidebar: [FORECAST, CHANCE_OF_RAIN, BATTERY, DATE],
      bar: [NEXT_ALARM],
    },
    round: {
      left: [FORECAST, CHANCE_OF_RAIN],
      right: [DATE, BATTERY],
      bar: [NEXT_ALARM],
    },
    essential: [DATE],
  },
  {
    name: 'Weather Enthusiast',
    rect: {
      sidebar: [FORECAST, CHANCE_OF_RAIN, DATE],
      bar: [CURRENT_WEATHER, UV_INDEX],
    },
    round: {
      left: [CURRENT_WEATHER, UV_INDEX],
      right: [CHANCE_OF_RAIN, DATE],
      bar: [FORECAST],
    },
    essential: [DATE],
  },
  {
    name: 'Fitness',
    requires: ['HEALTH'],
    rect: {
      sidebar: [DISTANCE, CALORIES, DATE],
      bar: [STEPS, HEART_RATE],
    },
    round: {
      left: [DISTANCE, CALORIES],
      right: [DATE],
      bar: [STEPS],
    },
    essential: [DATE],
  },
  {
    name: 'Sleep Fan',
    requires: ['HEALTH', 'ALARMS'],
    rect: {
      sidebar: [SLEEP, HEART_RATE, BATTERY, DATE],
      bar: [NEXT_ALARM],
    },
    round: { left: [SLEEP], right: [DATE], bar: [BATTERY] },
    essential: [DATE],
  },
  {
    name: 'Traveler',
    rect: {
      sidebar: [CURRENT_WEATHER, WORLD_TIME_2, DATE],
      bar: [WORLD_TIME_1],
    },
    round: {
      left: [CURRENT_WEATHER, WORLD_TIME_2],
      right: [DATE],
      bar: [WORLD_TIME_1],
    },
    essential: [DATE],
  },
  {
    name: 'Time Enthusiast',
    rect: {
      sidebar: [WEEK_NUMBER, WORLD_TIME_1, SWATCH_TIME, DATE],
      bar: [SECONDS],
    },
    round: { left: [WEEK_NUMBER, WORLD_TIME_1, SWATCH_TIME], right: [DATE], bar: [SECONDS] },
    essential: [DATE],
  },
];

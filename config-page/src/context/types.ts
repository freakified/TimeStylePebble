export interface WatchInfo {
  platform: string;
  model: string;
  language: string;
  firmware: {
    major: number;
    minor: number;
  };
}

export interface Capabilities {
  APLITE: boolean;
  BASALT: boolean;
  CHALK: boolean;
  DIORITE: boolean;
  EMERY: boolean;
  FLINT: boolean;
  GABBRO: boolean;
  BW: boolean;
  COLOR: boolean;
  ROUND: boolean;
  RECT: boolean;
  DISPLAY_144x168: boolean;
  DISPLAY_180x180_ROUND: boolean;
  DISPLAY_200x228: boolean;
  DISPLAY_260x260_ROUND: boolean;
  MICROPHONE: boolean;
  SMARTSTRAP: boolean;
  SMARTSTRAP_POWER: boolean;
  HEALTH: boolean;
}

// which side the vertical sidebar sits on; the bottom bar isn't a layout
// choice, it's on whenever its widget slots hold something
export type Layout =
  | 'right-sidebar'
  | 'left-sidebar';

export interface Settings {
  // Internal theme selectors (stripped before sending to watch)
  SETTING_THEME: string;
  SETTING_NIGHT_THEME: string;  // kept for future night theme support
  SETTING_USE_NIGHT_THEME: number;  // kept for future night theme support

  // Colors (hex strings without '#') — day theme
  colorBackground: string;
  colorTime: string;
  colorSidebar: string;
  colorSidebarText: string;
  colorAltSidebarText: string;  // text on "dark" regions (the bottom bar)

  // Widget slots (integer IDs)
  // 0=Empty, 2=Battery, 3=World Time 1, 4=Date, 5=Seconds, 6=Week Number,
  // 7=Current Weather, 8=Today's Forecast, 9=Sleep, 10=Steps,
  // 11=Swatch Internet Time, 12=Heart Rate, 13=UV Index, 14=World Time 2,
  // 15=Sunrise & Sunset, 16=Wind Speed, 17=Chance of Rain
  // On round these same six slots are split between the two arcs:
  // 0..2 are the left arc's widget list, 3..5 the right arc's.
  widget0Id: number;  // first sidebar slot (top on rect, left arc top on round)
  widget1Id: number;  // second sidebar slot
  widget2Id: number;  // third sidebar slot
  widget3Id: number;  // fourth sidebar slot (bottom on non-emery rects; right arc top on round)
  widget4Id: number;  // fifth sidebar slot (emery only on rect)
  widget5Id: number;  // sixth sidebar slot (bottom on emery)
  bottomBarWidget0Id: number;  // bar slot 1 (rect only)
  bottomBarWidget1Id: number;  // bar slot 2 (rect only)
  bottomBarWidget2Id: number;  // bar slot 3 (emery only)

  // General display
  languageId: number;
  leadingZero: boolean;
  clockFont: 'default' | 'bold' | 'bold-h' | 'bold-m' | 'leco';
  layout: Layout;
  largeSidebarFont: boolean;
  wideSidebar: boolean;  // wider sidebar: widgets show their full form
  // round only: fill both arcs with the sidebar color. Off (the default)
  // fills only the arc on the layout's side and lets the other blend
  // into the watchface.
  colorBothArcs: boolean;
  decimalSeparator: '.' | ',';

  // Bluetooth
  bluetoothVibe: boolean;

  // Status icon (disconnection / quiet time while active; mini-battery removed for now)
  statusBarEnabled: boolean;
  statusBarShowConnection: boolean; // show "connected" icon even when connected

  // Notifications
  hourlyVibe: 'none' | 'everyHour' | 'everyHalfHour';

  // Weather
  weatherUnits: 'f' | 'c';
  weatherLocation: string;
  weatherLocationLat: string;
  weatherLocationLng: string;
  weatherMode: boolean;
  weatherDataSource: string;
  weatherApiKey: string;

  // World time (city names match config-page/src/data/cities.ts and
  // src/pkjs/cities.js; the phone recomputes DST-aware offsets from them).
  // altClockName/altClockOffset are the watch's current label + offset in
  // minutes — read-only echoes used as fallbacks, not edited directly.
  worldTime1City: string;
  worldTime1Label: string;  // optional custom label, max 4 chars
  worldTime2City: string;
  worldTime2Label: string;
  altClockName: string;
  altClockOffset: number;
  altClock2Name: string;
  altClock2Offset: number;

  // Health
  healthActivityMetric: 'steps' | 'distance';
  healthSleepType: 'total' | 'restful';
}

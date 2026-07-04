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

export type Layout =
  | 'right-sidebar'
  | 'left-sidebar'
  | 'right-sidebar-bottom-bar'
  | 'left-sidebar-bottom-bar'
  | 'bottom-bar-only';

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

  // Widget slots (integer IDs)
  // 0=Empty, 2=Battery, 3=Alt TZ, 4=Date, 5=Seconds, 6=Week Number,
  // 7=Current Weather, 8=Today's Forecast, 9=Sleep, 10=Steps,
  // 11=Swatch Internet Time, 12=Heart Rate, 13=UV Index
  widget0Id: number;  // first sidebar slot (top on rect, left arc on round)
  widget1Id: number;  // second sidebar slot (right arc on round)
  widget2Id: number;  // third sidebar slot (rect only)
  widget3Id: number;  // fourth sidebar slot (emery only)
  bottomBarWidget0Id: number;  // bar slot 1 (rect only)
  bottomBarWidget1Id: number;  // bar slot 2 (rect only)
  bottomBarWidget2Id: number;  // bar slot 3 (stacked bars only)
  bottomBarWidget3Id: number;  // bar slot 4 (stacked bars only)

  // General display
  languageId: number;
  leadingZero: boolean;
  clockFont: 'default' | 'bold' | 'bold-h' | 'bold-m' | 'leco';
  layout: Layout;
  largeSidebarFont: boolean;
  wideSidebar: boolean;
  decimalSeparator: '.' | ',';

  // Bluetooth
  disconnectIcon: boolean;
  bluetoothVibe: boolean;

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

  // Battery
  batteryMeter: 'icon-and-percent' | 'icon-only';
  autoBattery: boolean;

  // Alternate time zone
  altClockName: string;    // max 4 chars
  altClockOffset: number;  // -12 to +14

  // Health
  healthActivityMetric: 'steps' | 'distance';
  healthSleepType: 'total' | 'restful';
}

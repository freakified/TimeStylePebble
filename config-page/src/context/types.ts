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

export interface Settings {
  // Internal theme selectors (stripped before sending to watch)
  SETTING_THEME: string;
  SETTING_NIGHT_THEME: string;  // kept for future night theme support
  SETTING_USE_NIGHT_THEME: number;  // kept for future night theme support

  // Colors (hex strings without '#') — day theme
  color_bg: string;
  color_time: string;
  color_sidebar: string;
  sidebar_text_color: string;

  // Widget slots (integer IDs)
  // 0=Empty, 2=Battery, 3=Alt TZ, 4=Date, 5=Seconds, 6=Week Number,
  // 7=Current Weather, 8=Today's Forecast, 9=Sleep, 10=Steps,
  // 11=Swatch Internet Time, 12=Heart Rate, 13=UV Index
  widget_0_id: number;  // top slot (rect) or left slot (round)
  widget_1_id: number;  // middle slot (rect only)
  widget_2_id: number;  // bottom slot (rect) or right slot (round)

  // General display
  language_id: number;                    // 0–36
  leading_zero_setting: string;           // 'yes' | 'no'
  clock_font_setting: string;             // 'default' | 'bold' | 'bold-h' | 'bold-m' | 'leco'
  sidebar_position: string;               // 'left' | 'right'
  use_large_sidebar_font_setting: string; // 'yes' | 'no'
  decimal_separator: string;              // '.' | ','

  // Bluetooth
  disconnect_icon_setting: string;  // 'yes' | 'no'
  bluetooth_vibe_setting: string;   // 'yes' | 'no'

  // Notifications
  hourly_vibe_setting: string;  // 'no' | 'half' | 'yes'

  // Weather
  units: string;            // 'f' | 'c'
  weather_loc: string;
  weather_loc_lat: string;
  weather_loc_lng: string;
  weather_setting: string;  // 'auto' | 'manual'
  weather_datasource: string;
  weather_api_key: string;

  // Battery
  battery_meter_setting: string;  // 'icon-and-percent' | 'icon-only'
  autobattery_setting: string;    // 'on' | 'off'

  // Alternate time zone
  altclock_name: string;    // max 4 chars
  altclock_offset: number;  // -12 to +14

  // Health
  health_use_distance: string;      // 'yes' | 'no'
  health_use_restful_sleep: string; // 'yes' | 'no'
}

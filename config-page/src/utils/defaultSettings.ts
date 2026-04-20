import { Settings, Capabilities } from '../context/types';

export const getDefaultSettings = (capabilities: Capabilities): Settings => ({
  // Internal theme selectors
  SETTING_THEME: '0',
  SETTING_NIGHT_THEME: '0',
  SETTING_USE_NIGHT_THEME: 0,

  // Colors — match Jekyll config.js defaults
  color_bg: '000000',
  color_time: capabilities.BW ? 'FFFFFF' : 'FF5500',
  color_sidebar: capabilities.BW ? 'FFFFFF' : 'FF5500',
  sidebar_text_color: '000000',

  // Widget slots
  widget_0_id: 7,  // Current Weather
  widget_1_id: 0,  // Empty
  widget_2_id: 4,  // Today's Date

  // General
  language_id: 0,
  leading_zero_setting: false,
  clock_font_setting: 'default',
  sidebar_position: 'right',
  use_large_sidebar_font_setting: false,
  decimal_separator: '.',

  // Bluetooth
  disconnect_icon_setting: false,
  bluetooth_vibe_setting: false,

  // Notifications
  hourly_vibe_setting: 'none',

  // Weather
  units: 'f',
  weather_loc: '',
  weather_loc_lat: '',
  weather_loc_lng: '',
  weather_setting: 'auto',
  weather_datasource: 'openmeteo',
  weather_api_key: '',

  // Battery
  battery_meter_setting: 'icon-and-percent',
  autobattery_setting: 'on',

  // Alternate time zone
  altclock_name: 'ALT',
  altclock_offset: 0,

  // Health
  health_use_distance: 'steps',
  health_use_restful_sleep: 'total',
});

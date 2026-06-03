import { Settings, Capabilities } from '../context/types';

export const getDefaultSettings = (capabilities: Capabilities): Settings => ({
  // Internal theme selectors
  SETTING_THEME: '0',
  SETTING_NIGHT_THEME: '0',
  SETTING_USE_NIGHT_THEME: 0,

  // Colors — match Jekyll config.js defaults
  colorBackground: '000000',
  colorTime: capabilities.BW ? 'FFFFFF' : 'FF5500',
  colorSidebar: capabilities.BW ? 'FFFFFF' : 'FF5500',
  colorSidebarText: '000000',

  // Widget slots
  widget0Id: 7,  // Current Weather
  widget1Id: 0,  // Empty
  widget2Id: 4,  // Today's Date

  // General
  languageId: 0,
  leadingZero: false,
  clockFont: 'default',
  sidebarPosition: 'right',
  largeSidebarFont: false,
  wideSidebar: false,
  decimalSeparator: '.',

  // Bluetooth
  disconnectIcon: false,
  bluetoothVibe: false,

  // Notifications
  hourlyVibe: 'none',

  // Weather
  weatherUnits: 'f',
  weatherLocation: '',
  weatherLocationLat: '',
  weatherLocationLng: '',
  weatherMode: true,
  weatherDataSource: 'openmeteo',
  weatherApiKey: '',

  // Battery
  batteryMeter: 'icon-and-percent',
  autoBattery: true,

  // Alternate time zone
  altClockName: 'ALT',
  altClockOffset: 0,

  // Health
  healthActivityMetric: 'steps',
  healthSleepType: 'total',
});

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
  colorAltSidebarText: 'FFFFFF',

  // Widget slots
  widget0Id: 7,  // Current Weather
  widget1Id: capabilities.ROUND ? 4 : 0,  // Date on round arcs, else Empty
  widget2Id: capabilities.ROUND || capabilities.EMERY ? 0 : 4,  // Today's Date
  widget3Id: capabilities.EMERY ? 4 : 0,  // Date pins to emery's 4th slot
  bottomBarWidget0Id: 2,  // Battery
  bottomBarWidget1Id: 5,  // Seconds
  bottomBarWidget2Id: 0,  // Empty
  bottomBarWidget3Id: 0,  // Empty

  // General
  languageId: 0,
  leadingZero: false,
  clockFont: 'default',
  layout: 'right-sidebar',
  largeSidebarFont: false,
  seamlessSidebar: false,
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

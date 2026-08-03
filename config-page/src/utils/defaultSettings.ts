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
  // packed widget list: Current Weather on top, Date pinned to the bottom
  widget0Id: 7,
  widget1Id: 4,
  widget2Id: 0,
  widget3Id: 0,
  widget4Id: 0,
  widget5Id: 0,
  bottomBarWidget0Id: 10, // Steps
  bottomBarWidget1Id: 0,  // Empty
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
  bluetoothVibe: false,

  // Status bar
  statusBarEnabled: true,
  statusBarShowConnection: false,

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

  // World time
  worldTime1City: 'UTC',
  worldTime1Label: '',
  worldTime2City: 'TOKYO',
  worldTime2Label: '',
  altClockName: 'UTC',
  altClockOffset: 0,
  altClock2Name: 'TYO',
  altClock2Offset: 540,

  // Health
  healthActivityMetric: 'steps',
  healthSleepType: 'total',
});

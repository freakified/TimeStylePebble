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

  // Widget slots — a packed widget list: Current Weather on top, Date
  // pinned to the bottom. Round splits the same list between its arcs
  // (0..2 left, 3..5 right), so its defaults straddle the split to put
  // one widget in each arc.
  widget0Id: 7,
  widget1Id: capabilities.ROUND ? 0 : 4,
  widget2Id: 0,
  widget3Id: capabilities.ROUND ? 4 : 0,
  widget4Id: 0,
  widget5Id: 0,
  // the bottom bar starts empty on purpose: it's on screen only when it
  // holds a widget, so a default here would ship the bar to everyone
  bottomBarWidget0Id: 0,
  bottomBarWidget1Id: 0,
  bottomBarWidget2Id: 0,

  // General
  languageId: 0,
  leadingZero: false,
  clockFont: 'default',
  layout: 'right-sidebar',
  largeSidebarFont: false,
  wideSidebar: false,
  colorBothArcs: false,
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

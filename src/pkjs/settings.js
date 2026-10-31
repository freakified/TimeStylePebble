// ids 2-4 were Avenir weights before the weights became their own
// settings; the watch migrates them away, so they're never sent
var CLOCK_FONT_BY_ID = {
  0: 'avenir',
  1: 'leco',
  5: 'garamond'
};

var CLOCK_FONT_ID_BY_NAME = {
  'avenir': 0,
  'leco': 1,
  'garamond': 5
};

var HOURLY_VIBE_BY_ID = {
  0: 'none',
  1: 'everyHour',
  2: 'everyHalfHour'
};

// a layout says only which side the sidebar goes on; the bottom bar is
// present whenever its widget slots hold something
var LAYOUT_BY_ID = {
  0: 'right-sidebar',
  1: 'left-sidebar'
};

var LAYOUT_ID_BY_NAME = {
  'right-sidebar': 0,
  'left-sidebar': 1
};

var HOURLY_VIBE_ID_BY_NAME = {
  'none': 0,
  'everyHour': 1,
  'everyHalfHour': 2
};

function padHex(value) {
  var hex = Number(value || 0).toString(16).toUpperCase();
  while (hex.length < 6) {
    hex = '0' + hex;
  }
  return hex;
}

function getNumber(value, fallback) {
  if (value === undefined || value === null) {
    return fallback;
  }
  return Number(value);
}

function getBoolean(value) {
  return getNumber(value, 0) !== 0;
}

function getMappedValue(map, key, fallback) {
  if (map[key] !== undefined) {
    return map[key];
  }
  return fallback;
}

function decimalSeparatorFromPayload(value) {
  if (value === ',') {
    return ',';
  }

  if (Number(value) === 44) {
    return ',';
  }

  return '.';
}

function settingsFromWatchPayload(payload) {
  return {
    colorBackground: padHex(payload.SettingColorBG),
    colorTime: padHex(payload.SettingColorTime),
    colorSidebar: padHex(payload.SettingColorSidebar),

    widget0Id: getNumber(payload.SettingWidget0ID, 7),
    widget1Id: getNumber(payload.SettingWidget1ID, 4),
    widget2Id: getNumber(payload.SettingWidget2ID, 0),
    widget3Id: getNumber(payload.SettingWidget3ID, 0),
    widget4Id: getNumber(payload.SettingWidget4ID, 0),
    widget5Id: getNumber(payload.SettingWidget5ID, 0),
    bottomBarWidget0Id: getNumber(payload.SettingBottomBarWidget0ID, 0),
    bottomBarWidget1Id: getNumber(payload.SettingBottomBarWidget1ID, 0),
    bottomBarWidget2Id: getNumber(payload.SettingBottomBarWidget2ID, 0),

    languageId: getNumber(payload.SettingLanguageID, 255),
    leadingZero: getBoolean(payload.SettingShowLeadingZero),
    clockFont: getMappedValue(CLOCK_FONT_BY_ID, getNumber(payload.SettingClockFontId, 0), 'avenir'),
    boldHours: getBoolean(payload.SettingClockHoursBold),
    boldMinutes: getBoolean(payload.SettingClockMinutesBold),
    layout: getMappedValue(LAYOUT_BY_ID, getNumber(payload.SettingLayoutId, 0), 'right-sidebar'),
    largeSidebarFont: getBoolean(payload.SettingUseLargeFonts),
    wideSidebar: getBoolean(payload.SettingUseWideSidebar),
    colorBothArcs: getBoolean(payload.SettingColorBothArcs),
    decimalSeparator: decimalSeparatorFromPayload(payload.SettingDecimalSep),

    bluetoothVibe: getBoolean(payload.SettingBluetoothVibe),

    hourlyVibe: getMappedValue(HOURLY_VIBE_BY_ID, getNumber(payload.SettingHourlyVibe, 0), 'none'),

    weatherUnits: getBoolean(payload.SettingUseMetric) ? 'c' : 'f',

    statusBarEnabled: getNumber(payload.SettingEnableStatusBar, 1) !== 0,
    statusBarShowConnection: getBoolean(payload.SettingStatusShowConnection),

    altClockName: payload.SettingAltClockName || 'UTC',
    altClockOffset: getNumber(payload.SettingAltClockOffset, 0),
    altClock2Name: payload.SettingAltClock2Name || 'UTC',
    altClock2Offset: getNumber(payload.SettingAltClock2Offset, 0),

    healthSleepType: getBoolean(payload.SettingHealthUseRestfulSleep) ? 'restful' : 'total'
  };
}

function settingsToWatchPayload(configData) {
  var dict = {};

  if (configData.colorBackground) {
    dict.SettingColorBG = parseInt(configData.colorBackground, 16);
  }

  if (configData.colorSidebar) {
    dict.SettingColorSidebar = parseInt(configData.colorSidebar, 16);
  }

  if (configData.colorTime) {
    dict.SettingColorTime = parseInt(configData.colorTime, 16);
  }

  if (configData.languageId !== undefined) {
    dict.SettingLanguageID = configData.languageId;
  }

  if (configData.leadingZero !== undefined) {
    dict.SettingShowLeadingZero = configData.leadingZero ? 1 : 0;
  }

  if (configData.clockFont) {
    var clockFontId = getMappedValue(CLOCK_FONT_ID_BY_NAME, configData.clockFont, undefined);
    if (clockFontId !== undefined) {
      dict.SettingClockFontId = clockFontId;
    }
  }

  if (configData.boldHours !== undefined) {
    dict.SettingClockHoursBold = configData.boldHours ? 1 : 0;
  }

  if (configData.boldMinutes !== undefined) {
    dict.SettingClockMinutesBold = configData.boldMinutes ? 1 : 0;
  }

  if (configData.statusBarEnabled !== undefined) {
    dict.SettingEnableStatusBar = configData.statusBarEnabled ? 1 : 0;
  }

  if (configData.statusBarShowConnection !== undefined) {
    dict.SettingStatusShowConnection = configData.statusBarShowConnection ? 1 : 0;
  }

  if (configData.bluetoothVibe !== undefined) {
    dict.SettingBluetoothVibe = configData.bluetoothVibe ? 1 : 0;
  }

  if (configData.hourlyVibe !== undefined) {
    dict.SettingHourlyVibe = getMappedValue(HOURLY_VIBE_ID_BY_NAME, configData.hourlyVibe, 0);
  }

  dict.SettingWidget0ID = configData.widget0Id;
  dict.SettingWidget1ID = configData.widget1Id;
  dict.SettingWidget2ID = configData.widget2Id;
  dict.SettingBottomBarWidget0ID = configData.bottomBarWidget0Id;
  dict.SettingBottomBarWidget1ID = configData.bottomBarWidget1Id;

  if (configData.widget3Id !== undefined) {
    dict.SettingWidget3ID = configData.widget3Id;
  }

  if (configData.widget4Id !== undefined) {
    dict.SettingWidget4ID = configData.widget4Id;
  }

  if (configData.widget5Id !== undefined) {
    dict.SettingWidget5ID = configData.widget5Id;
  }

  if (configData.bottomBarWidget2Id !== undefined) {
    dict.SettingBottomBarWidget2ID = configData.bottomBarWidget2Id;
  }

  if (configData.layout !== undefined) {
    var layoutId = getMappedValue(LAYOUT_ID_BY_NAME, configData.layout, undefined);
    if (layoutId !== undefined) {
      dict.SettingLayoutId = layoutId;
    }
  }

  if (configData.largeSidebarFont !== undefined) {
    dict.SettingUseLargeFonts = configData.largeSidebarFont ? 1 : 0;
  }

  if (configData.colorBothArcs !== undefined) {
    dict.SettingColorBothArcs = configData.colorBothArcs ? 1 : 0;
  }

  if (configData.wideSidebar !== undefined) {
    dict.SettingUseWideSidebar = configData.wideSidebar ? 1 : 0;
  }

  if (configData.weatherUnits) {
    dict.SettingUseMetric = configData.weatherUnits == 'c' ? 1 : 0;
  }

  // world time (formerly "alt clock") settings are handled by worldtime.js,
  // which translates city selections into label + offset payloads

  if (configData.decimalSeparator) {
    dict.SettingDecimalSep = configData.decimalSeparator;
  }

  if (configData.healthSleepType !== undefined) {
    dict.SettingHealthUseRestfulSleep = configData.healthSleepType == 'restful' ? 1 : 0;
  }

  return dict;
}

function isConfigSnapshot(payload) {
  return payload && (
    payload.SettingColorBG !== undefined ||
    payload.SettingWidget0ID !== undefined ||
    payload.SettingClockFontId !== undefined
  );
}

module.exports = {
  isConfigSnapshot: isConfigSnapshot,
  settingsFromWatchPayload: settingsFromWatchPayload,
  settingsToWatchPayload: settingsToWatchPayload
};

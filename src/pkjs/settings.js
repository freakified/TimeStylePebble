var CLOCK_FONT_BY_ID = {
  0: 'default',
  1: 'leco',
  2: 'bold',
  3: 'bold-h',
  4: 'bold-m'
};

var CLOCK_FONT_ID_BY_NAME = {
  'default': 0,
  'leco': 1,
  'bold': 2,
  'bold-h': 3,
  'bold-m': 4
};

var HOURLY_VIBE_BY_ID = {
  0: 'none',
  1: 'everyHour',
  2: 'everyHalfHour'
};

var LAYOUT_BY_ID = {
  0: 'right-sidebar',
  1: 'left-sidebar',
  2: 'right-sidebar-bottom-bar',
  3: 'left-sidebar-bottom-bar',
  4: 'bottom-bar-only'
};

var LAYOUT_ID_BY_NAME = {
  'right-sidebar': 0,
  'left-sidebar': 1,
  'right-sidebar-bottom-bar': 2,
  'left-sidebar-bottom-bar': 3,
  'bottom-bar-only': 4
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
    colorSidebarText: padHex(payload.SettingSidebarTextColor),

    widget0Id: getNumber(payload.SettingWidget0ID, 7),
    widget1Id: getNumber(payload.SettingWidget1ID, 0),
    widget2Id: getNumber(payload.SettingWidget2ID, 4),
    widget3Id: getNumber(payload.SettingWidget3ID, 0),
    bottomBarWidget0Id: getNumber(payload.SettingBottomBarWidget0ID, 2),
    bottomBarWidget1Id: getNumber(payload.SettingBottomBarWidget1ID, 5),
    bottomBarWidget2Id: getNumber(payload.SettingBottomBarWidget2ID, 0),
    bottomBarWidget3Id: getNumber(payload.SettingBottomBarWidget3ID, 0),

    languageId: getNumber(payload.SettingLanguageID, 0),
    leadingZero: getBoolean(payload.SettingShowLeadingZero),
    clockFont: getMappedValue(CLOCK_FONT_BY_ID, getNumber(payload.SettingClockFontId, 0), 'default'),
    layout: getMappedValue(LAYOUT_BY_ID, getNumber(payload.SettingLayoutId, 0), 'right-sidebar'),
    largeSidebarFont: getBoolean(payload.SettingUseLargeFonts),
    wideSidebar: getBoolean(payload.SettingUseWideSidebar),
    decimalSeparator: decimalSeparatorFromPayload(payload.SettingDecimalSep),

    disconnectIcon: getBoolean(payload.SettingDisconnectIcon),
    bluetoothVibe: getBoolean(payload.SettingBluetoothVibe),

    hourlyVibe: getMappedValue(HOURLY_VIBE_BY_ID, getNumber(payload.SettingHourlyVibe, 0), 'none'),

    weatherUnits: getBoolean(payload.SettingUseMetric) ? 'c' : 'f',

    batteryMeter: getBoolean(payload.SettingShowBatteryPct) ? 'icon-and-percent' : 'icon-only',
    autoBattery: !getBoolean(payload.SettingDisableAutobattery),

    altClockName: payload.SettingAltClockName || 'ALT',
    altClockOffset: getNumber(payload.SettingAltClockOffset, 0),

    healthActivityMetric: getBoolean(payload.SettingHealthUseDistance) ? 'distance' : 'steps',
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

  if (configData.colorSidebarText) {
    dict.SettingSidebarTextColor = parseInt(configData.colorSidebarText, 16);
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

  if (configData.disconnectIcon !== undefined) {
    dict.SettingDisconnectIcon = configData.disconnectIcon ? 1 : 0;
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

  if (configData.bottomBarWidget2Id !== undefined) {
    dict.SettingBottomBarWidget2ID = configData.bottomBarWidget2Id;
  }

  if (configData.bottomBarWidget3Id !== undefined) {
    dict.SettingBottomBarWidget3ID = configData.bottomBarWidget3Id;
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

  if (configData.wideSidebar !== undefined) {
    dict.SettingUseWideSidebar = configData.wideSidebar ? 1 : 0;
  }

  if (configData.weatherUnits) {
    dict.SettingUseMetric = configData.weatherUnits == 'c' ? 1 : 0;
  }

  if (configData.batteryMeter) {
    if (configData.batteryMeter == 'icon-and-percent') {
      dict.SettingShowBatteryPct = 1;
    } else if (configData.batteryMeter == 'icon-only') {
      dict.SettingShowBatteryPct = 0;
    }
  }

  if (configData.autoBattery !== undefined) {
    dict.SettingDisableAutobattery = configData.autoBattery ? 0 : 1;
  }

  if (configData.altClockName) {
    dict.SettingAltClockName = configData.altClockName;
  }

  if (configData.altClockOffset !== null) {
    dict.SettingAltClockOffset = parseInt(configData.altClockOffset, 10);
  }

  if (configData.decimalSeparator) {
    dict.SettingDecimalSep = configData.decimalSeparator;
  }

  if (configData.healthActivityMetric !== undefined) {
    dict.SettingHealthUseDistance = configData.healthActivityMetric == 'distance' ? 1 : 0;
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

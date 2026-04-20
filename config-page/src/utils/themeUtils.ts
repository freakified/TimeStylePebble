export interface Theme {
  name: string;
  settings: Record<string, string>;
}

/**
 * Transforms a day theme settings keys to their night theme counterparts.
 * e.g., SETTING_TIME_COLOR -> SETTING_NIGHT_TIME_COLOR
 */
export const toNightTheme = (theme: Theme): Theme => {
  const nightSettings: Record<string, string> = {};
  for (const [key, value] of Object.entries(theme.settings)) {
    if (key.startsWith('SETTING_')) {
      const nightKey = key.replace('SETTING_', 'SETTING_NIGHT_');
      nightSettings[nightKey] = value;
    } else {
      nightSettings[key] = value;
    }
  }
  return {
    ...theme,
    settings: nightSettings,
  };
};

/**
 * Combines light and dark themes into a single list, ensuring the first 
 * theme is marked as the default.
 * 
 * @param primaryThemes The first group of themes (e.g. light themes for day mode)
 * @param secondaryThemes The second group of themes (e.g. dark themes for day mode)
 * @param isNightMode Whether to transform keys to SETTING_NIGHT_ counterparts
 */
export const prepareThemes = (
  primaryThemes: Theme[],
  secondaryThemes: Theme[],
  isNightMode: boolean
): Theme[] => {
  let combined = [...primaryThemes, ...secondaryThemes];
  
  if (isNightMode) {
    combined = combined.map(toNightTheme);
  }

  return combined.map((theme, index) => {
    if (index === 0) {
      return {
        ...theme,
        name: `${theme.name} (Default)`,
      };
    }
    return theme;
  });
};

import { Settings } from '../context/types';
import { getActiveThemes, ThemeKind, ThemeSettings } from './themeUtils';
import { formFactor, getWidgetThemes, normalizeWidgetTheme } from './widgetThemes';

const COLOR_KEYS: (keyof Settings)[] = ['colorBackground', 'colorTime', 'colorSidebar'];

export const COLOR_THEMES: ThemeKind = {
  selectionKey: 'SETTING_THEME',
  storageKey: 'timestyle-saved-themes',
  savedNamePrefix: 'Saved Theme',
  presets: getActiveThemes,
  normalize: (raw) => {
    const settings: Record<string, string> = {};
    COLOR_KEYS.filter((key) => key in raw).forEach((key) => {
      settings[key] = String(raw[key]);
    });
    return Object.keys(settings).length > 0 ? (settings as ThemeSettings) : null;
  },
};

// widget slots mean different things on rect and round, and a theme
// filled for one screen size is wrong for another, so saved widget
// themes stay with the form factor they were made on
export const WIDGET_THEMES: ThemeKind = {
  selectionKey: 'SETTING_WIDGET_THEME',
  storageKey: 'timestyle-saved-widget-themes',
  savedNamePrefix: 'Saved Widgets',
  presets: getWidgetThemes,
  normalize: normalizeWidgetTheme,
  scope: formFactor,
};

export const THEME_KINDS = [COLOR_THEMES, WIDGET_THEMES];

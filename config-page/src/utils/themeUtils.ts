import { Capabilities, Settings } from '../context/types';
import { SavedTheme } from '../hooks/useSavedThemes';
import lightThemes from '../data/light-themes.json';
import darkThemes from '../data/dark-themes.json';
import lightThemesBw from '../data/light-themes-bw.json';
import darkThemesBw from '../data/dark-themes-bw.json';

/** The settings a theme holds: only its own kind's keys */
export type ThemeSettings = Partial<Settings>;

export interface Theme {
  name: string;
  settings: ThemeSettings;
}

/** The internal settings holding each picker's selection; never sent to the watch */
export type ThemeSelectionKey = 'SETTING_THEME' | 'SETTING_WIDGET_THEME';

/**
 * One kind of theme — colors, widgets — and everything the shared picker,
 * custom panel and saved-theme storage need to know about it.
 */
export interface ThemeKind {
  selectionKey: ThemeSelectionKey;
  /** localStorage key for this kind's saved themes */
  storageKey: string;
  /** saved themes are named "<prefix> 1", "<prefix> 2", ... */
  savedNamePrefix: string;
  /** the preset list for this watch, default first */
  presets: (capabilities: Capabilities) => Theme[];
  /**
   * This kind's settings picked out of anything theme-shaped (the live
   * settings, a saved theme, pasted JSON), coerced and cleaned up for this
   * watch. Null when there's nothing of this kind in it.
   */
  normalize: (raw: Record<string, unknown>, capabilities: Capabilities) => ThemeSettings | null;
  /**
   * Saved themes only show on watches with the same scope. Kinds without
   * one share their saved themes between every watch.
   */
  scope?: (capabilities: Capabilities) => string;
}

/**
 * Combines light and dark themes into a single list, ensuring the first
 * theme is marked as the default.
 *
 * @param primaryThemes The first group of themes (e.g. light themes)
 * @param secondaryThemes The second group of themes (e.g. dark themes)
 */
export const prepareThemes = (
  primaryThemes: Theme[],
  secondaryThemes: Theme[]
): Theme[] => {
  const combined = [...primaryThemes, ...secondaryThemes];

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

/** The color preset list for the watch's display: BW presets or color presets */
export const getActiveThemes = (capabilities: Capabilities): Theme[] =>
  capabilities.BW
    ? prepareThemes(lightThemesBw, darkThemesBw)
    : prepareThemes(lightThemes, darkThemes);

const themeMatches = (themeSettings: ThemeSettings, current: ThemeSettings) => {
  const keys = Object.keys(themeSettings) as (keyof Settings)[];
  return keys.length > 0 && keys.every((key) =>
    String(current[key]).toUpperCase() === String(themeSettings[key]).toUpperCase()
  );
};

/**
 * The picker id of the theme the current settings belong to: a saved
 * theme's id, a preset's index, or 'custom' when nothing matches. The
 * selection itself is never sent to the watch, so it's recovered from the
 * settings each time the page opens. The settings are normalized first,
 * so leftovers the watch never shows (a stale id in a slot this platform
 * doesn't render) don't stop a theme from matching.
 */
export const matchThemeId = (
  kind: ThemeKind,
  settings: Settings,
  capabilities: Capabilities,
  savedThemes: SavedTheme[]
): string => {
  const current = kind.normalize({ ...settings }, capabilities) ?? {};

  // saved themes come first, mirroring the picker's own lookup order
  const saved = savedThemes.find((t) => themeMatches(t.settings, current));
  if (saved) return saved.id;

  const index = kind.presets(capabilities).findIndex((t) => themeMatches(t.settings, current));
  return index >= 0 ? index.toString() : 'custom';
};

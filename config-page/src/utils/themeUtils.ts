import { Capabilities, Settings } from '../context/types';
import { SavedTheme } from '../hooks/useSavedThemes';
import lightThemes from '../data/light-themes.json';
import darkThemes from '../data/dark-themes.json';
import lightThemesBw from '../data/light-themes-bw.json';
import darkThemesBw from '../data/dark-themes-bw.json';

export interface Theme {
  name: string;
  settings: Record<string, string>;
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

/** The preset list for the watch's display: BW presets or color presets */
export const getActiveThemes = (capabilities: Capabilities): Theme[] =>
  capabilities.BW
    ? prepareThemes(lightThemesBw, darkThemesBw)
    : prepareThemes(lightThemes, darkThemes);

const themeMatches = (themeSettings: Record<string, string>, settings: Settings) => {
  const keys = Object.keys(themeSettings);
  return keys.length > 0 && keys.every((key) =>
    String(settings[key as keyof Settings]).toUpperCase() ===
    String(themeSettings[key]).toUpperCase()
  );
};

/**
 * The theme picker id whose colors match the current settings: a saved
 * theme's id, a preset's index, or 'custom' when nothing matches. The
 * selection itself is never sent to the watch, so it's recovered from the
 * colors each time the page opens.
 */
export const matchThemeId = (
  settings: Settings,
  themes: Theme[],
  savedThemes: SavedTheme[]
): string => {
  // saved themes come first, mirroring the picker's own lookup order
  const saved = savedThemes.find((t) => themeMatches(t.settings, settings));
  if (saved) return saved.id;

  const index = themes.findIndex((t) => themeMatches(t.settings, settings));
  return index >= 0 ? index.toString() : 'custom';
};

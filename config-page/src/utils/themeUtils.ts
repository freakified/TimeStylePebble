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

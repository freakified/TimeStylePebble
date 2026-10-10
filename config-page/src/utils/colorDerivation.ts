import { Settings } from '../context/types';

/*
 * Color derivation: themes store only three colors (background, time,
 * sidebar), and each region's text and icon colors are derived from them.
 * The watch does the real derivation (Settings_updateDynamicSettings in
 * src/c/settings.c); this mirrors it for the preview, so keep the two in
 * sync.
 */

/** regions at or below this luminance are dark: their icons draw as
 *  outlines rather than white badges */
const DARK_REGION_LUMINANCE = 0.03;

/** white text wins when its contrast ratio, times this, is at least
 *  black's: the screen renders mid-tones darker than sRGB would suggest */
const WHITE_TEXT_BIAS = 1.3;

/** below this contrast with its region, an accent gives way to the text color */
const MIN_ACCENT_CONTRAST = 2.5;

const normalize = (hex: string) => hex.replace('#', '').toUpperCase();

/** WCAG relative luminance of a 6-digit hex color */
export const luminance = (hex: string): number => {
  const h = normalize(hex);
  const [r, g, b] = [0, 2, 4].map((i) => {
    const c = parseInt(h.slice(i, i + 2), 16) / 255;
    return c <= 0.03928 ? c / 12.92 : ((c + 0.055) / 1.055) ** 2.4;
  });
  return 0.2126 * r + 0.7152 * g + 0.0722 * b;
};

export const contrastRatio = (a: string, b: string): number => {
  const [lo, hi] = [luminance(a), luminance(b)].sort((x, y) => x - y);
  return (hi + 0.05) / (lo + 0.05);
};

/** black or white, whichever reads best on `background` (biased towards white) */
const bestTextColor = (background: string): string =>
  contrastRatio(background, 'FFFFFF') * WHITE_TEXT_BIAS >= contrastRatio(background, '000000')
    ? 'FFFFFF'
    : '000000';

/** a region's colors, as 6-digit hex without '#' */
export interface RegionPalette {
  background: string;
  text: string;
  iconFill: string;
  iconStroke: string;
}

/** a region's text and icon colors, from its background and the two colors
 *  that could accent its icons (ties go to the first) */
const derivePalette = (background: string, accent1: string, accent2: string): RegionPalette => {
  const bg = normalize(background);
  const text = bestTextColor(bg);

  if (luminance(bg) > DARK_REGION_LUMINANCE) {
    return { background: bg, text, iconFill: 'FFFFFF', iconStroke: '000000' };
  }

  // on a dark region, icons are outlines in the more striking accent
  const accent = normalize(
    contrastRatio(accent2, bg) > contrastRatio(accent1, bg) ? accent2 : accent1);
  return {
    background: bg,
    text,
    iconFill: bg,
    iconStroke: contrastRatio(accent, bg) >= MIN_ACCENT_CONTRAST ? accent : text,
  };
};

/** the sidebar wears the sidebar color, accented with the time or
 *  background color */
export const sidebarPalette = (s: Settings): RegionPalette =>
  derivePalette(s.colorSidebar, s.colorTime, s.colorBackground);

/** "dark" regions (the bottom bar) draw on the watchface background,
 *  accented with the time or sidebar color */
export const darkRegionPalette = (s: Settings): RegionPalette =>
  derivePalette(s.colorBackground, s.colorTime, s.colorSidebar);

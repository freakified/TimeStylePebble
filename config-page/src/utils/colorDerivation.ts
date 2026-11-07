import { Settings } from '../context/types';

/*
 * Color derivation: themes store only three colors (background, time,
 * sidebar), and each region's text and icon colors are derived from them.
 * The watch does the real derivation (Settings_updateDynamicSettings in
 * src/c/settings.c); this mirrors it for the preview, so keep the two in
 * sync.
 */

/** RLCD gamma exponent for Pebble's 2-bit color channels */
const RLCD_GAMMA = 1.6;

/** regions at or below this luminance are dark: their icons draw as
 *  outlines rather than white badges */
const DARK_REGION_LUMINANCE = 0.18;

/** white text wins when its contrast ratio, times this, is at least
 *  black's: the screen renders mid-tones darker than sRGB would suggest,
 *  especially saturated colors where at least one subpixel is closed */
const WHITE_TEXT_BIAS = 3.0;
const PASTEL_WHITE_TEXT_BIAS = 1.3;

/** below this contrast with its region, an accent gives way to the text color */
const MIN_ACCENT_CONTRAST = 2.9;

const normalize = (hex: string) => hex.replace('#', '').toUpperCase();

const channels = (hex: string): [number, number, number] => {
  const h = normalize(hex);
  return [
    parseInt(h.slice(0, 2), 16),
    parseInt(h.slice(2, 4), 16),
    parseInt(h.slice(4, 6), 16),
  ];
};

/** RLCD relative luminance of a 6-digit hex color: Pebble's reflective
 *  color filters have a 1.6 gamma curve and broader blue/red passbands
 *  than sRGB (0.25 R + 0.61 G + 0.14 B) */
export const luminance = (hex: string): number => {
  const [r, g, b] = channels(hex).map((c) => (c / 255) ** RLCD_GAMMA);
  return 0.25 * r + 0.61 * g + 0.14 * b;
};

export const contrastRatio = (a: string, b: string): number => {
  const [lo, hi] = [luminance(a), luminance(b)].sort((x, y) => x - y);
  return (hi + 0.05) / (lo + 0.05);
};

/** how much lighter `accent` is than a dark `background` */
const accentContrast = (accent: string, background: string): number =>
  (luminance(accent) + 0.05) / (luminance(background) + 0.05);

/** black or white, whichever reads best on `background` (biased towards white) */
const bestTextColor = (background: string): string => {
  const [r, g, b] = channels(background);
  const bias = r > 0 && g > 0 && b > 0 ? PASTEL_WHITE_TEXT_BIAS : WHITE_TEXT_BIAS;
  return contrastRatio(background, 'FFFFFF') * bias >= contrastRatio(background, '000000')
    ? 'FFFFFF'
    : '000000';
};

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

  // on a dark region, icons are outlines in the brighter accent
  const accent = normalize(
    accentContrast(accent2, bg) > accentContrast(accent1, bg) ? accent2 : accent1);
  return {
    background: bg,
    text,
    iconFill: bg,
    iconStroke: accentContrast(accent, bg) >= MIN_ACCENT_CONTRAST ? accent : text,
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

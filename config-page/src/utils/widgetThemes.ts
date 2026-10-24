import { Capabilities, Settings } from '../context/types';
import { FormFactor, RectWidgets, RoundWidgets, WidgetPreset, WIDGET_PRESETS } from '../data/widgetThemes';
import { getWidgetOptions } from '../data/widgetTypes';
import { getDefaultSettings } from './defaultSettings';
import { hiddenParts } from './widgetParts';
import { prepareThemes, Theme, ThemeSettings } from './themeUtils';
import {
  ALL_SLOT_KEYS,
  packWidgetLists,
  readWidgetLists,
  regionFitChecks,
  regionSlotCaps,
  SlotKey,
  WIDGET_REGIONS,
  WidgetLists,
} from './widgetSlots';

/** The four screen sizes widget themes are laid out for, and saved under */
export const formFactor = (capabilities: Capabilities): FormFactor =>
  capabilities.ROUND
    ? capabilities.DISPLAY_260x260_ROUND ? 'large-round' : 'small-round'
    : capabilities.DISPLAY_200x228 ? 'large-rect' : 'small-rect';

const toLists = (widgets: RectWidgets | RoundWidgets): WidgetLists =>
  'sidebar' in widgets
    ? { primary: widgets.sidebar, secondary: widgets.bar ?? [], tertiary: [] }
    : { primary: widgets.left, secondary: widgets.right, tertiary: widgets.bar ?? [] };

const supportedWidgets = (capabilities: Capabilities) =>
  new Set(getWidgetOptions(capabilities).map((option) => option.value));

type PartMasks = Pick<Settings, 'widgetHideMarker' | 'widgetHideDetail'>;

const maskOf = (ids: number[]) => ids.reduce((mask, id) => (mask | (1 << id)) >>> 0, 0);

/**
 * The hidden-part masks as the watch reads them (hidden_parts()), cut
 * down to the widgets actually placed. A widget theme's masks then only
 * say what its own widgets look like: a bit left over on a widget that
 * isn't showing can't stop the theme from matching, and applying the
 * theme clears it.
 */
const placedPartMasks = (masks: Partial<PartMasks>, lists: WidgetLists): PartMasks => {
  const placed = [...lists.primary, ...lists.secondary, ...lists.tertiary];
  const parts = placed.map((id) => ({ id, hidden: hiddenParts(id, masks as Settings) }));
  return {
    widgetHideMarker: maskOf(parts.filter((p) => p.hidden.marker).map((p) => p.id)),
    widgetHideDetail: maskOf(parts.filter((p) => p.hidden.detail).map((p) => p.id)),
  };
};

/**
 * A preset's slot settings on this watch, or null when the watch can't
 * show it at all.
 *
 * Each region takes its candidates in order (the preset's essential
 * widgets first), skipping widgets the watch
 * can't show or that another region already holds, until it reaches its
 * slot cap. A candidate that would push the region over its fit is
 * skipped rather than ending the fill, so a smaller widget further down
 * the list can still take the room.
 *
 * Fit is measured against the default display settings, not the user's:
 * a preset then resolves the same way however the font size and sidebar
 * width are set, so the picker keeps recognizing it when they change. The
 * preset's own hidden parts do count, since they're part of the preset.
 */
export const resolveWidgetPreset = (
  preset: WidgetPreset,
  capabilities: Capabilities
): ThemeSettings | null => {
  if (preset.requires?.some((capability) => !capabilities[capability])) {
    return null;
  }

  const exact = preset.exact?.[formFactor(capabilities)];
  const candidates = toLists(exact ?? (capabilities.ROUND ? preset.round : preset.rect));

  const supported = supportedWidgets(capabilities);
  const caps = regionSlotCaps(capabilities);
  const fits = regionFitChecks(capabilities);
  const masks = {
    widgetHideMarker: maskOf(preset.hide?.marker ?? []),
    widgetHideDetail: maskOf(preset.hide?.detail ?? []),
  };
  const reference = { ...getDefaultSettings(capabilities), ...masks };

  const placed = new Set<number>();
  const lists: WidgetLists = { primary: [], secondary: [], tertiary: [] };

  // essential widgets get first claim on the room, but every region still
  // shows its widgets in the order the preset lists them
  const essential = new Set(preset.essential ?? []);

  WIDGET_REGIONS.forEach((region) => {
    const listed = candidates[region];
    const fillOrder = [
      ...listed.filter((id) => essential.has(id)),
      ...listed.filter((id) => !essential.has(id)),
    ];

    for (const id of fillOrder) {
      if (lists[region].length >= caps[region]) break;
      if (!supported.has(id) || placed.has(id)) continue;

      const next = listed.filter((other) => other === id || lists[region].includes(other));
      if (!exact && fits[region](next, reference, capabilities).state === 'over') continue;

      lists[region] = next;
      placed.add(id);
    }
  });

  return { ...packWidgetLists(lists, capabilities), ...placedPartMasks(masks, lists) };
};

/** The widget preset list for this watch, default first */
export const getWidgetThemes = (capabilities: Capabilities): Theme[] =>
  prepareThemes(
    WIDGET_PRESETS
      .map((preset) => ({ name: preset.name, settings: resolveWidgetPreset(preset, capabilities) }))
      .filter((theme): theme is Theme => theme.settings !== null),
    []
  );

/**
 * The slot settings out of anything theme-shaped, as this watch would
 * show them: packed, zero-padded, and without widgets it can't show or
 * duplicates, plus the hidden-part masks of the widgets left. Null when
 * there's no slot setting in it at all.
 */
export const normalizeWidgetTheme = (
  raw: Record<string, unknown>,
  capabilities: Capabilities
): ThemeSettings | null => {
  if (!ALL_SLOT_KEYS.some((key) => key in raw)) {
    return null;
  }

  const slots = {} as Pick<Settings, SlotKey>;
  ALL_SLOT_KEYS.forEach((key) => {
    const id = Number(raw[key] ?? 0);
    slots[key] = Number.isInteger(id) ? id : 0;
  });

  const supported = supportedWidgets(capabilities);
  const seen = new Set<number>();
  const keep = (id: number) => {
    if (!supported.has(id) || seen.has(id)) return false;
    seen.add(id);
    return true;
  };

  const read = readWidgetLists(slots, capabilities);
  const lists = {
    primary: read.primary.filter(keep),
    secondary: read.secondary.filter(keep),
    tertiary: read.tertiary.filter(keep),
  };
  const masks = {
    widgetHideMarker: Number(raw.widgetHideMarker ?? 0) || 0,
    widgetHideDetail: Number(raw.widgetHideDetail ?? 0) || 0,
  };
  return { ...packWidgetLists(lists, capabilities), ...placedPartMasks(masks, lists) };
};

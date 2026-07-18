import { Capabilities } from '../context/types';
import { ALL_WIDGET_OPTIONS } from '../data/widgetTypes';

// The unit budget is advisory: the config page warns when a region is
// over budget, but never blocks — the watch degrades deterministically
// (squeezed widths, text fallbacks) if the user goes over anyway.
//
// Capacities are calibrated against the watch's real pixel budgets
// (region sizes in sidebar_layout.h, widget measure code), quantized to
// the same coarse units as widgetTypes.ts. Large sidebar fonts grow
// every widget, so they cost the region one unit of capacity.

export const sidebarCapacity = (capabilities: Capabilities, largeFonts: boolean): number =>
  (capabilities.EMERY ? 8 : 6) - (largeFonts ? 1 : 0);

export const inlineBarCapacity = (capabilities: Capabilities, largeFonts: boolean): number =>
  (capabilities.EMERY ? 5 : 4) - (largeFonts ? 1 : 0);

const widgetsById = new Map(ALL_WIDGET_OPTIONS.map((w) => [w.value, w]));

export const sidebarUnitsUsed = (widgetIds: number[]): number =>
  widgetIds.reduce((sum, id) => sum + (widgetsById.get(id)?.vUnits ?? 0), 0);

export const inlineBarUnitsUsed = (widgetIds: number[]): number =>
  widgetIds.reduce((sum, id) => sum + (widgetsById.get(id)?.hUnits ?? 0), 0);

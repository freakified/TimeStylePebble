import { ALL_WIDGET_OPTIONS } from '../data/widgetTypes';

// The vertical sidebar keeps a coarse unit model: widget heights don't
// depend on their content the way widths do, and the sidebar is always
// full screen height (the status bar yields its corner rather than
// shortening it), so a small integer scale is well calibrated here.
//
// The inline bottom bar does NOT use this -- it is measured in real watch
// pixels by assessInlineBar() in widgetMetrics.ts, because its capacity
// moves with the sidebar width, the font size, and the inter-widget gaps.

const widgetsById = new Map(ALL_WIDGET_OPTIONS.map((w) => [w.value, w]));

export const sidebarUnitsUsed = (widgetIds: number[]): number =>
  widgetIds.reduce((sum, id) => sum + (widgetsById.get(id)?.vUnits ?? 0), 0);

import { Layout } from '../context/types';

/*
 * A layout says only where the vertical sidebar goes. Whether there's a
 * bottom bar is not a layout choice: the bar exists exactly when its slots
 * hold a widget, so ask the widget list, not the layout — see
 * barHasWidgets() below and sidebar_buildRegions() in src/c/sidebar.c,
 * which decides the same way.
 */

export const layoutHasSidebar = (layout: Layout): boolean =>
  layout !== 'no-sidebar';

export const layoutSidebarOnLeft = (layout: Layout): boolean =>
  layout === 'left-sidebar';

/** The bar is on screen iff something is in it. */
export const barHasWidgets = (barWidgetIds: number[]): boolean =>
  barWidgetIds.some((id) => id !== 0);

// the standalone bar uses the tall "stacked" style; alongside a sidebar
// it's the narrow inline bar. The style follows from the sidebar's
// presence rather than being recorded separately.
export const barIsStacked = (layout: Layout): boolean =>
  !layoutHasSidebar(layout);

/** Rendered bar slots: HREGION_SLOT_CAP_STACKED / _INLINE in sidebar_layout.h. */
export const barSlotCap = (layout: Layout, isEmery: boolean): number =>
  barIsStacked(layout) ? 4 : isEmery ? 3 : 2;

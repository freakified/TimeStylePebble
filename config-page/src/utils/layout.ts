import { Layout } from '../context/types';

/*
 * A layout says only which side the vertical sidebar goes on. Whether
 * there's a bottom bar is not a layout choice: the bar exists exactly
 * when its slots hold a widget, so ask the widget list, not the layout —
 * see barHasWidgets() below and sidebar_buildRegions() in src/c/sidebar.c,
 * which decides the same way.
 */

export const layoutSidebarOnLeft = (layout: Layout): boolean =>
  layout === 'left-sidebar';

/** The bar is on screen iff something is in it. */
export const barHasWidgets = (barWidgetIds: number[]): boolean =>
  barWidgetIds.some((id) => id !== 0);

/** Rendered bar slots: HREGION_SLOT_CAP in sidebar_layout.h. */
export const barSlotCap = (isEmery: boolean): number => (isEmery ? 3 : 2);

/*
 * Slot math for the widget selector's two packed lists.
 *
 * Both regions are edited as one unit: a widget lives in exactly one place,
 * so every edit rewrites the sidebar list AND the bar list, and the caller
 * commits them together. Keeping the arithmetic here (rather than inside
 * the component's closures) is what makes the drop-index behavior legible.
 */

export type WidgetRegion = 'sidebar' | 'bar';

export interface WidgetLists {
    sidebar: number[];
    bar: number[];
}

export type DropPosition = 'before' | 'after';

/**
 * Move `widgetId` into `region`, positioned against the widget it was
 * dropped on (`targetWidgetId`; null appends). Covers both a reorder within
 * a region and a move between them — they are the same operation.
 *
 * The drop point is resolved against the list the widget has ALREADY left.
 * Dragging a row downward past its own position otherwise lands one slot
 * short of where the drop indicator promised, because removing the row
 * shifts every later row up by one.
 */
export const moveWidget = (
    lists: WidgetLists,
    widgetId: number,
    region: WidgetRegion,
    targetWidgetId: number | null,
    dropPosition: DropPosition
): WidgetLists => {
    // dropped on itself: nothing moves. Without this the widget is filtered
    // out, its own target key no longer resolves, and it lands at the end.
    if (targetWidgetId === widgetId) {
        return lists;
    }

    const next: WidgetLists = {
        sidebar: lists.sidebar.filter((id) => id !== widgetId),
        bar: lists.bar.filter((id) => id !== widgetId),
    };
    const list = next[region];

    const at = targetWidgetId === null ? -1 : list.indexOf(targetWidgetId);
    const index = at < 0 ? list.length : dropPosition === 'after' ? at + 1 : at;

    list.splice(index, 0, widgetId);
    return next;
};

/** Place `widgetId` in one region's slot `index`, dropping it from the other. */
export const setWidgetAt = (
    lists: WidgetLists,
    widgetId: number,
    region: WidgetRegion,
    index: number
): WidgetLists => {
    const sidebar = [...lists.sidebar];
    const bar = [...lists.bar];
    (region === 'sidebar' ? sidebar : bar)[index] = widgetId;

    // a widget lives in one place: drop any other occurrence
    return {
        sidebar: sidebar.filter((id, i) => id !== widgetId || (region === 'sidebar' && i === index)),
        bar: bar.filter((id, i) => id !== widgetId || (region === 'bar' && i === index)),
    };
};

/** Append `widgetId` to a region, dropping it from wherever it was. */
export const addWidget = (
    lists: WidgetLists,
    widgetId: number,
    region: WidgetRegion
): WidgetLists => {
    const next: WidgetLists = {
        sidebar: lists.sidebar.filter((id) => id !== widgetId),
        bar: lists.bar.filter((id) => id !== widgetId),
    };
    next[region].push(widgetId);
    return next;
};

/** Drop the widget at `index` from one region, leaving the other untouched. */
export const removeWidgetAt = (
    lists: WidgetLists,
    region: WidgetRegion,
    index: number
): WidgetLists => ({
    sidebar: region === 'sidebar' ? lists.sidebar.filter((_, i) => i !== index) : lists.sidebar,
    bar: region === 'bar' ? lists.bar.filter((_, i) => i !== index) : lists.bar,
});

/*
 * Slot math for the widget selector's packed lists.
 *
 * The widget regions are named for their rank rather than their shape:
 * on rect the primary is the sidebar and the secondary the bottom bar
 * (there is no tertiary), on round they are the left and right arcs and
 * the tertiary is the single-widget bottom bar.
 *
 * All regions are edited as one unit: a widget lives in exactly one place,
 * so every edit rewrites EVERY list, and the caller commits them together.
 * Keeping the arithmetic here (rather than inside the component's closures)
 * is what makes the drop-index behavior legible.
 */

export const WIDGET_REGIONS = ['primary', 'secondary', 'tertiary'] as const;

export type WidgetRegion = typeof WIDGET_REGIONS[number];

export type WidgetLists = Record<WidgetRegion, number[]>;

/** Every list with `widgetId` dropped, so it can be placed somewhere new. */
const without = (lists: WidgetLists, widgetId: number): WidgetLists => ({
    primary: lists.primary.filter((id) => id !== widgetId),
    secondary: lists.secondary.filter((id) => id !== widgetId),
    tertiary: lists.tertiary.filter((id) => id !== widgetId),
});

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

    const next = without(lists, widgetId);
    const list = next[region];

    const at = targetWidgetId === null ? -1 : list.indexOf(targetWidgetId);
    const index = at < 0 ? list.length : dropPosition === 'after' ? at + 1 : at;

    list.splice(index, 0, widgetId);
    return next;
};

/** Place `widgetId` in one region's slot `index`, dropping it from the others. */
export const setWidgetAt = (
    lists: WidgetLists,
    widgetId: number,
    region: WidgetRegion,
    index: number
): WidgetLists => {
    // a widget lives in one place: drop any other occurrence
    const keep = (r: WidgetRegion) => {
        const list = [...lists[r]];
        if (r === region) list[index] = widgetId;
        return list.filter((id, i) => id !== widgetId || (r === region && i === index));
    };

    return { primary: keep('primary'), secondary: keep('secondary'), tertiary: keep('tertiary') };
};

/** Append `widgetId` to a region, dropping it from wherever it was. */
export const addWidget = (
    lists: WidgetLists,
    widgetId: number,
    region: WidgetRegion
): WidgetLists => {
    const next = without(lists, widgetId);
    next[region].push(widgetId);
    return next;
};

/** Drop the widget at `index` from one region, leaving the others untouched. */
export const removeWidgetAt = (
    lists: WidgetLists,
    region: WidgetRegion,
    index: number
): WidgetLists => ({
    ...lists,
    [region]: lists[region].filter((_, i) => i !== index),
});

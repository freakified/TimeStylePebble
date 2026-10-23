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

import { Capabilities, Settings } from '../context/types';
import { ARC_SLOT_CAP, barSlotCap } from './layout';
import {
    assessArc,
    assessInlineBar,
    assessRoundBar,
    assessSidebar,
    FitResult,
} from './widgetMetrics';

export const WIDGET_REGIONS = ['primary', 'secondary', 'tertiary'] as const;

export type WidgetRegion = typeof WIDGET_REGIONS[number];

export type WidgetLists = Record<WidgetRegion, number[]>;

const SIDEBAR_SLOT_KEYS = [
    'widget0Id',
    'widget1Id',
    'widget2Id',
    'widget3Id',
    'widget4Id',
    'widget5Id',
] as const;
const BAR_SLOT_KEYS = [
    'bottomBarWidget0Id',
    'bottomBarWidget1Id',
    'bottomBarWidget2Id',
] as const;

export type SlotKey = typeof SIDEBAR_SLOT_KEYS[number] | typeof BAR_SLOT_KEYS[number];

/** Every wire slot key, on every platform. */
export const ALL_SLOT_KEYS: readonly SlotKey[] = [...SIDEBAR_SLOT_KEYS, ...BAR_SLOT_KEYS];

/*
 * Which wire slots each region owns. Rect keeps the two key groups apart
 * — sidebar keys and bar keys — and has no tertiary region. Round's two
 * arcs split the sidebar keys down the middle instead (slots 0..2 are the
 * left arc, 3..5 the right), and its bar takes the bar keys.
 */
export const regionSlotKeys = (
    capabilities: Capabilities
): Record<WidgetRegion, readonly SlotKey[]> =>
    capabilities.ROUND
        ? {
            primary: SIDEBAR_SLOT_KEYS.slice(0, ARC_SLOT_CAP),
            secondary: SIDEBAR_SLOT_KEYS.slice(ARC_SLOT_CAP, ARC_SLOT_CAP * 2),
            tertiary: BAR_SLOT_KEYS,
        }
        : { primary: SIDEBAR_SLOT_KEYS, secondary: BAR_SLOT_KEYS, tertiary: [] };

/** How many of each region's slots the watch actually renders (the caps mirror sidebar_layout.h). */
export const regionSlotCaps = (capabilities: Capabilities): Record<WidgetRegion, number> =>
    capabilities.ROUND
        ? { primary: ARC_SLOT_CAP, secondary: ARC_SLOT_CAP, tertiary: barSlotCap(capabilities) }
        : {
            primary: capabilities.EMERY ? 6 : 4,
            secondary: barSlotCap(capabilities),
            tertiary: 0,
        };

type FitCheck = (ids: number[], settings: Settings, capabilities: Capabilities) => FitResult;

/** How each region measures whether its widgets fit. Rect's tertiary is never shown. */
export const regionFitChecks = (capabilities: Capabilities): Record<WidgetRegion, FitCheck> =>
    capabilities.ROUND
        ? { primary: assessArc, secondary: assessArc, tertiary: assessRoundBar }
        : { primary: assessSidebar, secondary: assessInlineBar, tertiary: assessInlineBar };

/** The packed widget list of every region, read from the slots the watch renders. */
export const readWidgetLists = (
    settings: Pick<Settings, SlotKey>,
    capabilities: Capabilities
): WidgetLists => {
    const keys = regionSlotKeys(capabilities);
    const caps = regionSlotCaps(capabilities);
    const read = (region: WidgetRegion) =>
        keys[region]
            .slice(0, caps[region])
            .map((key) => settings[key])
            .filter((id) => id !== 0);

    return { primary: read('primary'), secondary: read('secondary'), tertiary: read('tertiary') };
};

/**
 * Every region written back packed, zero-padding every wire slot so stale
 * ids can't hide in slots the current layout doesn't show. Together the
 * key groups cover every slot key on this platform, so the result is a
 * complete set of slot settings, ready to land in one state update.
 */
export const packWidgetLists = (
    lists: WidgetLists,
    capabilities: Capabilities
): Pick<Settings, SlotKey> => {
    const keys = regionSlotKeys(capabilities);
    const packed = {} as Pick<Settings, SlotKey>;
    WIDGET_REGIONS.forEach((region) => {
        keys[region].forEach((key, i) => {
            packed[key] = lists[region][i] !== undefined ? lists[region][i] : 0;
        });
    });
    return packed;
};

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

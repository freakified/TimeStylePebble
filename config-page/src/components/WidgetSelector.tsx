import React from 'react';
import { WatchPreview } from './WatchPreview';
import { useCapabilities, useConfig } from '../context/PebbleConfigContext';
import { getWidgetOptions } from '../data/widgetTypes';
import { ARC_SLOT_CAP, barSlotCap } from '../utils/layout';
import { sidebarUnitsUsed } from '../utils/budget';
import { assessArc, assessInlineBar, assessSidebar, FitResult } from '../utils/widgetMetrics';
import {
    addWidget as addWidgetToRegion,
    moveWidget,
    removeWidgetAt,
    setWidgetAt,
    DropPosition,
    WidgetLists,
    WidgetRegion,
} from '../utils/widgetSlots';
import { Settings } from '../context/types';

/*
 * Every platform has two widget regions — a sidebar and a bottom bar on
 * rect, the left and right arcs on round — and each is presented as a
 * packed widget list: N rows for the N configured widgets, plus an "add
 * widget" dropdown and per-row remove buttons. On the watch the sidebar
 * pins its first widget to the top and its last to the bottom, while the
 * bar and the arcs center their widgets as one group, so an "empty slot"
 * has no visual meaning either way — EMPTY (0) is only wire padding,
 * written back when the lists are packed into the slot keys.
 *
 * Row ORDER is meaningful (top-to-bottom in the sidebar and the arcs,
 * left-to-right in the bar), so rows drag by their handle: within a
 * region to reorder, and across regions to move a widget between them.
 * Every drop lands in one commit() so a widget is never briefly in two
 * places.
 *
 * The drag is hand-rolled on Pointer Events rather than borrowed from a
 * library. react-aria's drag and drop — the obvious candidate, already a
 * dependency — cannot work here: it drives pointer dragging through native
 * HTML5 drag events, which mobile browsers never fire from touch, and its
 * drag handle explicitly ignores any press that isn't keyboard or screen
 * reader (useDrag.mjs: `if (e.pointerType !== 'keyboard' && e.pointerType
 * !== 'virtual') return`). Since this page lives in a phone webview, that
 * is the one platform that has to work. Pointer Events give mouse and
 * touch a single code path, and the lists are small and plainly stacked,
 * so the hit-testing stays simple.
 */

const ALL_SIDEBAR_SLOT_KEYS = [
    'widget0Id',
    'widget1Id',
    'widget2Id',
    'widget3Id',
    'widget4Id',
    'widget5Id',
] as const;
const ALL_BAR_SLOT_KEYS = [
    'bottomBarWidget0Id',
    'bottomBarWidget1Id',
    'bottomBarWidget2Id',
] as const;

const WIDGET_CATEGORY_ORDER = ['General', 'Time & Date', 'Weather', 'Health'] as const;

/** Where a drag would land: null while the pointer is over nowhere useful. */
interface DropTarget {
    region: WidgetRegion;
    /** the row the widget would be placed against; null appends to an empty region */
    targetWidgetId: number | null;
    position: DropPosition;
}

interface DragState {
    /** the region the widget started in */
    region: WidgetRegion;
    widgetId: number;
    pointerId: number;
    startY: number;
    /** how far the lifted row has travelled, for its transform */
    dy: number;
    drop: DropTarget | null;
}

export const WidgetSelector: React.FC = () => {
    const capabilities = useCapabilities();
    const { settings, updateSettings } = useConfig();
    const widgetOptions = getWidgetOptions(capabilities);

    const controlsRef = React.useRef<HTMLDivElement>(null);
    const [drag, setDrag] = React.useState<DragState | null>(null);
    // a widget moved by keyboard changes rows (and across regions, DOM
    // nodes), so focus has to be put back on its handle afterwards
    const refocusRef = React.useRef<number | null>(null);

    /*
     * Which wire slots each region owns, and how many of them the watch
     * actually renders (the caps mirror sidebar_layout.h). Rect keeps the
     * two key groups apart — sidebar keys and bar keys. Round has no bar,
     * so its two arcs split the sidebar keys down the middle instead:
     * slots 0..2 are the left arc, 3..5 the right.
     */
    const primaryKeys = capabilities.ROUND
        ? ALL_SIDEBAR_SLOT_KEYS.slice(0, ARC_SLOT_CAP)
        : ALL_SIDEBAR_SLOT_KEYS;
    const secondaryKeys = capabilities.ROUND
        ? ALL_SIDEBAR_SLOT_KEYS.slice(ARC_SLOT_CAP, ARC_SLOT_CAP * 2)
        : ALL_BAR_SLOT_KEYS;

    const primarySlotCount = capabilities.ROUND
        ? ARC_SLOT_CAP
        : capabilities.EMERY ? 6 : 4;
    const secondarySlotCount = capabilities.ROUND
        ? ARC_SLOT_CAP
        : barSlotCap(capabilities.EMERY);

    type SlotKey = typeof ALL_SIDEBAR_SLOT_KEYS[number] | typeof ALL_BAR_SLOT_KEYS[number];

    const readIds = (keys: readonly SlotKey[], count: number) =>
        keys.slice(0, count).map((key) => settings[key]).filter((id) => id !== 0);

    const primaryIds = readIds(primaryKeys, primarySlotCount);
    const secondaryIds = readIds(secondaryKeys, secondarySlotCount);

    const lists: WidgetLists = { primary: primaryIds, secondary: secondaryIds };
    const idsOf = (region: WidgetRegion) => (region === 'primary' ? primaryIds : secondaryIds);
    const capOf = (region: WidgetRegion) =>
        region === 'primary' ? primarySlotCount : secondarySlotCount;
    // regions in the order they appear on screen, which is the order the
    // drag hit-test and the keyboard nudge both walk
    const shownRegions: WidgetRegion[] = ['primary', 'secondary'];

    // write both regions back packed, zero-padding every wire slot so
    // stale ids can't hide in slots the current layout doesn't show.
    // Together the two key groups cover every slot key on this platform,
    // and both land in one state update, so a widget is never momentarily
    // in two places (or neither) while moving between them.
    //
    // Nothing else needs writing: the bottom bar is on exactly when its
    // slots hold a widget, so filling or emptying them is the whole of
    // turning the bar on and off, on the watch as well as here.
    const commit = (next: WidgetLists) => {
        const partial: Partial<Settings> = {};
        primaryKeys.forEach((key, i) => {
            partial[key] = next.primary[i] !== undefined ? next.primary[i] : 0;
        });
        secondaryKeys.forEach((key, i) => {
            partial[key] = next.secondary[i] !== undefined ? next.secondary[i] : 0;
        });
        updateSettings(partial);
    };

    const changeWidget = (region: WidgetRegion, index: number, widgetId: number) =>
        commit(setWidgetAt(lists, widgetId, region, index));

    const addWidget = (region: WidgetRegion, widgetId: number) =>
        commit(addWidgetToRegion(lists, widgetId, region));

    const removeWidget = (region: WidgetRegion, index: number) =>
        commit(removeWidgetAt(lists, region, index));

    /* ── Dragging ──────────────────────────────────────────────────────
     *
     * Geometry is read from the DOM on every pointermove rather than
     * snapshotted on pointerdown: the rows don't move during a drag (the
     * lifted row is only transformed), but the page can still scroll under
     * the pointer, and live rects stay correct through that.
     */

    /** Which region's controls the pointer is over, clamped to the ends. */
    const regionAt = (y: number): HTMLElement | null => {
        const els = [...(controlsRef.current?.querySelectorAll<HTMLElement>('[data-region]') ?? [])];
        if (els.length === 0) return null;

        const hit = els.find((el) => {
            const r = el.getBoundingClientRect();
            return y >= r.top && y <= r.bottom;
        });
        if (hit) return hit;

        // above the first region or below the last one: clamp to the nearer
        return y < els[0].getBoundingClientRect().top ? els[0] : els[els.length - 1];
    };

    const dropTargetAt = (y: number, dragged: DragState): DropTarget | null => {
        const regionEl = regionAt(y);
        if (!regionEl) return null;

        const region = regionEl.dataset.region as WidgetRegion;

        // a move into the other region grows it by one; refuse at the cap
        // rather than dropping the widget on the floor. No indicator is
        // drawn, which is what tells the user it won't land there.
        if (region !== dragged.region && idsOf(region).length >= capOf(region)) {
            return null;
        }

        const rows = [...regionEl.querySelectorAll<HTMLElement>('[data-widget-id]')]
            .map((el) => ({ widgetId: Number(el.dataset.widgetId), rect: el.getBoundingClientRect() }))
            .filter((row) => row.widgetId !== dragged.widgetId);

        if (rows.length === 0) {
            return { region, targetWidgetId: null, position: 'before' };
        }

        // the row's midpoint is the tipping point between landing above it
        // and landing below it
        const passed = rows.filter((row) => y > row.rect.top + row.rect.height / 2).length;
        return passed >= rows.length
            ? { region, targetWidgetId: rows[rows.length - 1].widgetId, position: 'after' }
            : { region, targetWidgetId: rows[passed].widgetId, position: 'before' };
    };

    const onHandlePointerDown = (e: React.PointerEvent, region: WidgetRegion, widgetId: number) => {
        if (e.button !== 0 && e.pointerType === 'mouse') return;

        // capture on the handle so the drag survives the pointer leaving it
        e.currentTarget.setPointerCapture(e.pointerId);
        e.preventDefault();

        setDrag({ region, widgetId, pointerId: e.pointerId, startY: e.clientY, dy: 0, drop: null });
    };

    const onHandlePointerMove = (e: React.PointerEvent) => {
        if (!drag || e.pointerId !== drag.pointerId) return;

        const next: DragState = { ...drag, dy: e.clientY - drag.startY };
        next.drop = dropTargetAt(e.clientY, next);
        setDrag(next);
    };

    const onHandlePointerUp = (e: React.PointerEvent) => {
        if (!drag || e.pointerId !== drag.pointerId) return;

        if (drag.drop) {
            commit(moveWidget(
                lists,
                drag.widgetId,
                drag.drop.region,
                drag.drop.targetWidgetId,
                drag.drop.position
            ));
        }
        setDrag(null);
    };

    const onHandlePointerCancel = (e: React.PointerEvent) => {
        if (!drag || e.pointerId !== drag.pointerId) return;
        setDrag(null);
    };

    /**
     * Keyboard equivalent: with a handle focused, up/down moves its row one
     * slot, crossing into the adjacent region at either end.
     */
    const nudge = (region: WidgetRegion, widgetId: number, delta: -1 | 1) => {
        const ids = idsOf(region);
        const to = ids.indexOf(widgetId) + delta;

        if (to >= 0 && to < ids.length) {
            commit(moveWidget(lists, widgetId, region, ids[to], delta > 0 ? 'after' : 'before'));
            refocusRef.current = widgetId;
            return;
        }

        const nextRegion = shownRegions[shownRegions.indexOf(region) + delta];
        if (!nextRegion || idsOf(nextRegion).length >= capOf(nextRegion)) return;

        const nextIds = idsOf(nextRegion);
        const target = delta > 0 ? nextIds[0] : nextIds[nextIds.length - 1];
        commit(moveWidget(
            lists,
            widgetId,
            nextRegion,
            target !== undefined ? target : null,
            delta > 0 ? 'before' : 'after'
        ));
        refocusRef.current = widgetId;
    };

    const onHandleKeyDown = (e: React.KeyboardEvent, region: WidgetRegion, widgetId: number) => {
        if (e.key !== 'ArrowUp' && e.key !== 'ArrowDown') return;
        e.preventDefault();
        nudge(region, widgetId, e.key === 'ArrowDown' ? 1 : -1);
    };

    React.useEffect(() => {
        const widgetId = refocusRef.current;
        if (widgetId === null) return;
        refocusRef.current = null;
        controlsRef.current
            ?.querySelector<HTMLElement>(`[data-widget-id="${widgetId}"] .widget-slot-drag`)
            ?.focus();
    });

    /* ── Rendering ── */

    const usedIds = new Set([...primaryIds, ...secondaryIds]);
    const addableOptions = widgetOptions.filter((option) => !usedIds.has(option.value));

    const groupOptions = (options: typeof widgetOptions) =>
        WIDGET_CATEGORY_ORDER.map((category) => ({
            category,
            options: options.filter((option) => option.category === category),
        })).filter((group) => group.options.length > 0);

    const groupedWidgetOptions = groupOptions(widgetOptions);

    // the bar is the one region whose widgets read left-to-right; the
    // sidebar and both arcs stack, so their rows are named by height
    const rowLabel = (region: WidgetRegion, index: number, count: number) => {
        if (region === 'secondary' && !capabilities.ROUND) {
            return `Bar widget ${index + 1}`;
        }
        if (count <= 1) {
            return capabilities.ROUND ? 'Arc widget' : 'Sidebar widget';
        }
        if (index === 0) {
            return 'Top widget';
        }
        return index === count - 1 ? 'Bottom widget' : `Middle widget ${index}`;
    };

    /*
     * Fit is reported only when it's a problem. A meter that spends most of
     * its life saying "everything fits" is noise, and the measure is
     * qualitative anyway: it's good to a few pixels, but the watch degrades
     * gracefully (shorter text candidates, a smaller fallback font), so
     * there's no hard cliff to put a precise number against. "over" means
     * the widgets' full forms don't fit — not that anything breaks.
     */
    const CROWD_MESSAGE = 'Too many widgets to fit — they will crowd together on the watch.';
    const overMessage = (region: WidgetRegion) =>
        region === 'secondary' && !capabilities.ROUND
            ? 'Too many widgets to fit — the watch will shorten their text.'
            : CROWD_MESSAGE;

    const renderWarning = (region: WidgetRegion, fit: FitResult) =>
        fit.state === 'over' ? (
            <p className="widget-warning" role="status">
                <span className="widget-warning-icon" aria-hidden="true">!</span>
                {overMessage(region)}
            </p>
        ) : null;

    const renderWidgetRow = (region: WidgetRegion, widgetId: number, index: number, count: number) => {
        const selectId = `widget-slot-${region}-${index}`;
        const label = rowLabel(region, index, count);
        const widgetName =
            widgetOptions.find((option) => option.value === widgetId)?.label ?? 'widget';

        const isDragging = drag?.widgetId === widgetId;
        const dropSide = drag?.drop?.region === region && drag.drop.targetWidgetId === widgetId
            ? drag.drop.position
            : undefined;

        return (
            <div
                className="halite-item halite-select widget-slot-row"
                key={selectId}
                data-widget-id={widgetId}
                data-dragging={isDragging || undefined}
                data-drop={dropSide}
                style={isDragging ? { transform: `translateY(${drag.dy}px)` } : undefined}
            >
                <button
                    type="button"
                    className="widget-slot-drag"
                    aria-label={`Reorder ${widgetName}`}
                    title={`Drag to reorder ${widgetName}`}
                    onPointerDown={(e) => onHandlePointerDown(e, region, widgetId)}
                    onPointerMove={onHandlePointerMove}
                    onPointerUp={onHandlePointerUp}
                    onPointerCancel={onHandlePointerCancel}
                    onKeyDown={(e) => onHandleKeyDown(e, region, widgetId)}
                >
                    <span aria-hidden="true">⠿</span>
                </button>
                <label className="halite-item-label" htmlFor={selectId}>{label}</label>
                <select
                    id={selectId}
                    value={widgetId}
                    onChange={(e) => changeWidget(region, index, parseInt(e.target.value, 10))}
                    className="halite-select-input"
                >
                    {groupedWidgetOptions.map((group) => (
                        <optgroup key={group.category} label={group.category}>
                            {group.options.map((opt) => (
                                <option key={opt.value} value={opt.value}>
                                    {opt.label}
                                </option>
                            ))}
                        </optgroup>
                    ))}
                </select>
                <button
                    type="button"
                    className="widget-slot-remove"
                    aria-label={`Remove ${widgetName}`}
                    title={`Remove ${widgetName}`}
                    onClick={() => removeWidget(region, index)}
                >
                    ×
                </button>
            </div>
        );
    };

    const renderAddWidget = (region: WidgetRegion, count: number, cap: number) => {
        if (count >= cap || addableOptions.length === 0) {
            return null;
        }

        const selectId = `widget-add-${region}`;

        return (
            <div className="halite-item halite-select widget-add-row" key={selectId}>
                <label className="halite-item-label" htmlFor={selectId}>Add widget</label>
                <select
                    id={selectId}
                    value=""
                    onChange={(e) => addWidget(region, parseInt(e.target.value, 10))}
                    className="halite-select-input widget-add-select"
                >
                    <option value="" disabled>+ Add widget…</option>
                    {groupOptions(addableOptions).map((group) => (
                        <optgroup key={group.category} label={group.category}>
                            {group.options.map((opt) => (
                                <option key={opt.value} value={opt.value}>
                                    {opt.label}
                                </option>
                            ))}
                        </optgroup>
                    ))}
                </select>
            </div>
        );
    };

    /** One region's heading, rows, add row and warning; drags hit-test these. */
    const renderRegion = (
        region: WidgetRegion,
        heading: string,
        hint: string,
        ids: number[],
        cap: number,
        warning: React.ReactNode
    ) => (
        <div
            className="widget-region"
            data-region={region}
            data-drop-empty={
                ids.length === 0 && drag?.drop?.region === region ? true : undefined
            }
        >
            <h4 className="widget-region-heading">{heading}</h4>
            {ids.map((id, i) => renderWidgetRow(region, id, i, ids.length))}
            {ids.length === 0 && <p className="widget-region-hint">{hint}</p>}
            {renderAddWidget(region, ids.length, cap)}
            {warning}
        </div>
    );

    return (
        <div className="widget-selector-container">
            <div className="widget-selector-preview">
                <WatchPreview />
            </div>
            <div
                className={`widget-selector-controls${drag ? ' widget-selector-controls--dragging' : ''}`}
                ref={controlsRef}
            >
                {renderRegion(
                    'primary',
                    capabilities.ROUND ? 'Left arc' : 'Sidebar',
                    capabilities.ROUND
                        ? 'No widgets in this arc.'
                        : 'No widgets — the sidebar shows as a plain band.',
                    primaryIds,
                    primarySlotCount,
                    renderWarning(
                        'primary',
                        capabilities.ROUND
                            ? assessArc(sidebarUnitsUsed(primaryIds), capabilities, settings.largeSidebarFont)
                            : assessSidebar(sidebarUnitsUsed(primaryIds), capabilities, settings.largeSidebarFont)
                    )
                )}
                {renderRegion(
                    'secondary',
                    capabilities.ROUND ? 'Right arc' : 'Bottom bar',
                    capabilities.ROUND
                        ? 'No widgets in this arc.'
                        : 'Add a widget here to turn the bottom bar on.',
                    secondaryIds,
                    secondarySlotCount,
                    renderWarning(
                        'secondary',
                        capabilities.ROUND
                            ? assessArc(sidebarUnitsUsed(secondaryIds), capabilities, settings.largeSidebarFont)
                            : assessInlineBar(secondaryIds, settings, capabilities)
                    )
                )}
            </div>
        </div>
    );
};

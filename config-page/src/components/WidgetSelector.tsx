import React from 'react';
import { WatchPreview } from './WatchPreview';
import { useCapabilities, useConfig } from '../context/PebbleConfigContext';
import { getWidgetOptions } from '../data/widgetTypes';
import { layoutHasSidebar, layoutHasBottomBar, layoutHasStackedBar } from '../utils/layout';
import { sidebarUnitsUsed } from '../utils/budget';
import { assessInlineBar, assessSidebar, FitResult } from '../utils/widgetMetrics';

/*
 * Each region is presented as a packed widget list: N dropdowns for the
 * N configured widgets, plus an "add widget" dropdown and per-row remove
 * buttons. On the watch the sidebar pins its first widget to the top and
 * its last to the bottom, while the bottom bar centers its widgets as one
 * group, so an "empty slot" has no visual meaning either way — EMPTY (0)
 * is only wire padding, written back when the lists are packed into the
 * slot keys.
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
    'bottomBarWidget3Id',
] as const;

type WidgetRegion = 'sidebar' | 'bar';

const WIDGET_CATEGORY_ORDER = ['General', 'Time & Date', 'Weather', 'Health'] as const;

export const WidgetSelector: React.FC = () => {
    const capabilities = useCapabilities();
    const { settings, updateSetting } = useConfig();
    const widgetOptions = getWidgetOptions(capabilities);

    // widget counts per region mirror the watch's rendered slot caps
    // (sidebar_layout.h): sidebars hold a stack of small widgets, the
    // inline bar holds two (three on emery), the tall stacked bar four
    const sidebarSlotCount = capabilities.ROUND ? 2 : capabilities.EMERY ? 6 : 4;
    const barSlotCount = layoutHasStackedBar(settings.layout)
        ? 4
        : capabilities.EMERY ? 3 : 2;

    const showSidebar = capabilities.ROUND || layoutHasSidebar(settings.layout);
    const showBar = !capabilities.ROUND && layoutHasBottomBar(settings.layout);

    const readIds = (keys: readonly (typeof ALL_SIDEBAR_SLOT_KEYS[number] | typeof ALL_BAR_SLOT_KEYS[number])[], count: number) =>
        keys.slice(0, count).map((key) => settings[key]).filter((id) => id !== 0);

    const sidebarIds = readIds(ALL_SIDEBAR_SLOT_KEYS, sidebarSlotCount);
    // when the layout hides the bar, keep every stored bar slot so a
    // commit doesn't discard widgets configured for other layouts
    const barIds = readIds(ALL_BAR_SLOT_KEYS, showBar ? barSlotCount : ALL_BAR_SLOT_KEYS.length);

    // write both regions back packed, zero-padding every wire slot so
    // stale ids can't hide in slots the current layout doesn't show
    const commit = (nextSidebar: number[], nextBar: number[]) => {
        ALL_SIDEBAR_SLOT_KEYS.forEach((key, i) =>
            updateSetting(key, nextSidebar[i] !== undefined ? nextSidebar[i] : 0)
        );
        ALL_BAR_SLOT_KEYS.forEach((key, i) =>
            updateSetting(key, nextBar[i] !== undefined ? nextBar[i] : 0)
        );
    };

    const changeWidget = (region: WidgetRegion, index: number, widgetId: number) => {
        const sidebar = [...sidebarIds];
        const bar = [...barIds];
        (region === 'sidebar' ? sidebar : bar)[index] = widgetId;

        // a widget lives in one place: drop any other occurrence
        commit(
            sidebar.filter((id, i) => id !== widgetId || (region === 'sidebar' && i === index)),
            bar.filter((id, i) => id !== widgetId || (region === 'bar' && i === index))
        );
    };

    const addWidget = (region: WidgetRegion, widgetId: number) => {
        const sidebar = sidebarIds.filter((id) => id !== widgetId);
        const bar = barIds.filter((id) => id !== widgetId);
        (region === 'sidebar' ? sidebar : bar).push(widgetId);
        commit(sidebar, bar);
    };

    const removeWidget = (region: WidgetRegion, index: number) => {
        commit(
            region === 'sidebar' ? sidebarIds.filter((_, i) => i !== index) : sidebarIds,
            region === 'bar' ? barIds.filter((_, i) => i !== index) : barIds
        );
    };

    const usedIds = new Set([...sidebarIds, ...barIds]);
    const addableOptions = widgetOptions.filter((option) => !usedIds.has(option.value));

    const groupOptions = (options: typeof widgetOptions) =>
        WIDGET_CATEGORY_ORDER.map((category) => ({
            category,
            options: options.filter((option) => option.category === category),
        })).filter((group) => group.options.length > 0);

    const groupedWidgetOptions = groupOptions(widgetOptions);

    const rowLabel = (region: WidgetRegion, index: number, count: number) => {
        if (region === 'bar') {
            return `Bar widget ${index + 1}`;
        }
        if (capabilities.ROUND) {
            return index === 0 ? 'Left arc widget' : 'Right arc widget';
        }
        if (count <= 1) {
            return 'Sidebar widget';
        }
        if (index === 0) {
            return 'Top widget';
        }
        return index === count - 1 ? 'Bottom widget' : `Middle widget ${index}`;
    };

    /*
     * The fit readout is deliberately qualitative. The underlying measure
     * is good to a few pixels, but the watch also degrades gracefully
     * (shorter text candidates, a smaller fallback font), so there is no
     * hard cliff to report a precise number against — "over" means the
     * widgets' full forms don't fit, not that anything breaks.
     */
    const FIT_LABEL: Record<FitResult['state'], string> = {
        good: 'Everything fits',
        tight: 'Tight fit',
        over: 'Widgets will be squeezed',
    };

    const renderBudget = (fit: FitResult) => (
        <div className={`widget-budget widget-budget--${fit.state}`} role="status">
            <div className="widget-budget-track" aria-hidden="true">
                <div
                    className="widget-budget-fill"
                    style={{ width: `${Math.min(100, fit.ratio * 100)}%` }}
                />
            </div>
            <span>{FIT_LABEL[fit.state]}</span>
        </div>
    );

    const renderWidgetRow = (region: WidgetRegion, widgetId: number, index: number, count: number) => {
        const selectId = `widget-slot-${region}-${index}`;
        const label = rowLabel(region, index, count);
        const widgetName =
            widgetOptions.find((option) => option.value === widgetId)?.label ?? 'widget';

        return (
            <div className="halite-item halite-select widget-slot-row" key={selectId}>
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

    return (
        <div className="widget-selector-container">
            <div className="widget-selector-preview">
                <WatchPreview />
            </div>
            <div className="widget-selector-controls">
                {showSidebar && (
                    <>
                        {sidebarIds.map((id, i) =>
                            renderWidgetRow('sidebar', id, i, sidebarIds.length)
                        )}
                        {renderAddWidget('sidebar', sidebarIds.length, sidebarSlotCount)}
                        {!capabilities.ROUND &&
                            renderBudget(
                                assessSidebar(
                                    sidebarUnitsUsed(sidebarIds),
                                    capabilities,
                                    settings.largeSidebarFont
                                )
                            )}
                    </>
                )}
                {showBar && (
                    <>
                        {showSidebar && (
                            <div className="widget-selector-divider" role="separator" />
                        )}
                        {barIds.map((id, i) =>
                            renderWidgetRow('bar', id, i, barIds.length)
                        )}
                        {renderAddWidget('bar', barIds.length, barSlotCount)}
                        {!layoutHasStackedBar(settings.layout) &&
                            renderBudget(
                                assessInlineBar(
                                    barIds,
                                    settings,
                                    capabilities,
                                    layoutHasSidebar(settings.layout)
                                )
                            )}
                    </>
                )}
            </div>
        </div>
    );
};

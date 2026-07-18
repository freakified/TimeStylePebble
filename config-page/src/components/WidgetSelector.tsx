import React from 'react';
import { WatchPreview } from './WatchPreview';
import { useCapabilities, useConfig } from '../context/PebbleConfigContext';
import { getWidgetOptions } from '../data/widgetTypes';
import { layoutHasSidebar, layoutHasBottomBar, layoutHasStackedBar } from '../utils/layout';
import {
    sidebarCapacity,
    sidebarUnitsUsed,
    inlineBarCapacity,
    inlineBarUnitsUsed,
} from '../utils/budget';

const WIDGET_SETTING_KEYS = [
    'widget0Id',
    'widget1Id',
    'widget2Id',
    'widget3Id',
    'widget4Id',
    'widget5Id',
    'bottomBarWidget0Id',
    'bottomBarWidget1Id',
    'bottomBarWidget2Id',
    'bottomBarWidget3Id',
] as const;

type WidgetSettingKey = typeof WIDGET_SETTING_KEYS[number];

// slots per region mirror the watch's rendered caps (sidebar_layout.h):
// sidebars hold a stack of small widgets, the inline bar holds two
// (three on emery), the tall stacked bar four
const SIDEBAR_SLOT_KEYS_EMERY: WidgetSettingKey[] = [
    'widget0Id', 'widget1Id', 'widget2Id', 'widget3Id', 'widget4Id', 'widget5Id',
];
const SIDEBAR_SLOT_KEYS_RECT: WidgetSettingKey[] = [
    'widget0Id', 'widget1Id', 'widget2Id', 'widget3Id',
];
const BAR_SLOT_KEYS: WidgetSettingKey[] = [
    'bottomBarWidget0Id', 'bottomBarWidget1Id', 'bottomBarWidget2Id', 'bottomBarWidget3Id',
];
const WIDGET_CATEGORY_ORDER = ['General', 'Time & Date', 'Weather', 'Health'] as const;

export const WidgetSelector: React.FC = () => {
    const capabilities = useCapabilities();
    const { settings, updateSetting } = useConfig();
    const widgetOptions = getWidgetOptions(capabilities);
    const groupedWidgetOptions = WIDGET_CATEGORY_ORDER.map((category) => ({
        category,
        options: widgetOptions.filter((option) => option.category === category),
    })).filter((group) => group.options.length > 0);

    const selectClass = 'halite-item halite-select';

    const handleWidgetChange = (
        settingKey: typeof WIDGET_SETTING_KEYS[number],
        widgetId: number
    ) => {
        if (widgetId !== 0) {
            WIDGET_SETTING_KEYS.forEach((key) => {
                if (key !== settingKey && settings[key] === widgetId) {
                    updateSetting(key, 0);
                }
            });
        }

        updateSetting(settingKey, widgetId);
    };

    const sidebarSlotKeys = capabilities.EMERY
        ? SIDEBAR_SLOT_KEYS_EMERY
        : SIDEBAR_SLOT_KEYS_RECT;
    const sidebarSlotLabel = (i: number) =>
        i === 0 ? 'Top slot'
        : i === sidebarSlotKeys.length - 1 ? 'Bottom slot'
        : `Middle slot ${i}`;

    const barSlotCount = layoutHasStackedBar(settings.layout)
        ? 4
        : capabilities.EMERY ? 3 : 2;
    const barSlotKeys = BAR_SLOT_KEYS.slice(0, barSlotCount);

    const renderBudget = (used: number, capacity: number) => {
        const over = used > capacity;
        const fillPercent = Math.min(100, (used / capacity) * 100);

        return (
            <div
                className={`widget-budget${over ? ' widget-budget--over' : ''}`}
                role="status"
            >
                <div className="widget-budget-track" aria-hidden="true">
                    <div
                        className="widget-budget-fill"
                        style={{ width: `${fillPercent}%` }}
                    />
                </div>
                <span>
                    {used} of {capacity} space used
                    {over && ' — widgets may get squeezed'}
                </span>
            </div>
        );
    };

    const renderSlot = (label: string, settingKey: typeof WIDGET_SETTING_KEYS[number]) => {
        const selectId = `widget-selector-${settingKey}`;

        return (
            <div className={selectClass}>
                <label className="halite-item-label" htmlFor={selectId}>{label}</label>
                <select
                    id={selectId}
                    value={settings[settingKey]}
                    onChange={(e) => handleWidgetChange(settingKey, parseInt(e.target.value, 10))}
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
            </div>
        );
    };

    return (
        <div className="widget-selector-container">
            <div className="widget-selector-preview">
                <WatchPreview />
            </div>
            <div className="widget-selector-controls">
                {capabilities.ROUND ? (
                    <>
                        {renderSlot('Left slot', 'widget0Id')}
                        {renderSlot('Right slot', 'widget1Id')}
                    </>
                ) : (
                    <>
                        {layoutHasSidebar(settings.layout) && (
                            <>
                                {sidebarSlotKeys.map((key, i) =>
                                    renderSlot(sidebarSlotLabel(i), key)
                                )}
                                {renderBudget(
                                    sidebarUnitsUsed(
                                        sidebarSlotKeys.map((key) => settings[key])
                                    ),
                                    sidebarCapacity(capabilities, settings.largeSidebarFont)
                                )}
                            </>
                        )}
                        {layoutHasBottomBar(settings.layout) && (
                            <>
                                {layoutHasSidebar(settings.layout) && (
                                    <div className="widget-selector-divider" role="separator" />
                                )}
                                {barSlotKeys.map((key, i) =>
                                    renderSlot(`Bar slot ${i + 1}`, key)
                                )}
                                {!layoutHasStackedBar(settings.layout) &&
                                    renderBudget(
                                        inlineBarUnitsUsed(
                                            barSlotKeys.map((key) => settings[key])
                                        ),
                                        inlineBarCapacity(capabilities, settings.largeSidebarFont)
                                    )}
                            </>
                        )}
                    </>
                )}
            </div>
        </div>
    );
};

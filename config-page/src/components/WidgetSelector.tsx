import React from 'react';
import { WatchPreview } from './WatchPreview';
import { useCapabilities, useConfig } from '../context/PebbleConfigContext';
import { getWidgetOptions } from '../data/widgetTypes';

const WIDGET_SETTING_KEYS = [
    'widget0Id',
    'widget1Id',
    'widget2Id',
    'bottomBarWidget0Id',
    'bottomBarWidget1Id',
] as const;
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
                        {renderSlot('Right slot', 'widget2Id')}
                    </>
                ) : (
                    <>
                        {renderSlot('Top slot', 'widget0Id')}
                        {renderSlot('Middle slot', 'widget1Id')}
                        {renderSlot('Bottom slot', 'widget2Id')}
                        {settings.bottomBar && (
                            <>
                                {renderSlot('Bottom left', 'bottomBarWidget0Id')}
                                {renderSlot('Bottom right', 'bottomBarWidget1Id')}
                            </>
                        )}
                    </>
                )}
            </div>
        </div>
    );
};

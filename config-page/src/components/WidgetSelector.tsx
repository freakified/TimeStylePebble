import React from 'react';
import { WatchPreview } from './WatchPreview';
import { useCapabilities, useConfig } from '../context/PebbleConfigContext';
import { getWidgetOptions } from '../data/widgetTypes';

export const WidgetSelector: React.FC = () => {
    const capabilities = useCapabilities();
    const { settings, updateSetting } = useConfig();
    const widgetOptions = getWidgetOptions(capabilities);

    const selectClass = 'halite-item halite-select';

    const renderSlot = (label: string, settingKey: 'widget0Id' | 'widget1Id' | 'widget2Id') => (
        <div className={selectClass}>
            <label className="halite-item-label">{label}</label>
            <select
                value={settings[settingKey]}
                onChange={(e) => updateSetting(settingKey, parseInt(e.target.value, 10))}
                className="halite-select-input"
            >
                {widgetOptions.map((opt) => (
                    <option key={opt.value} value={opt.value}>
                        {opt.label}
                    </option>
                ))}
            </select>
        </div>
    );

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
                    </>
                )}
            </div>
        </div>
    );
};

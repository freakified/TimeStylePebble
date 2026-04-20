import React from 'react';
import { WatchPreview } from './WatchPreview';
import { useCapabilities, useConfig } from '../context/PebbleConfigContext';
import { getWidgetOptions } from '../data/widgetTypes';

export const WidgetSelector: React.FC = () => {
    const capabilities = useCapabilities();
    const { settings, updateSetting } = useConfig();
    const widgetOptions = getWidgetOptions(capabilities);

    const selectClass = 'halite-item halite-select';

    const renderSlot = (label: string, settingKey: 'widget_0_id' | 'widget_1_id' | 'widget_2_id') => (
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
                        {renderSlot('Left slot', 'widget_0_id')}
                        {renderSlot('Right slot', 'widget_2_id')}
                    </>
                ) : (
                    <>
                        {renderSlot('Top slot', 'widget_0_id')}
                        {renderSlot('Middle slot', 'widget_1_id')}
                        {renderSlot('Bottom slot', 'widget_2_id')}
                    </>
                )}
            </div>
        </div>
    );
};

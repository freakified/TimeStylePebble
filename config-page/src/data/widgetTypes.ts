// Widget type definitions for TimeStyle sidebar slots.
// Values are integer IDs matching the WidgetID enum on the watch.

import { Capabilities } from '../context/types';

export interface WidgetOption {
    value: number;
    label: string;
    requiresHealth?: boolean;
    requiresHRM?: boolean;
}

export const ALL_WIDGET_OPTIONS: WidgetOption[] = [
    { value: 0,  label: 'Empty' },
    { value: 2,  label: 'Battery' },
    { value: 3,  label: 'Alternate Time Zone' },
    { value: 4,  label: "Today's Date" },
    { value: 5,  label: 'Seconds' },
    { value: 6,  label: 'Week Number' },
    { value: 7,  label: 'Current Weather' },
    { value: 8,  label: "Today's Forecast" },
    { value: 9,  label: 'Sleep',                requiresHealth: true },
    { value: 10, label: 'Steps',                requiresHealth: true },
    { value: 11, label: 'Swatch™ Internet Time' },
    { value: 12, label: 'Heart Rate',           requiresHRM: true },
    { value: 13, label: 'UV Index' },
];

export function getWidgetOptions(capabilities: Capabilities): WidgetOption[] {
    return ALL_WIDGET_OPTIONS.filter((w) => {
        if (w.requiresHealth && !capabilities.HEALTH) return false;
        // HRM is only available on diorite and emery — no dedicated capability flag
        if (w.requiresHRM && !capabilities.DIORITE && !capabilities.EMERY) return false;
        return true;
    });
}

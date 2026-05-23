// Widget type definitions for TimeStyle sidebar slots.
// Values are integer IDs matching the WidgetID enum on the watch.

import { Capabilities } from '../context/types';

export interface WidgetOption {
    value: number;
    label: string;
    category: 'General' | 'Time & Date' | 'Weather' | 'Health';
    requiresHealth?: boolean;
    requiresHRM?: boolean;
}

export const ALL_WIDGET_OPTIONS: WidgetOption[] = [
    { value: 0,  label: 'Empty', category: 'General' },
    { value: 2,  label: 'Battery', category: 'General' },
    { value: 3,  label: 'Alternate Time Zone', category: 'Time & Date' },
    { value: 4,  label: "Today's Date", category: 'Time & Date' },
    { value: 5,  label: 'Seconds', category: 'Time & Date' },
    { value: 6,  label: 'Week Number', category: 'Time & Date' },
    { value: 7,  label: 'Current Weather', category: 'Weather' },
    { value: 8,  label: "Today's Forecast", category: 'Weather' },
    { value: 9,  label: 'Sleep', category: 'Health', requiresHealth: true },
    { value: 10, label: 'Steps', category: 'Health', requiresHealth: true },
    { value: 11, label: 'Swatch™ Internet Time', category: 'Time & Date' },
    { value: 12, label: 'Heart Rate', category: 'Health', requiresHRM: true },
    { value: 13, label: 'UV Index', category: 'Weather' },
];

export function getWidgetOptions(capabilities: Capabilities): WidgetOption[] {
    return ALL_WIDGET_OPTIONS.filter((w) => {
        if (w.requiresHealth && !capabilities.HEALTH) return false;
        // HRM is only available on diorite and emery — no dedicated capability flag
        if (w.requiresHRM && !capabilities.DIORITE && !capabilities.EMERY) return false;
        return true;
    });
}

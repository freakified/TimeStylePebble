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

// Widget id 0 (EMPTY) is deliberately absent: slots form a packed list
// in the config UI (add/remove), and EMPTY is only wire padding.
export const ALL_WIDGET_OPTIONS: WidgetOption[] = [
    { value: 2,  label: 'Battery', category: 'General' },
    { value: 3,  label: 'World Time 1', category: 'Time & Date' },
    { value: 14, label: 'World Time 2', category: 'Time & Date' },
    { value: 4,  label: "Today's Date", category: 'Time & Date' },
    { value: 5,  label: 'Seconds', category: 'Time & Date' },
    { value: 6,  label: 'Week Number', category: 'Time & Date' },
    { value: 7,  label: 'Current Weather', category: 'Weather' },
    { value: 8,  label: "Today's Forecast", category: 'Weather' },
    { value: 15, label: 'Sunrise & Sunset', category: 'Weather' },
    { value: 16, label: 'Wind Speed', category: 'Weather' },
    { value: 17, label: 'Chance of Rain', category: 'Weather' },
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

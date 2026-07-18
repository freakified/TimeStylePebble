// Widget type definitions for TimeStyle sidebar slots.
// Values are integer IDs matching the WidgetID enum on the watch.

import { Capabilities } from '../context/types';

export interface WidgetOption {
    value: number;
    label: string;
    category: 'General' | 'Time & Date' | 'Weather' | 'Health';
    // Coarse size classes for the config-time unit budget: how much of a
    // region the widget's canonical form occupies. vUnits is height in the
    // vertical sidebar (stacked form), hUnits is width in the inline bottom
    // bar. Derived from the watch's measure code: icon-above-value stacks
    // are 2 tall, calendars/divided pairs 3 tall. hUnits are calibrated
    // against measured full-form widths (widgets never compact, so a
    // region's units must cover the widest typical form): one unit is a
    // quarter of the basalt bar (~26px), rounded to nearest.
    vUnits: number;
    hUnits: number;
    requiresHealth?: boolean;
    requiresHRM?: boolean;
}

// Widget id 0 (EMPTY) is deliberately absent: slots form a packed list
// in the config UI (add/remove), and EMPTY is only wire padding.
export const ALL_WIDGET_OPTIONS: WidgetOption[] = [
    { value: 2,  label: 'Battery', category: 'General', vUnits: 2, hUnits: 2 },
    { value: 3,  label: 'World Time 1', category: 'Time & Date', vUnits: 1, hUnits: 3 },
    { value: 14, label: 'World Time 2', category: 'Time & Date', vUnits: 1, hUnits: 3 },
    { value: 4,  label: "Today's Date", category: 'Time & Date', vUnits: 3, hUnits: 2 },
    { value: 5,  label: 'Seconds', category: 'Time & Date', vUnits: 1, hUnits: 1 },
    { value: 6,  label: 'Week Number', category: 'Time & Date', vUnits: 1, hUnits: 2 },
    { value: 7,  label: 'Current Weather', category: 'Weather', vUnits: 2, hUnits: 2 },
    { value: 8,  label: "Today's Forecast", category: 'Weather', vUnits: 3, hUnits: 2 },
    { value: 15, label: 'Sunrise & Sunset', category: 'Weather', vUnits: 2, hUnits: 3 },
    { value: 16, label: 'Wind Speed', category: 'Weather', vUnits: 1, hUnits: 3 },
    { value: 17, label: 'Chance of Rain', category: 'Weather', vUnits: 2, hUnits: 2 },
    { value: 9,  label: 'Sleep', category: 'Health', vUnits: 3, hUnits: 3, requiresHealth: true },
    { value: 10, label: 'Steps', category: 'Health', vUnits: 2, hUnits: 3, requiresHealth: true },
    { value: 11, label: 'Swatch™ Internet Time', category: 'Time & Date', vUnits: 1, hUnits: 2 },
    { value: 12, label: 'Heart Rate', category: 'Health', vUnits: 2, hUnits: 2, requiresHRM: true },
    { value: 13, label: 'UV Index', category: 'Weather', vUnits: 1, hUnits: 1 },
];

export function getWidgetOptions(capabilities: Capabilities): WidgetOption[] {
    return ALL_WIDGET_OPTIONS.filter((w) => {
        if (w.requiresHealth && !capabilities.HEALTH) return false;
        // HRM is only available on diorite and emery — no dedicated capability flag
        if (w.requiresHRM && !capabilities.DIORITE && !capabilities.EMERY) return false;
        return true;
    });
}

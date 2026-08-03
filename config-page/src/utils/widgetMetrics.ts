import { Capabilities, Settings } from '../context/types';
import { findWorldTimeCity } from '../data/cities';
import iconBattery from '../assets/preview/widget_previews/icon_battery.svg?raw';
import iconDate from '../assets/preview/widget_previews/icon_date.svg?raw';
import iconHrm from '../assets/preview/widget_previews/icon_hrm.svg?raw';
import iconSleep from '../assets/preview/widget_previews/icon_sleep.svg?raw';
import iconSteps from '../assets/preview/widget_previews/icon_steps.svg?raw';
import iconWeather from '../assets/preview/widget_previews/icon_weather.svg?raw';

/*
 * Shared widget content + measurement.
 *
 * There is one description of what each widget shows (getWidgetItems /
 * getInlineParts) and two rulers over it:
 *
 *   - the PREVIEW ruler (measureItem, measureInline) works in the
 *     preview's own stylized units — smaller icons and text than the
 *     watch actually draws, tuned to look right in an SVG mock;
 *   - the WATCH ruler (inlineWidgetWidth, below) works in real watch
 *     pixels and mirrors horizontal_content_width() in sidebar_widgets.c.
 *
 * The budget must use the watch ruler; the preview keeps its own. Sharing
 * the content layer is the point — a blank world-time label or the
 * wide-sidebar step format changes both at once.
 */

/* ── Icons ── */

export interface IconAsset {
    svg: string;
    w: number;
    h: number;
}

export const ICONS = {
    battery: { svg: iconBattery, w: 25, h: 18 },
    date:    { svg: iconDate,    w: 25, h: 25 },
    hrm:     { svg: iconHrm,     w: 25, h: 25 },
    sleep:   { svg: iconSleep,   w: 25, h: 22 },
    steps:   { svg: iconSteps,   w: 25, h: 18 },
    weather: { svg: iconWeather, w: 25, h: 25 },
} as const satisfies Record<string, IconAsset>;

/* ── Widget content ── */

export type WidgetItem =
    | { type: 'icon'; asset: IconAsset }
    | { type: 'calendar'; day: string }   // date icon with the day number inside
    | { type: 'text'; text: string; size: number }
    | { type: 'divider' };

export const STACK_GAP = 2;
export const TEXT_CHAR_W = 0.62;  // average bold advance as a fraction of font size

const DAY_NAMES = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
const MONTH_NAMES = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];

const getIsoWeek = (date: Date): number => {
    const utc = new Date(Date.UTC(date.getFullYear(), date.getMonth(), date.getDate()));
    const dayNum = utc.getUTCDay() || 7;
    utc.setUTCDate(utc.getUTCDate() + 4 - dayNum);
    const yearStart = Date.UTC(utc.getUTCFullYear(), 0, 1);
    return Math.ceil(((utc.getTime() - yearStart) / 86400000 + 1) / 7);
};

const text = (str: string, size = 9): WidgetItem => ({ type: 'text', text: str, size });

export const getWidgetItems = (widgetId: number, settings: Settings, compact = false): WidgetItem[] => {
    const now = new Date();

    switch (widgetId) {
        case 2:  // Battery
            return [{ type: 'icon', asset: ICONS.battery }, text('80%')];
        case 3: {  // World Time 1
            // an explicit blank label means no label; the fallback chain
            // only applies while no city has been picked yet
            const label1 = findWorldTimeCity(settings.worldTime1City)
                ? settings.worldTime1Label
                : settings.altClockName || 'UTC';
            return label1 ? [text(label1), text('5:42p', 8)] : [text('5:42p', 8)];
        }
        case 14: {  // World Time 2
            const label2 = findWorldTimeCity(settings.worldTime2City)
                ? settings.worldTime2Label
                : settings.altClock2Name || 'TYO';
            return label2 ? [text(label2), text('9:42a', 8)] : [text('9:42a', 8)];
        }
        case 4: {  // Today's Date
            const items: WidgetItem[] = [
                text(DAY_NAMES[now.getDay()]),
                { type: 'calendar', day: String(now.getDate()) },
            ];
            if (!compact) items.push(text(MONTH_NAMES[now.getMonth()]));
            return items;
        }
        case 5:  // Seconds
            return [text('30')];
        case 6:  // Week Number
            return [text('Wk'), text(String(getIsoWeek(now)))];
        case 7:  // Current Weather
            return [{ type: 'icon', asset: ICONS.weather }, text('72°')];
        case 8:  // Today's Forecast
            return [{ type: 'icon', asset: ICONS.weather }, text('75°'), { type: 'divider' }, text('60°')];
        case 9:  // Sleep
            return [{ type: 'icon', asset: ICONS.sleep }, text('7h'), text('42m', 7)];
        case 10:  // Steps
            return [{ type: 'icon', asset: ICONS.steps }, text(settings.wideSidebar ? '2700' : '2.7k')];
        case 11:  // Swatch Internet Time
            return [text('@'), text('543')];
        case 12:  // Heart Rate
            return [{ type: 'icon', asset: ICONS.hrm }, text('72')];
        case 13:  // UV Index
            return [text('UV'), text('6')];
        case 15:  // Sunrise & Sunset
            return [{ type: 'icon', asset: ICONS.weather }, text('8:14p', 8)];
        case 16:  // Wind Speed
            return [text('WIND', 7), text('12')];
        case 17:  // Chance of Rain
            return [text('RAIN', 7), text('40%')];
        default:
            return [];
    }
};

/* ── Preview ruler (stylized SVG units) ── */

export const measureItem = (item: WidgetItem): { w: number; h: number } => {
    switch (item.type) {
        case 'icon': return { w: item.asset.w, h: item.asset.h };
        case 'calendar': return { w: ICONS.date.w, h: ICONS.date.h };
        case 'text': return { w: item.text.length * item.size * TEXT_CHAR_W, h: item.size + 1 };
        case 'divider': return { w: 14, h: 3 };
    }
};

export interface WidgetSpec {
    items: WidgetItem[];
    w: number;
    h: number;
}

export const getWidgetSpec = (widgetId: number, settings: Settings, compact = false): WidgetSpec => {
    const items = getWidgetItems(widgetId, settings, compact);
    let w = 0;
    let h = 0;
    items.forEach((item, i) => {
        const size = measureItem(item);
        w = Math.max(w, size.w);
        h += size.h + (i > 0 ? STACK_GAP : 0);
    });
    return { items, w, h };
};

export const INLINE_ICON_H = 12;
export const INLINE_GAP = 3;
export const INLINE_FONT = 9;

export interface InlineParts {
    icon?: { asset: IconAsset } | { calendar: string };
    text: string;
}

/* Collapse a widget's stacked item list into the inline (icon + one
 * line of text) form the narrow bottom bar draws. */
export const getInlineParts = (widgetId: number, settings: Settings): InlineParts | null => {
    const items = getWidgetItems(widgetId, settings, true);
    if (items.length === 0) return null;

    let icon: InlineParts['icon'];
    const textParts: string[] = [];
    let pendingDivider = false;

    for (const item of items) {
        if (item.type === 'icon' && !icon) icon = { asset: item.asset };
        else if (item.type === 'calendar' && !icon) icon = { calendar: item.day };
        else if (item.type === 'divider') pendingDivider = true;
        else if (item.type === 'text') {
            if (textParts.length > 0) textParts.push(pendingDivider ? '/' : ' ');
            pendingDivider = false;
            textParts.push(item.text);
        }
    }

    return { icon, text: textParts.join('') };
};

export const measureInline = (parts: InlineParts): number => {
    let w = 0;
    if (parts.icon) {
        const asset = 'asset' in parts.icon ? parts.icon.asset : ICONS.date;
        w += asset.w * (INLINE_ICON_H / asset.h);
        if (parts.text) w += INLINE_GAP;
    }
    w += parts.text.length * INLINE_FONT * TEXT_CHAR_W;
    return w;
};

/* ── Watch ruler (real watch pixels) ──────────────────────────────────
 *
 * Mirrors sidebar_layout.h and horizontal_content_width() in
 * sidebar_widgets.c. Widget widths on the watch are
 *   icon(25) + gap(3) + textWidth + allowance(1)
 * with the icon terms dropped for text-only widgets.
 */

const WATCH_ICON_SIZE = 25;          // SIDEBAR_ICON_SIZE
const WATCH_ITEM_GAP = 3;            // horizontal_item_gap()
const WATCH_TEXT_ALLOWANCE = 1;      // SIDEBAR_TEXT_FIT_ALLOWANCE
const WATCH_BAR_PADDING_X = 4;       // HBAR_SLOT_PADDING_X
const WATCH_BAR_VBAR_GAP = 2;        // HBAR_VBAR_GAP

/*
 * Average advance width, in watch pixels per character, for the Gothic
 * Bold system fonts the sidebar uses. Measured off the watch itself:
 * temporary APP_LOG instrumentation in render_horizontal_flow dumped
 * SidebarWidgets_measure() widths for every bottom-bar widget across
 * basalt and emery at both font sizes, and these are the per-character
 * averages that best fit that data (median absolute error ~1px per
 * widget). Real advances vary by glyph -- '%' is wide, ':' is narrow --
 * so this is deliberately an average, not a per-glyph table.
 *
 * The sizes are what matter, not the platform: basalt at large fonts and
 * emery at normal fonts both use Gothic 24 and measured byte-identical.
 */
const GOTHIC_ADVANCE: Record<number, number> = {
    18: 7.5,
    24: 8.8,
    28: 10.0,
};

interface PlatformMetrics {
    screenW: number;
    vbarNarrow: number;
    vbarWide: number;
    barWidgetGap: number;
    fontSize: number;
    /** rendered inline bar slots (HREGION_SLOT_CAP_INLINE) */
    barSlots: number;
}

/** Per-platform geometry, mirroring the #ifdefs in sidebar_layout.h. */
export const platformMetrics = (
    capabilities: Capabilities,
    largeFonts: boolean
): PlatformMetrics => {
    if (capabilities.EMERY) {
        return {
            screenW: 200,
            vbarNarrow: largeFonts ? 43 : 34,
            vbarWide: largeFonts ? 51 : 46,
            barWidgetGap: 6,
            fontSize: largeFonts ? 28 : 24,
            barSlots: 3,
        };
    }
    // basalt / diorite / flint
    return {
        screenW: 144,
        vbarNarrow: 30,
        vbarWide: largeFonts ? 46 : 38,
        barWidgetGap: 4,
        fontSize: largeFonts ? 24 : 18,
        barSlots: 2,
    };
};

const textWidth = (str: string, fontSize: number): number =>
    str.length * (GOTHIC_ADVANCE[fontSize] ?? GOTHIC_ADVANCE[24]);

/** A widget's full-form width in the inline bottom bar, in watch pixels. */
export const inlineWidgetWidth = (
    widgetId: number,
    settings: Settings,
    fontSize: number
): number => {
    const parts = getInlineParts(widgetId, settings);
    if (!parts) return 0;

    let w = 0;
    if (parts.icon) {
        w += WATCH_ICON_SIZE;
        if (parts.text) w += WATCH_ITEM_GAP;
    }
    if (parts.text) w += textWidth(parts.text, fontSize) + WATCH_TEXT_ALLOWANCE;
    return w;
};

/**
 * Usable width of the inline bottom bar, in watch pixels. The bar yields
 * its corner to the vertical sidebar, so a wider sidebar
 * directly costs the bar room -- the effect the old unit budget missed.
 */
export const inlineBarContentWidth = (
    settings: Settings,
    capabilities: Capabilities,
    hasSidebar: boolean
): number => {
    const m = platformMetrics(capabilities, settings.largeSidebarFont);
    const sidebarW = hasSidebar
        ? (settings.wideSidebar ? m.vbarWide : m.vbarNarrow) + WATCH_BAR_VBAR_GAP
        : 0;
    return m.screenW - sidebarW - WATCH_BAR_PADDING_X * 2;
};

export type FitState = 'good' | 'tight' | 'over';

export interface FitResult {
    state: FitState;
    usedPx: number;
    capacityPx: number;
    /** 0..1+ — how full the region is, for the progress track */
    ratio: number;
}

// Slack below which the widgets still render at full size and full gap,
// but fill the bar end to end. Comfortably wider than the measurement's
// own error (~3px worst case on a 104px bar), so "tight" never means
// "we're not sure".
const TIGHT_SLACK = 0.1;

/**
 * How well a set of widgets fits the inline bottom bar.
 *
 * "over" means the widgets' full forms don't fit, which is exactly when
 * the watch starts compacting text and swapping in shorter candidates --
 * it still renders, just not in the form the user picked.
 */
export const assessInlineBar = (
    widgetIds: number[],
    settings: Settings,
    capabilities: Capabilities,
    hasSidebar: boolean
): FitResult => {
    const m = platformMetrics(capabilities, settings.largeSidebarFont);
    const capacityPx = inlineBarContentWidth(settings, capabilities, hasSidebar);

    const widths = widgetIds.map((id) => inlineWidgetWidth(id, settings, m.fontSize));
    const gaps = Math.max(0, widgetIds.length - 1) * m.barWidgetGap;
    const usedPx = widths.reduce((a, b) => a + b, 0) + gaps;

    const ratio = capacityPx > 0 ? usedPx / capacityPx : 0;
    const state: FitState =
        widgetIds.length === 0 ? 'good'
        : ratio > 1 ? 'over'
        : ratio > 1 - TIGHT_SLACK ? 'tight'
        : 'good';

    return { state, usedPx, capacityPx, ratio };
};

/* ── Vertical sidebar ─────────────────────────────────────────────────
 *
 * Heights are far more predictable than widths (text height doesn't
 * depend on content), and the sidebar is always full screen height --
 * the status bar yields its corner rather than shortening it. So the
 * sidebar keeps the coarse unit model, which is well calibrated for it.
 */

const SIDEBAR_UNIT_CAPACITY = (capabilities: Capabilities, largeFonts: boolean): number =>
    (capabilities.EMERY ? 8 : 6) - (largeFonts ? 1 : 0);

export const assessSidebar = (
    unitsUsed: number,
    capabilities: Capabilities,
    largeFonts: boolean
): FitResult => {
    const capacityPx = SIDEBAR_UNIT_CAPACITY(capabilities, largeFonts);
    const ratio = capacityPx > 0 ? unitsUsed / capacityPx : 0;
    const state: FitState =
        unitsUsed === 0 ? 'good'
        : ratio > 1 ? 'over'
        : ratio > 1 - TIGHT_SLACK ? 'tight'
        : 'good';

    return { state, usedPx: unitsUsed, capacityPx, ratio };
};

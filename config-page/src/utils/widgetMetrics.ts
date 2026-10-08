import { Capabilities, Settings } from '../context/types';
import { findWorldTimeCity } from '../data/cities';
import iconBattery from '../assets/preview/widget_previews/icon_battery.svg?raw';
import iconDate from '../assets/preview/widget_previews/icon_date.svg?raw';
import iconRain from '../assets/preview/widget_previews/icon_rain.svg?raw';
import iconHrm from '../assets/preview/widget_previews/icon_hrm.svg?raw';
import iconSleep from '../assets/preview/widget_previews/icon_sleep.svg?raw';
import iconSunrise from '../assets/preview/widget_previews/icon_sunrise.svg?raw';
import iconSunset from '../assets/preview/widget_previews/icon_sunset.svg?raw';
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
    // HEAVY_RAIN.pdc, the icon PrecipChance_getContent() draws
    rain:    { svg: iconRain,    w: 25, h: 25 },
    sleep:   { svg: iconSleep,   w: 25, h: 22 },
    steps:   { svg: iconSteps,   w: 25, h: 18 },
    sunrise: { svg: iconSunrise, w: 25, h: 25 },
    sunset:  { svg: iconSunset,  w: 25, h: 25 },
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

// sample sun times behind the Sunrise & Sunset widget's 6:42a / 8:14p
const SAMPLE_SUNRISE_MINUTE = 6 * 60 + 42;
const SAMPLE_SUNSET_MINUTE = 20 * 60 + 14;

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
        case 15: {  // Sunrise & Sunset
            // like NextSunEvent_getContent(), show whichever comes up next
            const minute = now.getHours() * 60 + now.getMinutes();
            const sunriseNext = minute < SAMPLE_SUNRISE_MINUTE || minute >= SAMPLE_SUNSET_MINUTE;
            return sunriseNext
                ? [{ type: 'icon', asset: ICONS.sunrise }, text('6:42a', 8)]
                : [{ type: 'icon', asset: ICONS.sunset }, text('8:14p', 8)];
        }
        case 16: {  // Wind Speed
            // like Wind_getContent(): the full form carries the unit, which
            // the bar has room for; the narrow sidebar falls back to the
            // bare number
            const unit = settings.weatherUnits === 'c' ? 'kph' : 'mph';
            return [text('WIND', 7), text(compact ? `12${unit}` : '12')];
        }
        case 17:  // Chance of Rain
            return [{ type: 'icon', asset: ICONS.rain }, text('40%')];
        default:
            return [];
    }
};

/* ── Preview ruler (stylized SVG units) ── */

/*
 * The preview's text sizes are drawn for Gothic 18, basalt's normal
 * sidebar font. Other platform/font-size combos scale text by their own
 * font size relative to that; icons don't scale, since the watch draws
 * them at SIDEBAR_ICON_SIZE everywhere.
 */
export const previewTextScale = (fontSize: number): number => fontSize / 18;

export const measureItem = (item: WidgetItem, textScale = 1): { w: number; h: number } => {
    switch (item.type) {
        case 'icon': return { w: item.asset.w, h: item.asset.h };
        case 'calendar': return { w: ICONS.date.w, h: ICONS.date.h };
        case 'text': {
            const size = item.size * textScale;
            return { w: item.text.length * size * TEXT_CHAR_W, h: size + 1 };
        }
        case 'divider': return { w: 14, h: 3 };
    }
};

export interface WidgetSpec {
    items: WidgetItem[];
    w: number;
    h: number;
}

export const getWidgetSpec = (
    widgetId: number,
    settings: Settings,
    compact = false,
    textScale = 1
): WidgetSpec => {
    const items = getWidgetItems(widgetId, settings, compact);
    let w = 0;
    let h = 0;
    items.forEach((item, i) => {
        const size = measureItem(item, textScale);
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

export const measureInline = (parts: InlineParts, textScale = 1): number => {
    let w = 0;
    if (parts.icon) {
        const asset = 'asset' in parts.icon ? parts.icon.asset : ICONS.date;
        w += asset.w * (INLINE_ICON_H / asset.h);
        if (parts.text) w += INLINE_GAP;
    }
    w += parts.text.length * INLINE_FONT * textScale * TEXT_CHAR_W;
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
export const WATCH_BAR_PADDING_X = 4;          // HBAR_SLOT_PADDING_X
export const WATCH_BAR_PADDING_X_COMPACT = 2;  // HBAR_SLOT_PADDING_X_COMPACT
export const WATCH_BAR_VBAR_GAP = 2;           // HBAR_VBAR_GAP
export const WATCH_BAR_WIDGET_GAP_COMPACT = 1; // HBAR_WIDGET_GAP_COMPACT
export const REGION_V_PADDING_COMPACT = 4;     // REGION_V_PADDING_COMPACT

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

/**
 * The Gothic Bold size the sidebar draws with: SidebarWidgets_updateFonts()
 * in sidebar_widgets.c, where emery and gabbro step every size up.
 */
const sidebarFontSet = (capabilities: Capabilities) =>
    capabilities.EMERY || capabilities.DISPLAY_260x260_ROUND
        ? { sm: 18, md: 24, lg: 28 }
        : { sm: 14, md: 18, lg: 24 };

export const sidebarFontSize = (capabilities: Capabilities, largeFonts: boolean): number => {
    const f = sidebarFontSet(capabilities);
    return largeFonts ? f.lg : f.md;
};

interface PlatformMetrics {
    screenW: number;
    screenH: number;
    vbarNarrow: number;
    vbarWide: number;
    /** HBAR_HEIGHT */
    barHeight: number;
    /** HBAR_CLOCK_ENCROACH: how far the clock may reach into the bar */
    barClockEncroach: number;
    /** REGION_V_PADDING_DEFAULT */
    vPadding: number;
    barWidgetGap: number;
    fontSize: number;
    /** rendered sidebar slots (VREGION_SLOT_CAP) */
    sidebarSlots: number;
    /** rendered bar slots (HREGION_SLOT_CAP) */
    barSlots: number;
}

/** Per-platform geometry, mirroring the #ifdefs in sidebar_layout.h. */
export const platformMetrics = (
    capabilities: Capabilities,
    largeFonts: boolean
): PlatformMetrics => {
    const fontSize = sidebarFontSize(capabilities, largeFonts);
    if (capabilities.EMERY) {
        return {
            screenW: 200,
            screenH: 228,
            vbarNarrow: largeFonts ? 43 : 34,
            vbarWide: largeFonts ? 51 : 46,
            barHeight: largeFonts ? 43 : 34,
            barClockEncroach: 10,
            vPadding: 9,
            barWidgetGap: 6,
            fontSize,
            sidebarSlots: 6,
            barSlots: 3,
        };
    }
    // basalt / diorite / flint
    return {
        screenW: 144,
        screenH: 168,
        vbarNarrow: 30,
        vbarWide: largeFonts ? 46 : 38,
        barHeight: 30,
        barClockEncroach: 0,
        vPadding: 8,
        barWidgetGap: 4,
        fontSize,
        sidebarSlots: 4,
        barSlots: 2,
    };
};

/* ── Round arcs ───────────────────────────────────────────────────────
 *
 * Mirrors the ROUND_ARC_* block in sidebar_layout.h.
 */

interface ArcMetrics {
    /** square screen edge, in watch pixels */
    screen: number;
    /** ROUND_ARC_WIDTH */
    width: number;
    /** ROUND_ARC_CONTENT_WIDTH */
    contentWidth: number;
    /** ROUND_ARC_WIDGET_GAP */
    widgetGap: number;
}

export const arcMetrics = (capabilities: Capabilities): ArcMetrics =>
    capabilities.DISPLAY_260x260_ROUND
        ? { screen: 260, width: 56, contentWidth: 42, widgetGap: 12 }
        : { screen: 180, width: 40, contentWidth: 30, widgetGap: 8 };

export const ARC_WIDGET_GAP_COMPACT = 1;      // ROUND_ARC_WIDGET_GAP_COMPACT
/** ROUND_ARC_INWARD_NUDGE: the column sits this far towards the screen
 * center from the arc's middle */
export const ARC_INWARD_NUDGE = 1;

/**
 * How tall the usable part of an arc is, in watch pixels:
 * ROUND_ARC_CONTENT_BAND in sidebar_layout.h, which has the derivation.
 *
 * The widget column is centered across the arc's waist, and away from
 * the waist the display's rim (radius screen/2) closes in on it faster
 * than the arc's own edge (radius screen) does, so the rim is what binds.
 * The band reaches as far as the rim can come in before touching an
 * icon-wide column of ink: (width - SIDEBAR_ICON_SIZE)/2 of slack at the
 * waist, plus the column's inward nudge. About 76px on chalk and 126px
 * on gabbro.
 */
export const arcContentBand = (capabilities: Capabilities): number => {
    const m = arcMetrics(capabilities);
    const r = m.screen / 2;
    const slack = (m.width - WATCH_ICON_SIZE) / 2 + ARC_INWARD_NUDGE;
    return 2 * Math.sqrt(r * r - (r - slack) ** 2);
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
    capabilities: Capabilities
): number => {
    const m = platformMetrics(capabilities, settings.largeSidebarFont);
    const sidebarW =
        (settings.wideSidebar ? m.vbarWide : m.vbarNarrow) + WATCH_BAR_VBAR_GAP;
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
    capabilities: Capabilities
): FitResult => {
    const m = platformMetrics(capabilities, settings.largeSidebarFont);
    const capacityPx = inlineBarContentWidth(settings, capabilities);

    const widths = widgetIds.map((id) => inlineWidgetWidth(id, settings, m.fontSize));
    const gaps = Math.max(0, widgetIds.length - 1) * m.barWidgetGap;
    const usedPx = widths.reduce((a, b) => a + b, 0) + gaps;

    return assessPx(widgetIds.length, usedPx, capacityPx);
};

/* ── Stacked heights (real watch pixels) ─────────────────────────────
 *
 * The sidebar and the arcs stack widgets vertically, and a stacked
 * widget's height doesn't depend on its text, only on its content kind
 * and the font metrics. So unlike widths, heights can be mirrored
 * exactly: this is generic_measure(), BatteryMeter_measure() and
 * date_content_measure() in sidebar_widgets.c, for the stacked (non-bar)
 * render spec both regions use.
 */

// METRIC_GOTHIC_*_HEIGHT in text_metrics.h: visible glyph height
const GOTHIC_HEIGHT: Record<number, number> = { 14: 8, 18: 11, 24: 14, 28: 18 };

const STACK_ICON_TEXT_GAP = 5;     // SIDEBAR_ICON_TEXT_GAP
const STACK_VALUE_GAP = 5;         // SIDEBAR_STACKED_VALUE_GAP
const STACK_LABEL_GAP = 3;         // SIDEBAR_BASIC_LABEL_GAP
const STACK_DIVIDER_H = 1;         // SIDEBAR_VALUE_PAIR_DIVIDER_HEIGHT
const BATTERY_ICON_H = 14;         // BatteryMeter_measure's icon height
const DATE_DAY_TOP_PADDING = 10;        // DATE_CALENDAR_DAY_TOP_PADDING
const DATE_DAY_BOTTOM_PADDING = 4;      // DATE_CALENDAR_DAY_BOTTOM_PADDING(_LARGE)
const DATE_PLAIN_VERTICAL_PADDING = 4;  // DATE_PLAIN_VERTICAL_PADDING

/** A widget's stacked height on the watch, in watch pixels. */
export const stackedWidgetHeight = (
    widgetId: number,
    settings: Settings,
    capabilities: Capabilities
): number => {
    const large = settings.largeSidebarFont;
    const f = sidebarFontSet(capabilities);
    const sm = GOTHIC_HEIGHT[f.sm];
    const md = GOTHIC_HEIGHT[f.md];
    const lg = GOTHIC_HEIGHT[f.lg];
    const current = large ? lg : md;
    const iconValue = WATCH_ICON_SIZE + STACK_ICON_TEXT_GAP + current;

    switch (widgetId) {
        case 2:  // Battery
            return BATTERY_ICON_H + STACK_ICON_TEXT_GAP + current;
        case 4: {  // Today's Date: weekday, calendar, month
            const calendarH = large
                ? current + DATE_PLAIN_VERTICAL_PADDING * 2
                : DATE_DAY_TOP_PADDING + current + DATE_DAY_BOTTOM_PADDING;
            return current + STACK_ICON_TEXT_GAP + calendarH + STACK_ICON_TEXT_GAP + current;
        }
        case 5:  // Seconds: plain text draws in the large font
            return lg;
        case 3: case 14: case 6: case 11: case 13: case 16: {
            // label/value: a blank label drops its line entirely
            const hasLabel = getWidgetItems(widgetId, settings).length > 1;
            return current + (hasLabel ? sm + STACK_LABEL_GAP : 0);
        }
        case 8:  // Today's Forecast: icon, high, divider, low
            return iconValue + STACK_VALUE_GAP + STACK_DIVIDER_H + STACK_VALUE_GAP + current;
        case 9:  // Sleep: icon, hours, minutes subtext
            return WATCH_ICON_SIZE + STACK_ICON_TEXT_GAP + md + STACK_VALUE_GAP + sm;
        case 7: case 10: case 12: case 15: case 17:
            return iconValue;
        default:
            return 0;
    }
};

const assessPx = (count: number, usedPx: number, capacityPx: number): FitResult => {
    const ratio = capacityPx > 0 ? usedPx / capacityPx : 0;
    const state: FitState =
        count === 0 ? 'good'
        : ratio > 1 ? 'over'
        : ratio > 1 - TIGHT_SLACK ? 'tight'
        : 'good';

    return { state, usedPx, capacityPx, ratio };
};

const stackedTotal = (widgetIds: number[], settings: Settings, capabilities: Capabilities) =>
    widgetIds.reduce((sum, id) => sum + stackedWidgetHeight(id, settings, capabilities), 0);

/*
 * The least space between two sidebar widgets that still reads as two
 * widgets rather than one run-on stack. The watch will draw closer than
 * this -- render_vertical_flow just keeps dividing what's left -- but
 * past it the widgets visibly crowd.
 */
const SIDEBAR_MIN_GAP = 4;

/**
 * How well a set of widgets fits the vertical sidebar. The sidebar is
 * always full screen height (the status bar yields its corner rather than
 * shortening it); a crowded one tightens its end padding to the compact
 * value, and "over" is when even then the widgets can't keep
 * SIDEBAR_MIN_GAP between them.
 */
export const assessSidebar = (
    widgetIds: number[],
    settings: Settings,
    capabilities: Capabilities
): FitResult => {
    const m = platformMetrics(capabilities, settings.largeSidebarFont);
    const gaps = Math.max(0, widgetIds.length - 1) * SIDEBAR_MIN_GAP;
    return assessPx(
        widgetIds.length,
        stackedTotal(widgetIds, settings, capabilities) + gaps,
        m.screenH - REGION_V_PADDING_COMPACT * 2
    );
};

/**
 * How well a set of widgets fits one round arc: the watch tightens the
 * arc's gaps down to the compact gap to keep the stack inside the arc's
 * content band, and "over" is when even that isn't enough and the stack
 * spills off the lens.
 */
export const assessArc = (
    widgetIds: number[],
    settings: Settings,
    capabilities: Capabilities
): FitResult => {
    const gaps = Math.max(0, widgetIds.length - 1) * ARC_WIDGET_GAP_COMPACT;
    return assessPx(
        widgetIds.length,
        stackedTotal(widgetIds, settings, capabilities) + gaps,
        Math.floor(arcContentBand(capabilities))
    );
};

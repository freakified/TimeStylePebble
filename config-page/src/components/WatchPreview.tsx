import React from 'react';
import { useConfig, useCapabilities } from '../context/PebbleConfigContext';
import { Capabilities, Settings } from '../context/types';
import {
    layoutHasSidebar,
    layoutHasBottomBar,
    layoutHasStackedBar,
    layoutSidebarOnLeft,
} from '../utils/layout';
import iconBattery from '../assets/preview/widget_previews/icon_battery.svg?raw';
import iconDate from '../assets/preview/widget_previews/icon_date.svg?raw';
import iconHrm from '../assets/preview/widget_previews/icon_hrm.svg?raw';
import iconSleep from '../assets/preview/widget_previews/icon_sleep.svg?raw';
import iconSteps from '../assets/preview/widget_previews/icon_steps.svg?raw';
import iconWeather from '../assets/preview/widget_previews/icon_weather.svg?raw';

export interface WatchPreviewProps {
    overrideSettings?: Partial<Settings>;
    isNight?: boolean;  // reserved for future night theme support
}

const TIME_PATHS = {
    hours: {
        avenir:     'M31.424 74.696h-6.912V15.272L12.224 25.736 8 20.936 25.28 6.728h6.144zM80.584 5q6.335 0 10.56 3.168 4.32 3.072 6.911 8.16 2.592 4.992 3.649 11.424a74 74 0 0 1 1.151 13.056q0 6.624-1.151 13.056-1.056 6.336-3.648 11.424-2.592 4.992-6.912 8.064-4.225 3.072-10.56 3.072-6.433 0-10.752-3.072-4.224-3.072-6.816-8.064-2.592-5.088-3.648-11.424a80.5 80.5 0 0 1-1.056-13.056q0-6.624 1.056-13.056 1.055-6.432 3.648-11.424 2.591-5.088 6.816-8.16Q74.152 5 80.584 5m0 5.856q-4.512 0-7.489 2.784-2.975 2.784-4.704 7.104-1.727 4.32-2.4 9.696a82 82 0 0 0-.671 10.368q0 5.088.672 10.368.671 5.28 2.4 9.6 1.727 4.224 4.704 7.008 2.975 2.688 7.487 2.688 4.417 0 7.392-2.688Q90.953 65 92.68 60.776q1.726-4.32 2.4-9.6.767-5.28.767-10.368T95.08 30.44q-.672-5.376-2.4-9.696-1.729-4.32-4.705-7.104t-7.391-2.784',
        avenirBold: 'M104.25 40.712q0 8.256-1.632 14.88-1.536 6.624-4.608 11.232-2.976 4.608-7.489 7.104-4.415 2.496-10.175 2.496t-10.273-2.496q-4.51-2.496-7.583-7.104t-4.705-11.232q-1.535-6.624-1.535-14.88t1.535-14.88q1.633-6.624 4.705-11.232 3.07-4.608 7.583-7.104T80.346 5q5.76 0 10.176 2.496 4.51 2.496 7.487 7.104 3.073 4.608 4.609 11.232 1.632 6.624 1.632 14.88m-11.808 0q0-3.552-.48-8.064-.386-4.512-1.728-8.544-1.248-4.032-3.648-6.72-2.4-2.784-6.24-2.784-3.936 0-6.337 2.784-2.4 2.688-3.743 6.72-1.25 4.032-1.729 8.544t-.48 8.064q0 3.552.48 8.064t1.728 8.448q1.345 3.936 3.745 6.72 2.4 2.688 6.335 2.688 3.84 0 6.24-2.688 2.4-2.784 3.649-6.72 1.343-3.936 1.728-8.448.48-4.512.48-8.064M28.16 20.744l-13.44 11.04L8 23.816 29.216 6.728H39.68v67.968H28.16z',
        leco:       'M57 74V7h47v67zm13.429-53.6v40.2H90.57V20.4zM8 20.4V7h26.667v53.6H48V74H8V60.6h13.333V20.4z',
    },
    minutes: {
        avenir:     'M48.488 74.696H5v-7.2l27.84-28.128a49 49 0 0 0 2.976-3.264 29 29 0 0 0 2.688-3.744 19 19 0 0 0 1.824-4.032Q41 26.12 41 23.816q0-2.976-1.056-5.376t-2.976-4.032q-1.824-1.728-4.32-2.592-2.4-.96-5.088-.96-5.472 0-9.216 3.456-3.744 3.36-4.8 8.64l-6.816-1.344a20.2 20.2 0 0 1 2.496-6.624q1.92-3.072 4.608-5.28a22 22 0 0 1 6.24-3.456Q23.528 5 27.56 5q4.128 0 7.776 1.248Q39.08 7.4 41.864 9.8q2.784 2.304 4.416 5.856 1.728 3.456 1.728 8.064 0 3.264-.96 6.048a23 23 0 0 1-2.4 5.28 30.2 30.2 0 0 1-3.456 4.8 135 135 0 0 1-4.224 4.416l-24.48 24.288h36zM76.336 36.104h2.976q3.264 0 6.144-.672t4.992-2.112a10.8 10.8 0 0 0 3.456-3.936q1.248-2.4 1.248-5.76 0-2.88-1.056-5.184-1.056-2.4-2.976-4.032-1.824-1.728-4.32-2.592-2.4-.96-5.088-.96-4.32 0-7.68 2.016-3.264 2.016-5.28 5.952l-5.856-3.072q3.168-5.472 8.064-8.064Q75.856 5 82.096 5q4.032 0 7.584 1.248 3.648 1.152 6.336 3.456 2.784 2.304 4.32 5.76 1.632 3.36 1.632 7.776 0 6.048-3.36 10.176-3.36 4.032-8.448 5.376v.192q5.856 1.248 9.792 5.664 3.936 4.32 3.936 11.232 0 4.896-1.824 8.736-1.728 3.744-4.704 6.432-2.976 2.592-7.008 3.936-3.936 1.44-8.352 1.44-7.488 0-13.056-3.552-5.568-3.648-8.448-10.368l6.528-2.4q1.728 4.896 5.76 7.68 4.032 2.688 9.216 2.688 2.88 0 5.568-.96 2.784-1.056 4.8-2.88 2.112-1.92 3.36-4.608t1.248-6.144q0-3.84-1.536-6.432-1.44-2.688-3.936-4.32t-5.664-2.304a27.6 27.6 0 0 0-6.528-.768h-2.976z',
        avenirBold: 'm4.48 64.136 26.208-25.728q3.168-3.072 4.992-6.144 1.92-3.168 1.92-7.2 0-4.8-3.168-7.584-3.072-2.88-7.68-2.88-4.896 0-7.872 3.36-2.976 3.264-3.648 8.256L4 24.488q.48-4.224 2.4-7.776a21.9 21.9 0 0 1 4.896-6.144q2.976-2.592 6.912-4.032Q22.24 5 26.944 5q4.416 0 8.448 1.248 4.128 1.248 7.296 3.84 3.168 2.496 4.992 6.24 1.92 3.648 1.92 8.544 0 3.264-.864 6.144a23 23 0 0 1-2.4 5.28 30.2 30.2 0 0 1-3.456 4.8 54 54 0 0 1-4.224 4.32l-19.584 18.72h30.624v10.56H4.48zM73.922 34.664h3.072q2.592 0 5.088-.384 2.592-.48 4.704-1.632t3.36-3.072q1.344-2.016 1.344-5.184 0-2.304-.864-4.128a8.7 8.7 0 0 0-2.4-3.072q-1.44-1.248-3.456-1.92a12.1 12.1 0 0 0-4.032-.672q-4.224 0-7.008 2.4t-3.84 6.336l-10.848-2.784q.864-3.456 2.88-6.24a21.3 21.3 0 0 1 4.992-4.896q2.976-2.112 6.624-3.264T81.218 5q4.416 0 8.352 1.248Q93.602 7.4 96.578 9.8a16.1 16.1 0 0 1 4.8 5.76q1.728 3.456 1.728 8.064 0 5.472-3.36 9.792-3.264 4.224-8.832 5.568v.192q6.24 1.248 10.08 5.664 3.936 4.32 3.936 10.656 0 5.28-2.112 9.216-2.016 3.936-5.472 6.528-3.36 2.592-7.584 3.84a28 28 0 0 1-8.544 1.344q-4.32 0-8.16-.96-3.84-.864-7.104-2.784-3.168-1.92-5.664-4.992-2.4-3.072-3.744-7.296l10.752-3.36q1.248 4.032 4.608 6.816 3.456 2.784 8.928 2.784 2.208 0 4.416-.576 2.304-.672 4.032-2.016 1.824-1.344 2.88-3.456 1.152-2.112 1.152-5.184 0-3.264-1.632-5.472-1.536-2.304-3.936-3.552-2.4-1.344-5.376-1.92a28.4 28.4 0 0 0-5.568-.576h-2.88z',
        leco:       'M5 27.1V7h47v40.2H18.429v13.4H52V74H5V33.8h33.571V20.4H18.43v6.7zM104 7v67H57V60.6h33.571V47.2H63.714V33.8h26.857V20.4H57V7z',
    },
} as const;

function getTimePaths(clockFont: Settings['clockFont']) {
    const hKey = (clockFont === 'bold' || clockFont === 'bold-h') ? 'avenirBold'
               : clockFont === 'leco' ? 'leco' : 'avenir';
    const mKey = (clockFont === 'bold' || clockFont === 'bold-m') ? 'avenirBold'
               : clockFont === 'leco' ? 'leco' : 'avenir';
    return { hourPath: TIME_PATHS.hours[hKey], minutePath: TIME_PATHS.minutes[mKey] };
}

/* ── Widget rendering ─────────────────────────────────────────────────
 * Widgets mirror the watch's two presentation modes: "stacked" (icon
 * above text — sidebars, round arcs, and the standalone bottom bar) and
 * "inline" (icon beside text — the narrow bottom bar). Each widget is
 * described as a vertical list of items; the inline renderer re-derives
 * an icon + single-line form from the same list.
 */

interface IconAsset {
    svg: string;
    w: number;
    h: number;
}

const ICONS = {
    battery: { svg: iconBattery, w: 25, h: 18 },
    date:    { svg: iconDate,    w: 25, h: 25 },
    hrm:     { svg: iconHrm,     w: 25, h: 25 },
    sleep:   { svg: iconSleep,   w: 25, h: 22 },
    steps:   { svg: iconSteps,   w: 25, h: 18 },
    weather: { svg: iconWeather, w: 25, h: 25 },
} as const satisfies Record<string, IconAsset>;

type WidgetItem =
    | { type: 'icon'; asset: IconAsset }
    | { type: 'calendar'; day: string }   // date icon with the day number inside
    | { type: 'text'; text: string; size: number }
    | { type: 'divider' };

const STACK_GAP = 2;
const TEXT_FONT = "-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif";
const TEXT_CHAR_W = 0.62;  // average bold advance as a fraction of font size

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

const getWidgetItems = (widgetId: number, settings: Settings, compact = false): WidgetItem[] => {
    const now = new Date();

    switch (widgetId) {
        case 2: {  // Battery
            const items: WidgetItem[] = [{ type: 'icon', asset: ICONS.battery }];
            if (settings.batteryMeter === 'icon-and-percent') items.push(text('80%'));
            return items;
        }
        case 3:  // Alternate Time Zone
            return [text(settings.altClockName || 'ALT'), text('5p')];
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
            return [{ type: 'icon', asset: ICONS.steps }, text(settings.seamlessSidebar ? '2700' : '2.7k')];
        case 11:  // Swatch Internet Time
            return [text('@'), text('543')];
        case 12:  // Heart Rate
            return [{ type: 'icon', asset: ICONS.hrm }, text('72')];
        case 13:  // UV Index
            return [text('UV'), text('6')];
        default:
            return [];
    }
};

const measureItem = (item: WidgetItem): { w: number; h: number } => {
    switch (item.type) {
        case 'icon': return { w: item.asset.w, h: item.asset.h };
        case 'calendar': return { w: ICONS.date.w, h: ICONS.date.h };
        case 'text': return { w: item.text.length * item.size * TEXT_CHAR_W, h: item.size + 1 };
        case 'divider': return { w: 14, h: 3 };
    }
};

interface WidgetSpec {
    items: WidgetItem[];
    w: number;
    h: number;
}

const getWidgetSpec = (widgetId: number, settings: Settings, compact = false): WidgetSpec => {
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

const getHex = (color: string) => color ? (color.startsWith('#') ? color : `#${color}`) : '#000000';

const isBlack = (color: string) => {
    const normalized = color.toLowerCase();
    return normalized === '#000' || normalized === '#000000';
};

interface RegionColors {
    background: string;  // region fill
    text: string;        // widget text color
    iconFill: string;
    iconStroke: string;
}

// "Standard" region appearance, recolored the way the watch does: white
// fills stay white on a colored sidebar with black strokes; on a black
// sidebar the fills go black and the strokes take the sidebar text color.
const standardRegionColors = (s: Settings): RegionColors => {
    const background = getHex(s.colorSidebar);
    const text = getHex(s.colorSidebarText);
    return isBlack(background)
        ? { background, text, iconFill: '#000000', iconStroke: text }
        : { background, text, iconFill: '#ffffff', iconStroke: '#000000' };
};

// "Dark" region appearance (the bottom bar in two-sidebar layouts): draws
// on the watchface background, strokes icons with the time color, and uses
// the alt sidebar text color.
const darkRegionColors = (s: Settings): RegionColors => ({
    background: getHex(s.colorBackground),
    text: getHex(s.colorAltSidebarText),
    iconFill: getHex(s.colorBackground),
    iconStroke: getHex(s.colorTime),
});

const getTintedIconDataUri = (svg: string, colors: RegionColors) => {
    const tinted = svg.replace(/#fff/gi, colors.iconFill).replace(/#000/gi, colors.iconStroke);
    return `data:image/svg+xml;utf8,${encodeURIComponent(tinted)}`;
};

const textNode = (
    key: React.Key,
    str: string,
    x: number,
    y: number,
    fontSize: number,
    fill: string,
    anchor: 'middle' | 'start' = 'middle'
) => (
    <text
        key={key}
        x={x}
        y={y}
        textAnchor={anchor}
        dominantBaseline="central"
        fill={fill}
        fontSize={fontSize}
        fontFamily={TEXT_FONT}
        fontWeight="bold"
    >
        {str}
    </text>
);

const renderIconImage = (
    key: React.Key,
    asset: IconAsset,
    x: number,
    y: number,
    w: number,
    h: number,
    colors: RegionColors
) => (
    <image
        key={key}
        href={getTintedIconDataUri(asset.svg, colors)}
        x={x}
        y={y}
        width={w}
        height={h}
        preserveAspectRatio="xMidYMid meet"
    />
);

// The day number sits inside the calendar body (below the binding rings).
const CALENDAR_DAY_CENTER_Y = 14;

const renderCalendar = (
    key: React.Key,
    day: string,
    x: number,
    y: number,
    scale: number,
    colors: RegionColors
) => (
    <g key={key}>
        {renderIconImage('bg', ICONS.date, x, y, ICONS.date.w * scale, ICONS.date.h * scale, colors)}
        {textNode('day', day, x + (ICONS.date.w / 2) * scale, y + CALENDAR_DAY_CENTER_Y * scale,
                  9 * scale, colors.iconStroke)}
    </g>
);

/** Draw a widget's items top-to-bottom, horizontally centered on `cx`. */
const renderItemsStacked = (
    items: WidgetItem[],
    cx: number,
    top: number,
    scale: number,
    colors: RegionColors
) => {
    let y = top;
    return items.map((item, i) => {
        const size = measureItem(item);
        const itemTop = y;
        y += (size.h + STACK_GAP) * scale;

        switch (item.type) {
            case 'icon': {
                const w = size.w * scale;
                const h = size.h * scale;
                return renderIconImage(i, item.asset, cx - w / 2, itemTop, w, h, colors);
            }
            case 'calendar':
                return renderCalendar(i, item.day, cx - (size.w * scale) / 2, itemTop, scale, colors);
            case 'text':
                return textNode(i, item.text, cx, itemTop + (size.h * scale) / 2, item.size * scale, colors.text);
            case 'divider':
                return (
                    <line
                        key={i}
                        x1={cx - (size.w * scale) / 2}
                        x2={cx + (size.w * scale) / 2}
                        y1={itemTop + 1.5 * scale}
                        y2={itemTop + 1.5 * scale}
                        stroke={colors.text}
                        strokeWidth={1}
                    />
                );
        }
    });
};

/** Draw a widget centered on (cx, centerY), scaled to fit maxW × maxH. */
const renderStackedWidget = (
    widgetId: number,
    settings: Settings,
    cx: number,
    centerY: number,
    maxW: number,
    maxH: number,
    colors: RegionColors,
    extraScale = 1,
    compact = false
) => {
    const spec = getWidgetSpec(widgetId, settings, compact);
    if (spec.items.length === 0) return null;

    const scale = Math.min(maxW / spec.w, maxH / spec.h, 1) * extraScale;
    return renderItemsStacked(spec.items, cx, centerY - (spec.h * scale) / 2, scale, colors);
};

/* ── Inline (narrow bottom bar) rendering ── */

const INLINE_ICON_H = 12;
const INLINE_GAP = 3;
const INLINE_FONT = 9;

interface InlineParts {
    icon?: { asset: IconAsset } | { calendar: string };
    text: string;
}

const getInlineParts = (widgetId: number, settings: Settings): InlineParts | null => {
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

const measureInline = (parts: InlineParts): number => {
    let w = 0;
    if (parts.icon) {
        const asset = 'asset' in parts.icon ? parts.icon.asset : ICONS.date;
        w += asset.w * (INLINE_ICON_H / asset.h);
        if (parts.text) w += INLINE_GAP;
    }
    w += parts.text.length * INLINE_FONT * TEXT_CHAR_W;
    return w;
};

const renderInlineWidget = (
    parts: InlineParts,
    x: number,
    centerY: number,
    colors: RegionColors
) => {
    const nodes: React.ReactNode[] = [];
    let cursor = x;

    if (parts.icon) {
        const asset = 'asset' in parts.icon ? parts.icon.asset : ICONS.date;
        const scale = INLINE_ICON_H / asset.h;
        const w = asset.w * scale;
        const top = centerY - INLINE_ICON_H / 2;

        if ('asset' in parts.icon) {
            nodes.push(renderIconImage('icon', asset, cursor, top, w, INLINE_ICON_H, colors));
        } else {
            nodes.push(renderCalendar('icon', parts.icon.calendar, cursor, top, scale, colors));
        }
        cursor += w + (parts.text ? INLINE_GAP : 0);
    }

    if (parts.text) {
        nodes.push(textNode('text', parts.text, cursor, centerY, INLINE_FONT, colors.text, 'start'));
    }

    return nodes;
};

/* ── Region geometry (mirrors sidebar_layout.h, in 144×168 units) ── */

const SIDEBAR_W_NARROW = 30;
const SIDEBAR_W_WIDE = 46;
const BAR_H_INLINE = 30;
const BAR_H_STACKED = 50;
const BAR_SIDEBAR_GAP = 2;
const BAR_PADDING_X = 4;
const SIDEBAR_V_PADDING = 8;
const SIDEBAR_MIN_SLOT_GAP = 4;

/** Render a rectangular (basalt/diorite/emery) TimeStyle face */
const RectPreview: React.FC<{ effectiveSettings: Settings; capabilities: Capabilities }> = ({
    effectiveSettings: s,
    capabilities,
}) => {
    const bg = getHex(s.colorBackground);
    const timeColor = getHex(s.colorTime);

    const W = 144, H = 168;

    const hasSidebar = layoutHasSidebar(s.layout);
    const hasBar = layoutHasBottomBar(s.layout);

    // the primary sidebar follows the seamless setting (drawing on the
    // watchface background and widening); a secondary bottom bar is
    // always seamless
    const colors = s.seamlessSidebar ? darkRegionColors(s) : standardRegionColors(s);
    const barColors = hasSidebar ? darkRegionColors(s) : colors;

    const stackedBar = layoutHasStackedBar(s.layout);
    const sidebarOnLeft = layoutSidebarOnLeft(s.layout);

    const SIDEBAR_W = hasSidebar ? (s.seamlessSidebar ? SIDEBAR_W_WIDE : SIDEBAR_W_NARROW) : 0;
    const BAR_H = hasBar ? (stackedBar ? BAR_H_STACKED : BAR_H_INLINE) : 0;

    const sidebarX = sidebarOnLeft ? 0 : W - SIDEBAR_W;
    const mainX = sidebarOnLeft ? SIDEBAR_W : 0;
    const mainW = W - SIDEBAR_W;

    const { hourPath, minutePath } = getTimePaths(s.clockFont);

    /* Sidebar: first and last slots pin to the ends, interior slots
     * spread evenly between them (the watch's vertical flow). */
    const renderSidebar = () => {
        const slotIds = capabilities.EMERY
            ? [s.widget0Id, s.widget1Id, s.widget2Id, s.widget3Id]
            : [s.widget0Id, s.widget1Id, s.widget2Id];

        const maxW = SIDEBAR_W - 2;
        const specs = slotIds.map((id) => getWidgetSpec(id, s));
        const baseScales = specs.map((spec) => (spec.w > 0 ? Math.min(maxW / spec.w, 1) : 1));

        const naturalTotal = specs.reduce((sum, spec, i) => sum + spec.h * baseScales[i], 0);
        const avail = H - SIDEBAR_V_PADDING * 2 - SIDEBAR_MIN_SLOT_GAP * (slotIds.length - 1);
        const fit = naturalTotal > 0 ? Math.min(1, avail / naturalTotal) : 1;

        const heights = specs.map((spec, i) => spec.h * baseScales[i] * fit);
        const cx = sidebarX + SIDEBAR_W / 2;
        const last = slotIds.length - 1;

        const tops: number[] = new Array(slotIds.length).fill(0);
        tops[0] = SIDEBAR_V_PADDING;
        tops[last] = H - SIDEBAR_V_PADDING - heights[last];

        // interior widgets spread evenly between the pinned ends
        const interiorTotal = heights.slice(1, last).reduce((a, b) => a + b, 0);
        let y = tops[0] + heights[0];
        let remaining = tops[last] - y - interiorTotal;
        let gapsLeft = last;  // interior count + 1

        for (let i = 1; i < last; i++) {
            const gap = remaining / gapsLeft;
            remaining -= gap;
            gapsLeft--;
            y += gap;
            tops[i] = y;
            y += heights[i];
        }

        return (
            <>
                <rect x={sidebarX} y={0} width={SIDEBAR_W} height={H} fill={colors.background} />
                {slotIds.map((id, i) => (
                    <g key={`sidebar-${i}`}>
                        {renderItemsStacked(getWidgetItems(id, s), cx, tops[i], baseScales[i] * fit, colors)}
                    </g>
                ))}
            </>
        );
    };

    /* Bottom bar: yields its corner to the sidebar with a small gap. */
    const renderBottomBar = () => {
        const reserved = hasSidebar ? SIDEBAR_W + BAR_SIDEBAR_GAP : 0;
        const barX = sidebarOnLeft ? reserved : 0;
        const barW = W - reserved;
        const barY = H - BAR_H;

        const slotIds = stackedBar
            ? [s.bottomBarWidget0Id, s.bottomBarWidget1Id, s.bottomBarWidget2Id, s.bottomBarWidget3Id]
            : [s.bottomBarWidget0Id, s.bottomBarWidget1Id];
        const visible = slotIds.filter((id) => id !== 0);

        let content: React.ReactNode = null;

        if (stackedBar && visible.length > 0) {
            // equal columns, widgets stacked and centered per column
            const usable = barW - BAR_PADDING_X * 2;
            const colW = usable / visible.length;
            content = visible.map((id, i) => (
                <g key={`bar-${i}`}>
                    {renderStackedWidget(
                        id, s,
                        barX + BAR_PADDING_X + colW * i + colW / 2,
                        barY + BAR_H / 2,
                        colW - 4, BAR_H - 6,
                        barColors, 1, true
                    )}
                </g>
            ));
        } else if (!stackedBar) {
            // widgets distribute space-evenly along the bar
            const parts = visible
                .map((id) => getInlineParts(id, s))
                .filter((p): p is InlineParts => p !== null);
            const widths = parts.map(measureInline);
            const totalW = widths.reduce((a, b) => a + b, 0);
            let remaining = barW - totalW;
            let gapsLeft = parts.length + 1;
            let x = barX;

            content = parts.map((p, i) => {
                const gap = remaining / gapsLeft;
                remaining -= gap;
                gapsLeft--;
                x += gap;
                const node = (
                    <g key={`bar-${i}`}>{renderInlineWidget(p, x, barY + BAR_H / 2, barColors)}</g>
                );
                x += widths[i];
                return node;
            });
        }

        return (
            <>
                <rect x={barX} y={barY} width={barW} height={BAR_H} fill={barColors.background} />
                {content}
            </>
        );
    };

    /* Time: centered in the area left over by the regions. */
    const renderTime = () => {
        const availH = H - BAR_H;
        const SVG_W = Math.min(108, mainW - 6, Math.floor(((availH - 6) / 2) * 109 / 81));
        const SVG_H = Math.round(SVG_W * 81 / 109);
        const timeX = mainX + (mainW - SVG_W) / 2;
        const timeY = (availH - SVG_H * 2) / 2;
        return (
            <>
                <svg x={timeX} y={timeY} width={SVG_W} height={SVG_H} viewBox="0 0 109 81">
                    <path d={hourPath} fill={timeColor} />
                </svg>
                <svg x={timeX} y={timeY + SVG_H} width={SVG_W} height={SVG_H} viewBox="0 0 109 81">
                    <path d={minutePath} fill={timeColor} />
                </svg>
            </>
        );
    };

    return (
        <div className="halite-watch-preview-container">
            <div className="halite-watch-face">
                <svg
                    xmlns="http://www.w3.org/2000/svg"
                    width={W}
                    height={H}
                    viewBox={`0 0 ${W} ${H}`}
                    fill="none"
                    className="halite-watch-preview"
                >
                    {/* Main background */}
                    <rect x={0} y={0} width={W} height={H} fill={bg} />

                    {renderTime()}
                    {hasSidebar && renderSidebar()}
                    {hasBar && renderBottomBar()}

                    {/* Border */}
                    <rect x={0.5} y={0.5} width={W - 1} height={H - 1} stroke="#333333" strokeWidth="1" fill="none" />
                </svg>
            </div>
        </div>
    );
};

/** Render a round (chalk/gabbro) TimeStyle face */
const RoundPreview: React.FC<{ effectiveSettings: Settings }> = ({ effectiveSettings: s }) => {
    const bg = getHex(s.colorBackground);
    const timeColor = getHex(s.colorTime);
    const colors = standardRegionColors(s);

    const { hourPath, minutePath } = getTimePaths(s.clockFont);

    const CX = 90, CY = 90, R = 90;

    return (
        <div className="halite-watch-preview-container">
            <div className="halite-watch-face halite-watch-face--round">
                <svg
                    xmlns="http://www.w3.org/2000/svg"
                    width={180}
                    height={180}
                    viewBox="0 0 180 180"
                    fill="none"
                    className="halite-watch-preview"
                >
                    {/* Main circular background */}
                    <circle cx={CX} cy={CY} r={R} fill={bg} />

                    {/* Sidebar circles — large circles positioned outside the viewport to create curved sidebar bands */}
                    <circle cx={-50} cy={CY} r={R} fill={colors.background} />
                    <circle cx={230} cy={CY} r={R} fill={colors.background} />

                    {/* Widgets — one per arc, stacked presentation */}
                    {renderStackedWidget(s.widget0Id, s, 20, 90, 28, 110, colors)}
                    {renderStackedWidget(s.widget1Id, s, 160, 90, 28, 110, colors)}

                    {/* Time display — hours stacked above minutes */}
                    {(() => {
                        const SVG_W = 95;
                        const SVG_H = Math.round(SVG_W * 81 / 109);
                        const timeX = (180 - SVG_W) / 2;
                        const timeY = (180 - SVG_H * 2) / 2;
                        return (
                            <>
                                <svg x={timeX} y={timeY} width={SVG_W} height={SVG_H} viewBox="0 0 109 81">
                                    <path d={hourPath} fill={timeColor} />
                                </svg>
                                <svg x={timeX} y={timeY + SVG_H} width={SVG_W} height={SVG_H} viewBox="0 0 109 81">
                                    <path d={minutePath} fill={timeColor} />
                                </svg>
                            </>
                        );
                    })()}

                    {/* Circle border */}
                    <circle cx={CX} cy={CY} r={R - 0.5} stroke="#333333" strokeWidth="1" fill="none" />
                </svg>
            </div>
        </div>
    );
};

export const WatchPreview: React.FC<WatchPreviewProps> = ({ overrideSettings, isNight: _isNight = false }) => {
    const { settings } = useConfig();
    const capabilities = useCapabilities();

    const effectiveSettings = { ...settings, ...overrideSettings } as Settings;

    if (capabilities.ROUND) {
        return <RoundPreview effectiveSettings={effectiveSettings} />;
    }
    return <RectPreview effectiveSettings={effectiveSettings} capabilities={capabilities} />;
};

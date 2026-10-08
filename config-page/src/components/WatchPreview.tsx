import React from 'react';
import { useConfig, useCapabilities } from '../context/PebbleConfigContext';
import { Capabilities, Settings } from '../context/types';
import {
    ARC_SLOT_CAP,
    arcIsAccent,
    barHasWidgets,
    layoutSidebarOnLeft,
} from '../utils/layout';
import {
    ICONS,
    IconAsset,
    WidgetSpec,
    getWidgetSpec,
    stackGap,
    InlineParts,
    getInlineParts,
    measureInline,
    INLINE_GAP,
    fitText,
    PREVIEW_CAP_HEIGHT,
    platformMetrics,
    arcMetrics,
    arcContentBand,
    previewFonts,
    PreviewFonts,
    stackedWidgetHeight,
    ARC_WIDGET_GAP_COMPACT,
    ARC_INWARD_NUDGE,
    REGION_V_PADDING_COMPACT,
    WATCH_BAR_PADDING_X,
    WATCH_BAR_PADDING_X_COMPACT,
    WATCH_BAR_VBAR_GAP,
    WATCH_BAR_WIDGET_GAP_COMPACT,
} from '../utils/widgetMetrics';

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
 * above text — sidebars and round arcs) and "inline" (icon beside text —
 * the bottom bar). Each widget is described as a vertical list of items;
 * the inline renderer re-derives an icon + single-line form from the same
 * list.
 */

// Roboto Condensed Bold, bundled in index.css, stands in for Gothic Bold
const TEXT_FONT = "'Roboto Condensed', 'Arial Narrow', sans-serif";

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

// "Dark" region appearance (the bottom bar): draws on the watchface
// background, strokes icons with the time color, and uses the alt
// sidebar text color.
const darkRegionColors = (s: Settings): RegionColors => ({
    background: getHex(s.colorBackground),
    text: getHex(s.colorAltSidebarText),
    iconFill: getHex(s.colorBackground),
    iconStroke: getHex(s.colorTime),
});

const getTintedIconDataUri = (svg: string, colors: RegionColors) => {
    // match whole short or long hex colors: the Illustrator exports spell
    // white #FFFFFF, and a bare /#fff/ would leave its tail behind. One
    // pass, so a black fill (dark themes) isn't then re-tinted as stroke.
    const tinted = svg.replace(/#(?:fff(?:fff)?|000(?:000)?)\b/gi, (hex) =>
        hex[1].toLowerCase() === 'f' ? colors.iconFill : colors.iconStroke);
    return `data:image/svg+xml;utf8,${encodeURIComponent(tinted)}`;
};

const textNode = (
    key: React.Key,
    str: string,
    x: number,
    capTop: number,
    fontSize: number,
    fill: string,
    anchor: 'middle' | 'start' = 'middle'
) => (
    // sat on its baseline so the caps span exactly capTop..capTop+capH,
    // the box the watch's text metrics lay out
    <text
        key={key}
        x={x}
        y={capTop + fontSize * PREVIEW_CAP_HEIGHT}
        textAnchor={anchor}
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
const CALENDAR_DAY_FONT = 10;

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
        {textNode('day', day, x + (ICONS.date.w / 2) * scale,
                  y + CALENDAR_DAY_CENTER_Y * scale - (CALENDAR_DAY_FONT * PREVIEW_CAP_HEIGHT * scale) / 2,
                  CALENDAR_DAY_FONT * scale, colors.iconStroke)}
    </g>
);

/** Draw a widget's items top-to-bottom, horizontally centered on `cx`. */
const renderItemsStacked = (
    spec: WidgetSpec,
    cx: number,
    top: number,
    colors: RegionColors,
    fonts: PreviewFonts,
    maxW: number
) => {
    let y = top;
    return spec.items.map((item, i) => {
        if (i > 0) y += stackGap(spec.items[i - 1], item);
        const size = spec.sizes[i];
        const itemTop = y;
        y += size.h;

        switch (item.type) {
            case 'icon':
                return renderIconImage(i, item.asset, cx - size.w / 2, itemTop, size.w, size.h, colors);
            case 'calendar':
                return renderCalendar(i, item.day, cx - size.w / 2, itemTop, 1, colors);
            case 'text':
                return textNode(i, item.text, cx, itemTop,
                                fitText(item.text, item.font, fonts, maxW).fontSize, colors.text);
            case 'divider':
                return (
                    <line
                        key={i}
                        x1={cx - size.w / 2}
                        x2={cx + size.w / 2}
                        y1={itemTop + size.h / 2}
                        y2={itemTop + size.h / 2}
                        stroke={colors.text}
                        strokeWidth={size.h}
                    />
                );
        }
    });
};

/* ── Inline (bottom bar) rendering ── */

const renderInlineWidget = (
    parts: InlineParts,
    x: number,
    centerY: number,
    colors: RegionColors,
    fonts: PreviewFonts
) => {
    const nodes: React.ReactNode[] = [];
    let cursor = x;

    if (parts.icon) {
        const asset = 'asset' in parts.icon ? parts.icon.asset : ICONS.date;
        // full watch size, centered on the bar like the watch's
        // SIDEBAR_ICON_SIZE icon frame
        const top = centerY - asset.h / 2;

        if ('asset' in parts.icon) {
            nodes.push(renderIconImage('icon', asset, cursor, top, asset.w, asset.h, colors));
        } else {
            nodes.push(renderCalendar('icon', parts.icon.calendar, cursor, top, 1, colors));
        }
        cursor += asset.w + (parts.text ? INLINE_GAP : 0);
    }

    if (parts.text) {
        const fitted = fitText(parts.text, 'current', fonts);
        nodes.push(textNode('text', parts.text, cursor, centerY - fitted.capH / 2,
                            fitted.fontSize, colors.text, 'start'));
    }

    return nodes;
};

/*
 * Both previews draw in the platform's real screen pixels, taking their
 * region geometry from platformMetrics() / arcMetrics() -- the one place
 * the config page mirrors sidebar_layout.h -- and let CSS scale the SVG to
 * whatever box it's shown in.
 *
 * The clock's digits are sized off the clock area's height, as
 * update_clock_area_layer() does (its font size is 4/7 of that height),
 * calibrated so that a full-height basalt clock draws 80px tall glyph
 * boxes.
 */
const TIME_GLYPH_H_PER_CLOCK_H = 80 / 168;
const TIME_GLYPH_ASPECT = 109 / 81;  // the TIME_PATHS viewBox

/** Render a rectangular (basalt/diorite/emery) TimeStyle face */
const RectPreview: React.FC<{ effectiveSettings: Settings; capabilities: Capabilities }> = ({
    effectiveSettings: s,
    capabilities,
}) => {
    const bg = getHex(s.colorBackground);
    const timeColor = getHex(s.colorTime);

    const m = platformMetrics(capabilities, s.largeSidebarFont);
    const W = m.screenW, H = m.screenH;
    const fonts = previewFonts(capabilities, s.largeSidebarFont);
    const barClipId = React.useId();

    // the bar is on screen exactly when it holds a widget, the same rule
    // sidebar_buildRegions() applies on the watch
    const barSlotIds = [
        s.bottomBarWidget0Id, s.bottomBarWidget1Id, s.bottomBarWidget2Id,
    ].slice(0, m.barSlots);
    const hasBar = barHasWidgets(barSlotIds);

    // the sidebar always wears the accent color; the bottom bar draws on
    // the watchface background instead
    const colors = standardRegionColors(s);
    const barColors = darkRegionColors(s);

    const sidebarOnLeft = layoutSidebarOnLeft(s.layout);

    const SIDEBAR_W = s.wideSidebar ? m.vbarWide : m.vbarNarrow;
    const barH = hasBar ? m.barHeight : 0;

    const sidebarX = sidebarOnLeft ? 0 : W - SIDEBAR_W;
    const mainX = sidebarOnLeft ? SIDEBAR_W : 0;
    const mainW = W - SIDEBAR_W;

    const { hourPath, minutePath } = getTimePaths(s.clockFont);

    /* Sidebar: the slots form a packed widget list; the first pins to the
     * top, the last to the bottom, interior widgets spread evenly between
     * them (the watch's vertical flow).
     *
     * Widgets are NOT scaled down to make a crowded stack fit. The watch
     * doesn't do that — render_vertical_flow draws every widget at full
     * size and its only concession to a full sidebar is tightening the end
     * padding, after which widgets simply crowd into each other. Mirroring
     * that is what keeps this preview honest with the budget meter: an
     * over-budget config has to *look* over-budget here too.
     *
     * So the stack is laid out with the watch's own widget heights (the
     * same ones the fit budget sums), and each stylized widget is drawn
     * centered in its slot. */
    const renderSidebar = () => {
        const slotIds = [s.widget0Id, s.widget1Id, s.widget2Id, s.widget3Id, s.widget4Id, s.widget5Id]
            .slice(0, m.sidebarSlots)
            .filter((id) => id !== 0);

        if (slotIds.length === 0) {
            return <rect x={sidebarX} y={0} width={SIDEBAR_W} height={H} fill={colors.background} />;
        }

        const maxW = SIDEBAR_W - 2;
        const specs = slotIds.map((id) => getWidgetSpec(id, s, fonts, maxW));

        const heights = slotIds.map((id) => stackedWidgetHeight(id, s, capabilities));
        const naturalTotal = heights.reduce((a, b) => a + b, 0);

        // the watch's one concession to a crowded stack: tighter end padding
        const vPadding = naturalTotal > H - m.vPadding * 2
            ? REGION_V_PADDING_COMPACT
            : m.vPadding;

        const cx = sidebarX + SIDEBAR_W / 2;
        const last = slotIds.length - 1;

        const tops: number[] = new Array(slotIds.length).fill(0);
        tops[0] = vPadding;
        if (last > 0) {
            tops[last] = H - vPadding - heights[last];
        }

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
                        {renderItemsStacked(specs[i], cx, tops[i] + (heights[i] - specs[i].h) / 2,
                                            colors, fonts, maxW)}
                    </g>
                ))}
            </>
        );
    };

    /* Bottom bar: yields its corner to the sidebar with a small gap. Its
     * widgets cluster as one centered group with a fixed gap. */
    const renderBottomBar = () => {
        const reserved = SIDEBAR_W + WATCH_BAR_VBAR_GAP;
        const barX = sidebarOnLeft ? reserved : 0;
        const barW = W - reserved;
        const barY = H - barH;

        // mirror the watch's rendered cap: two slots, three on emery
        const parts = barSlotIds
            .filter((id) => id !== 0)
            .map((id) => getInlineParts(id, s))
            .filter((p): p is InlineParts => p !== null);
        const widths = parts.map((p) => measureInline(p, fonts));
        const totalW = widths.reduce((a, b) => a + b, 0);
        const contentW = barW - WATCH_BAR_PADDING_X * 2;
        const gapCount = parts.length - 1;

        let gap = m.barWidgetGap;
        if (gapCount > 0 && totalW + gap * gapCount > contentW) {
            gap = Math.max(WATCH_BAR_WIDGET_GAP_COMPACT, (contentW - totalW) / gapCount);
        }

        let x = Math.max(
            barX + WATCH_BAR_PADDING_X_COMPACT,
            barX + (barW - (totalW + gap * gapCount)) / 2
        );

        const content = parts.map((p, i) => {
            const node = (
                <g key={`bar-${i}`}>{renderInlineWidget(p, x, barY + barH / 2, barColors, fonts)}</g>
            );
            x += widths[i] + gap;
            return node;
        });

        return (
            <>
                <rect x={barX} y={barY} width={barW} height={barH} fill={barColors.background} />
                {/* the watch clips an overfull bar at its edges rather than
                    letting it run into the sidebar */}
                <clipPath id={barClipId}>
                    <rect x={barX} y={barY} width={barW} height={barH} />
                </clipPath>
                <g clipPath={`url(#${barClipId})`}>{content}</g>
            </>
        );
    };

    /* Time: centered in the area left over by the regions. */
    const renderTime = () => {
        // a bar lends the clock the padding above its widgets
        const availH = H - (hasBar ? barH - m.barClockEncroach : 0);
        const SVG_H = Math.round(availH * TIME_GLYPH_H_PER_CLOCK_H);
        const SVG_W = Math.round(SVG_H * TIME_GLYPH_ASPECT);
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
                    // the preview box is basalt-shaped; emery is ~2% wider
                    // for its height, so stretch rather than letterbox
                    preserveAspectRatio="none"
                    fill="none"
                    className="halite-watch-preview"
                >
                    {/* Main background */}
                    <rect x={0} y={0} width={W} height={H} fill={bg} />

                    {renderTime()}
                    {renderSidebar()}
                    {hasBar && renderBottomBar()}

                    {/* Border */}
                    <rect x={0.5} y={0.5} width={W - 1} height={H - 1} stroke="#333333" strokeWidth="1" fill="none" />
                </svg>
            </div>
        </div>
    );
};

/* update_clock_area_layer() insets the round clock by this, top and bottom */
const ROUND_VERTICAL_PADDING = 15;

/** Render a round (chalk/gabbro) TimeStyle face */
const RoundPreview: React.FC<{ effectiveSettings: Settings; capabilities: Capabilities }> = ({
    effectiveSettings: s,
    capabilities,
}) => {
    const bg = getHex(s.colorBackground);
    const timeColor = getHex(s.colorTime);

    const { hourPath, minutePath } = getTimePaths(s.clockFont);

    const arc = arcMetrics(capabilities);
    // ROUND_ARC_CONTENT_BAND is the same figure, rounded down
    const band = Math.floor(arcContentBand(capabilities));
    const fonts = previewFonts(capabilities, s.largeSidebarFont);

    const S = arc.screen;
    const CX = S / 2, CY = S / 2, R = S / 2;

    const slotIds = [
        s.widget0Id, s.widget1Id, s.widget2Id,
        s.widget3Id, s.widget4Id, s.widget5Id,
    ];

    /*
     * Each arc is a circle the size of the screen, offset so only a
     * sliver shows down its edge — the same construction draw_round_arc
     * uses, which is what makes the arc taper away from its waist. An
     * arc that isn't the accent one draws no circle at all: the arcs sit
     * on top of the clock area, so filling a seamless arc with the
     * watchface color would punch a hole in the time.
     */
    const renderArc = (side: 'left' | 'right') => {
        const accent = arcIsAccent(s.layout, s.colorBothArcs, side);
        const colors = accent ? standardRegionColors(s) : darkRegionColors(s);

        const ids = (side === 'left'
            ? slotIds.slice(0, ARC_SLOT_CAP)
            : slotIds.slice(ARC_SLOT_CAP, ARC_SLOT_CAP * 2)
        ).filter((id) => id !== 0);

        // the widget column is centered across the arc, nudged towards the
        // screen center (ROUND_ARC_CONTENT_INSET), straight down the arc
        const inset = arc.width / 2 + ARC_INWARD_NUDGE;
        const cx = side === 'left' ? inset : S - inset;

        const specs = ids.map((id) => getWidgetSpec(id, s, fonts, arc.contentWidth));
        // watch heights, as in the sidebar: each stylized widget centers in
        // the slot the watch would give it
        const heights = ids.map((id) => stackedWidgetHeight(id, s, capabilities));
        const contentH = heights.reduce((a, b) => a + b, 0);

        // spread the stack out, but never past the band it has to fit
        // in — the watch tightens a crowded arc's gaps the same way
        const gapCount = Math.max(0, ids.length - 1);
        let gap = arc.widgetGap;
        if (gapCount > 0 && contentH + gap * gapCount > band) {
            gap = Math.max(ARC_WIDGET_GAP_COMPACT, (band - contentH) / gapCount);
        }

        // the stack centers on the arc's waist
        let y = CY - (contentH + gap * gapCount) / 2;

        return (
            <>
                {accent && (
                    <circle cx={side === 'left' ? arc.width - S : 2 * S - arc.width}
                            cy={CY} r={S} fill={colors.background} />
                )}
                {specs.map((spec, i) => {
                    const top = y + (heights[i] - spec.h) / 2;
                    y += heights[i] + gap;
                    return (
                        <g key={`arc-${side}-${i}`}>
                            {renderItemsStacked(spec, cx, top, colors, fonts, arc.contentWidth)}
                        </g>
                    );
                })}
            </>
        );
    };

    return (
        <div className="halite-watch-preview-container">
            <div className="halite-watch-face halite-watch-face--round">
                <svg
                    xmlns="http://www.w3.org/2000/svg"
                    width={S}
                    height={S}
                    viewBox={`0 0 ${S} ${S}`}
                    fill="none"
                    className="halite-watch-preview"
                >
                    {/* Main circular background */}
                    <circle cx={CX} cy={CY} r={R} fill={bg} />

                    {/* Time display — hours stacked above minutes */}
                    {(() => {
                        const clockH = S - ROUND_VERTICAL_PADDING * 2;
                        const SVG_H = Math.round(clockH * TIME_GLYPH_H_PER_CLOCK_H);
                        const SVG_W = Math.round(SVG_H * TIME_GLYPH_ASPECT);
                        const timeX = (S - SVG_W) / 2;
                        const timeY = (S - SVG_H * 2) / 2;
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

                    {/* Arcs draw over the clock area, as they do on the watch */}
                    {renderArc('left')}
                    {renderArc('right')}

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
        return <RoundPreview effectiveSettings={effectiveSettings} capabilities={capabilities} />;
    }
    return <RectPreview effectiveSettings={effectiveSettings} capabilities={capabilities} />;
};

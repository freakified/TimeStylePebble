import React from 'react';
import { useConfig, useCapabilities } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';

export interface WatchPreviewProps {
    overrideSettings?: Partial<Settings>;
    isNight?: boolean;  // reserved for future night theme support
}

const WIDGET_LABELS: Record<number, string> = {
    0:  '',
    2:  'BATT',
    3:  'ALT',
    4:  'MON\nJAN 1',
    5:  'SEC',
    6:  'WK',
    7:  '72°\nSUNNY',
    8:  '75°/60°',
    9:  'SLEEP',
    10: 'STEPS',
    11: 'BEAT',
    12: 'BPM',
    13: 'UV',
};

/** Render a rectangular (basalt/aplite/diorite/emery) TimeStyle face */
const RectPreview: React.FC<{ effectiveSettings: Settings }> = ({ effectiveSettings }) => {
    const getHex = (color: string) => color ? (color.startsWith('#') ? color : `#${color}`) : '#000000';

    const bg = getHex(effectiveSettings.color_bg);
    const timeColor = getHex(effectiveSettings.color_time);
    const sidebarColor = getHex(effectiveSettings.color_sidebar);
    const textColor = getHex(effectiveSettings.sidebar_text_color);
    const sidebarOnLeft = effectiveSettings.sidebar_position === 'left';

    // Face dimensions
    const W = 144, H = 168;
    const SIDEBAR_W = 30;
    const mainX = sidebarOnLeft ? SIDEBAR_W : 0;
    const sidebarX = sidebarOnLeft ? 0 : W - SIDEBAR_W;
    const mainW = W - SIDEBAR_W;

    // Widget slot Y positions within the sidebar
    const slot0Y = 20;   // top
    const slot1Y = 76;   // middle
    const slot2Y = 132;  // bottom

    const renderWidgetLabel = (widgetId: number, y: number) => {
        const label = WIDGET_LABELS[widgetId] ?? '';
        if (!label) return null;
        const lines = label.split('\n');
        return lines.map((line, i) => (
            <text
                key={i}
                x={sidebarX + SIDEBAR_W / 2}
                y={y + i * 10}
                textAnchor="middle"
                fill={textColor}
                fontSize="7"
                fontFamily="monospace"
                fontWeight="bold"
            >
                {line}
            </text>
        ));
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
                    <rect x={mainX} y={0} width={mainW} height={H} fill={bg} />

                    {/* Sidebar */}
                    <rect x={sidebarX} y={0} width={SIDEBAR_W} height={H} fill={sidebarColor} />

                    {/* Sidebar divider lines */}
                    <line x1={sidebarX} y1={H / 3} x2={sidebarX + SIDEBAR_W} y2={H / 3} stroke={bg} strokeWidth="1" opacity="0.3" />
                    <line x1={sidebarX} y1={(H / 3) * 2} x2={sidebarX + SIDEBAR_W} y2={(H / 3) * 2} stroke={bg} strokeWidth="1" opacity="0.3" />

                    {/* Widget labels */}
                    {renderWidgetLabel(effectiveSettings.widget_0_id, slot0Y)}
                    {renderWidgetLabel(effectiveSettings.widget_1_id, slot1Y)}
                    {renderWidgetLabel(effectiveSettings.widget_2_id, slot2Y)}

                    {/* Time display */}
                    <g transform={`translate(${mainX + mainW / 2}, ${H / 2 - 11})`}>
                        <svg
                            x="-40"
                            y="0"
                            width="80"
                            height="22"
                            viewBox="0 0 80 22"
                            fill={timeColor}
                        >
                            <path d="M0 4.48V0h8.96v17.92h4.48v4.48H0v-4.48h4.48V4.48zM16.493 22.4V0h15.68v22.4zm4.48-17.92v13.44h6.72V4.48zM36.329 22.4v-4.48h4.48v4.48zm0-13.44V4.48h4.48v4.48zM60.658 0v22.4h-15.68v-4.48h11.2v-4.48h-8.96V8.96h8.96V4.48h-11.2V0zM64.814 22.4V0h15.68v22.4zm11.2-8.96h-6.72v4.48h6.72zm-6.72-4.48h6.72V4.48h-6.72z" />
                        </svg>
                    </g>

                    {/* Border */}
                    <rect x={0.5} y={0.5} width={W - 1} height={H - 1} stroke="#333333" strokeWidth="1" fill="none" />
                </svg>
            </div>
        </div>
    );
};

/** Render a round (chalk) TimeStyle face */
const RoundPreview: React.FC<{ effectiveSettings: Settings }> = ({ effectiveSettings }) => {
    const getHex = (color: string) => color ? (color.startsWith('#') ? color : `#${color}`) : '#000000';

    const bg = getHex(effectiveSettings.color_bg);
    const timeColor = getHex(effectiveSettings.color_time);
    const sidebarColor = getHex(effectiveSettings.color_sidebar);
    const textColor = getHex(effectiveSettings.sidebar_text_color);

    const CX = 90, CY = 90, R = 90;
    const PANEL_W = 28, PANEL_H = 80;

    return (
        <div className="halite-watch-preview-container">
            <div className="halite-watch-face">
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

                    {/* Left widget panel */}
                    <rect x={0} y={(180 - PANEL_H) / 2} width={PANEL_W} height={PANEL_H} fill={sidebarColor} />

                    {/* Right widget panel */}
                    <rect x={180 - PANEL_W} y={(180 - PANEL_H) / 2} width={PANEL_W} height={PANEL_H} fill={sidebarColor} />

                    {/* Left widget label */}
                    {WIDGET_LABELS[effectiveSettings.widget_0_id] && (
                        <text
                            x={PANEL_W / 2}
                            y={90}
                            textAnchor="middle"
                            fill={textColor}
                            fontSize="7"
                            fontFamily="monospace"
                            fontWeight="bold"
                        >
                            {WIDGET_LABELS[effectiveSettings.widget_0_id]}
                        </text>
                    )}

                    {/* Right widget label */}
                    {WIDGET_LABELS[effectiveSettings.widget_2_id] && (
                        <text
                            x={180 - PANEL_W / 2}
                            y={90}
                            textAnchor="middle"
                            fill={textColor}
                            fontSize="7"
                            fontFamily="monospace"
                            fontWeight="bold"
                        >
                            {WIDGET_LABELS[effectiveSettings.widget_2_id]}
                        </text>
                    )}

                    {/* Time display */}
                    <g transform={`translate(${CX - 40}, ${CY - 11})`}>
                        <svg width="80" height="22" viewBox="0 0 80 22" fill={timeColor}>
                            <path d="M0 4.48V0h8.96v17.92h4.48v4.48H0v-4.48h4.48V4.48zM16.493 22.4V0h15.68v22.4zm4.48-17.92v13.44h6.72V4.48zM36.329 22.4v-4.48h4.48v4.48zm0-13.44V4.48h4.48v4.48zM60.658 0v22.4h-15.68v-4.48h11.2v-4.48h-8.96V8.96h8.96V4.48h-11.2V0zM64.814 22.4V0h15.68v22.4zm11.2-8.96h-6.72v4.48h6.72zm-6.72-4.48h6.72V4.48h-6.72z" />
                        </svg>
                    </g>

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
    return <RectPreview effectiveSettings={effectiveSettings} />;
};

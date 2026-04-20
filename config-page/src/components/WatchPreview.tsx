import React from 'react';
import { useConfig, useCapabilities } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';

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

    const bg = getHex(effectiveSettings.colorBackground);
    const timeColor = getHex(effectiveSettings.colorTime);
    const sidebarColor = getHex(effectiveSettings.colorSidebar);
    const textColor = getHex(effectiveSettings.colorSidebarText);
    const sidebarOnLeft = effectiveSettings.sidebarPosition === 'left';

    // Face dimensions
    const W = 144, H = 168;
    const SIDEBAR_W = 30;
    const mainX = sidebarOnLeft ? SIDEBAR_W : 0;
    const sidebarX = sidebarOnLeft ? 0 : W - SIDEBAR_W;
    const mainW = W - SIDEBAR_W;

    const { hourPath, minutePath } = getTimePaths(effectiveSettings.clockFont);

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
                    {renderWidgetLabel(effectiveSettings.widget0Id, slot0Y)}
                    {renderWidgetLabel(effectiveSettings.widget1Id, slot1Y)}
                    {renderWidgetLabel(effectiveSettings.widget2Id, slot2Y)}

                    {/* Time display — hours stacked above minutes */}
                    {(() => {
                        const SVG_W = 108;
                        const SVG_H = Math.round(SVG_W * 81 / 109);
                        const timeX = mainX + (mainW - SVG_W) / 2;
                        const timeY = (H - SVG_H * 2) / 2;
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

    const bg = getHex(effectiveSettings.colorBackground);
    const timeColor = getHex(effectiveSettings.colorTime);
    const sidebarColor = getHex(effectiveSettings.colorSidebar);
    const textColor = getHex(effectiveSettings.colorSidebarText);

    const { hourPath, minutePath } = getTimePaths(effectiveSettings.clockFont);

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
                    <circle cx={-50} cy={CY} r={R} fill={sidebarColor} />
                    <circle cx={230} cy={CY} r={R} fill={sidebarColor} />

                    {/* Left widget label */}
                    {WIDGET_LABELS[effectiveSettings.widget0Id] && (
                        <text
                            x={20}
                            y={90}
                            textAnchor="middle"
                            fill={textColor}
                            fontSize="7"
                            fontFamily="monospace"
                            fontWeight="bold"
                        >
                            {WIDGET_LABELS[effectiveSettings.widget0Id]}
                        </text>
                    )}

                    {/* Right widget label */}
                    {WIDGET_LABELS[effectiveSettings.widget2Id] && (
                        <text
                            x={160}
                            y={90}
                            textAnchor="middle"
                            fill={textColor}
                            fontSize="7"
                            fontFamily="monospace"
                            fontWeight="bold"
                        >
                            {WIDGET_LABELS[effectiveSettings.widget2Id]}
                        </text>
                    )}

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
    return <RectPreview effectiveSettings={effectiveSettings} />;
};

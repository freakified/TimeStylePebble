import React from 'react';
import { useConfig, useCapabilities } from '../context/PebbleConfigContext';
import { Capabilities, Settings } from '../context/types';
import { darkRegionPalette, RegionPalette, sidebarPalette } from '../utils/colorDerivation';
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
        lecoLight:  'M8.62 13.7V7H32.07V67.2H48.82V74H8.62V67.2H25.37V13.7ZM56.48 74V7H103.38V74ZM63.18 13.7V67.3H96.68V13.7Z',
        garamond:   'M10.48 75.02Q9.75 75.02 9.26 74.59Q8.76 74.16 8.76 73.64Q8.76 72.41 10.51 71.95Q14.23 70.97 16.7 70.11Q19.17 69.26 20.4 67.71Q21.62 66.16 21.62 63.32V22.85Q21.62 20.62 20.31 19.44Q19 18.27 16.94 17.78Q14.87 17.28 12.5 17.08Q11.48 16.94 10.59 16.48Q9.69 16.02 9.69 15.03Q9.69 14.05 10.57 13.48Q11.44 12.91 12.61 12.8Q18 12.14 22.26 10.87Q26.53 9.6 30.67 6.21Q30.81 6.1 31.02 6.05Q31.23 6.01 31.43 6.01Q32.06 6.01 32.59 6.29Q33.11 6.58 33.04 7.04Q32.88 8.57 32.71 10.42Q32.54 12.28 32.54 14.76V64.2Q32.54 66.68 33.68 68.04Q34.81 69.39 37.25 70.2Q39.68 71.01 43.63 72.03Q44.38 72.2 44.95 72.59Q45.53 72.98 45.53 73.68Q45.53 74.99 43.96 74.99Q41.16 74.99 38.31 74.85Q35.46 74.7 32.72 74.59Q29.98 74.48 27.28 74.48Q24.59 74.48 21.77 74.59Q18.94 74.7 16.09 74.86Q13.23 75.02 10.48 75.02ZM79.2 76.21Q73.36 76.21 69.06 73.19Q64.76 70.17 61.91 65.15Q59.05 60.14 57.68 54.09Q56.31 48.05 56.31 42.12Q56.31 35.97 57.68 29.71Q59.06 23.45 61.97 18.26Q64.87 13.07 69.46 9.88Q74.04 6.7 80.34 6.7Q86.19 6.7 90.49 9.72Q94.78 12.74 97.62 17.76Q100.46 22.78 101.85 28.76Q103.24 34.75 103.24 40.58Q103.24 46.65 101.87 52.91Q100.5 59.16 97.59 64.45Q94.68 69.74 90.15 72.97Q85.62 76.21 79.2 76.21ZM80.97 72.69Q84.98 72.69 87.43 68.43Q89.89 64.17 91 57.3Q92.11 50.44 92.11 42.52Q92.11 36.6 91.25 30.89Q90.4 25.17 88.65 20.51Q86.89 15.85 84.34 13.1Q81.79 10.35 78.39 10.35Q74.39 10.35 72.02 14.51Q69.66 18.67 68.6 25.48Q67.55 32.29 67.55 40.21Q67.55 46.12 68.39 51.92Q69.23 57.71 70.9 62.4Q72.57 67.09 75.07 69.89Q77.57 72.69 80.97 72.69Z',
        garamondBold: 'M7.58 75.02Q6.92 75.02 6.31 74.59Q5.7 74.16 5.7 73.46Q5.7 71.79 7.67 71.27Q10.71 70.45 12.86 69.72Q15.02 68.98 16.21 67.66Q17.41 66.34 17.41 64.01V24.95Q17.41 22.38 16.07 21.03Q14.73 19.69 12.64 19.08Q10.55 18.47 8.18 18.1Q7.27 17.9 6.32 17.38Q5.37 16.87 5.37 15.65Q5.37 14.28 6.36 13.65Q7.34 13.02 8.69 12.91Q15.04 12.42 20.1 10.89Q25.17 9.36 30.16 6.5Q30.36 6.39 30.65 6.25Q30.94 6.12 31.32 6.12Q32.12 6.12 32.82 6.44Q33.51 6.75 33.49 7.27Q33.46 8.62 33.17 10.42Q32.89 12.22 32.89 15.1V64.16Q32.89 66.23 34.22 67.5Q35.55 68.77 37.9 69.58Q40.25 70.39 43.34 71.29Q44.25 71.57 44.8 71.99Q45.35 72.41 45.35 73.4Q45.35 74.93 43.66 74.93Q41.26 74.93 37.85 74.82Q34.43 74.7 31.12 74.59Q27.81 74.48 25.52 74.48Q23.23 74.48 19.98 74.59Q16.73 74.7 13.39 74.86Q10.05 75.02 7.58 75.02ZM80.1 76.21Q73.64 76.21 68.89 73.19Q64.13 70.17 60.99 65.16Q57.85 60.15 56.34 54.11Q54.83 48.07 54.83 42.15Q54.83 36 56.34 29.75Q57.86 23.5 61.05 18.28Q64.24 13.06 69.31 9.88Q74.38 6.7 81.35 6.7Q87.82 6.7 92.57 9.72Q97.32 12.74 100.42 17.75Q103.51 22.76 105.07 28.74Q106.63 34.73 106.63 40.55Q106.63 46.67 105.12 52.93Q103.62 59.18 100.42 64.46Q97.22 69.74 92.21 72.98Q87.2 76.21 80.1 76.21ZM81.76 71.37Q85.18 71.37 87.3 67.32Q89.41 63.26 90.37 56.67Q91.34 50.08 91.34 42.48Q91.34 36.81 90.6 31.29Q89.86 25.78 88.36 21.29Q86.86 16.8 84.67 14.14Q82.49 11.49 79.57 11.49Q76.14 11.49 74.1 15.56Q72.05 19.63 71.14 26.17Q70.23 32.7 70.23 40.3Q70.23 45.98 70.92 51.51Q71.62 57.04 73.09 61.53Q74.56 66.01 76.7 68.69Q78.84 71.37 81.76 71.37Z',
    },
    minutes: {
        avenir:     'M48.488 74.696H5v-7.2l27.84-28.128a49 49 0 0 0 2.976-3.264 29 29 0 0 0 2.688-3.744 19 19 0 0 0 1.824-4.032Q41 26.12 41 23.816q0-2.976-1.056-5.376t-2.976-4.032q-1.824-1.728-4.32-2.592-2.4-.96-5.088-.96-5.472 0-9.216 3.456-3.744 3.36-4.8 8.64l-6.816-1.344a20.2 20.2 0 0 1 2.496-6.624q1.92-3.072 4.608-5.28a22 22 0 0 1 6.24-3.456Q23.528 5 27.56 5q4.128 0 7.776 1.248Q39.08 7.4 41.864 9.8q2.784 2.304 4.416 5.856 1.728 3.456 1.728 8.064 0 3.264-.96 6.048a23 23 0 0 1-2.4 5.28 30.2 30.2 0 0 1-3.456 4.8 135 135 0 0 1-4.224 4.416l-24.48 24.288h36zM76.336 36.104h2.976q3.264 0 6.144-.672t4.992-2.112a10.8 10.8 0 0 0 3.456-3.936q1.248-2.4 1.248-5.76 0-2.88-1.056-5.184-1.056-2.4-2.976-4.032-1.824-1.728-4.32-2.592-2.4-.96-5.088-.96-4.32 0-7.68 2.016-3.264 2.016-5.28 5.952l-5.856-3.072q3.168-5.472 8.064-8.064Q75.856 5 82.096 5q4.032 0 7.584 1.248 3.648 1.152 6.336 3.456 2.784 2.304 4.32 5.76 1.632 3.36 1.632 7.776 0 6.048-3.36 10.176-3.36 4.032-8.448 5.376v.192q5.856 1.248 9.792 5.664 3.936 4.32 3.936 11.232 0 4.896-1.824 8.736-1.728 3.744-4.704 6.432-2.976 2.592-7.008 3.936-3.936 1.44-8.352 1.44-7.488 0-13.056-3.552-5.568-3.648-8.448-10.368l6.528-2.4q1.728 4.896 5.76 7.68 4.032 2.688 9.216 2.688 2.88 0 5.568-.96 2.784-1.056 4.8-2.88 2.112-1.92 3.36-4.608t1.248-6.144q0-3.84-1.536-6.432-1.44-2.688-3.936-4.32t-5.664-2.304a27.6 27.6 0 0 0-6.528-.768h-2.976z',
        avenirBold: 'm4.48 64.136 26.208-25.728q3.168-3.072 4.992-6.144 1.92-3.168 1.92-7.2 0-4.8-3.168-7.584-3.072-2.88-7.68-2.88-4.896 0-7.872 3.36-2.976 3.264-3.648 8.256L4 24.488q.48-4.224 2.4-7.776a21.9 21.9 0 0 1 4.896-6.144q2.976-2.592 6.912-4.032Q22.24 5 26.944 5q4.416 0 8.448 1.248 4.128 1.248 7.296 3.84 3.168 2.496 4.992 6.24 1.92 3.648 1.92 8.544 0 3.264-.864 6.144a23 23 0 0 1-2.4 5.28 30.2 30.2 0 0 1-3.456 4.8 54 54 0 0 1-4.224 4.32l-19.584 18.72h30.624v10.56H4.48zM73.922 34.664h3.072q2.592 0 5.088-.384 2.592-.48 4.704-1.632t3.36-3.072q1.344-2.016 1.344-5.184 0-2.304-.864-4.128a8.7 8.7 0 0 0-2.4-3.072q-1.44-1.248-3.456-1.92a12.1 12.1 0 0 0-4.032-.672q-4.224 0-7.008 2.4t-3.84 6.336l-10.848-2.784q.864-3.456 2.88-6.24a21.3 21.3 0 0 1 4.992-4.896q2.976-2.112 6.624-3.264T81.218 5q4.416 0 8.352 1.248Q93.602 7.4 96.578 9.8a16.1 16.1 0 0 1 4.8 5.76q1.728 3.456 1.728 8.064 0 5.472-3.36 9.792-3.264 4.224-8.832 5.568v.192q6.24 1.248 10.08 5.664 3.936 4.32 3.936 10.656 0 5.28-2.112 9.216-2.016 3.936-5.472 6.528-3.36 2.592-7.584 3.84a28 28 0 0 1-8.544 1.344q-4.32 0-8.16-.96-3.84-.864-7.104-2.784-3.168-1.92-5.664-4.992-2.4-3.072-3.744-7.296l10.752-3.36q1.248 4.032 4.608 6.816 3.456 2.784 8.928 2.784 2.208 0 4.416-.576 2.304-.672 4.032-2.016 1.824-1.344 2.88-3.456 1.152-2.112 1.152-5.184 0-3.264-1.632-5.472-1.536-2.304-3.936-3.552-2.4-1.344-5.376-1.92a28.4 28.4 0 0 0-5.568-.576h-2.88z',
        leco:       'M5 27.1V7h47v40.2H18.429v13.4H52V74H5V33.8h33.571V20.4H18.43v6.7zM104 7v67H57V60.6h33.571V47.2H63.714V33.8h26.857V20.4H57V7z',
        lecoLight:  'M6.71 20.4V7H53.61V43.85H13.41V67.2H53.61V74H6.71V37.15H46.91V13.7H13.41V20.4ZM105.29 7V74H58.39V67.3H98.59V43.85H65.09V37.15H98.59V13.7H58.39V7Z',
        garamond:   'M49.05 75.05Q48.33 75.05 46.39 74.96Q44.46 74.87 41.93 74.76Q39.39 74.65 36.71 74.56Q34.03 74.47 31.76 74.47Q29.53 74.47 26.05 74.56Q22.57 74.65 18.95 74.71Q15.32 74.76 12.67 74.85Q10.01 74.95 9.33 74.95Q8.96 74.95 8.07 74.29Q7.17 73.64 7.17 73.11Q7.17 72.45 7.48 72.12Q7.79 71.8 8.22 71.37L25.7 52.83Q31.19 47 34.48 40.28Q37.78 33.56 37.78 26.31Q37.78 21 34.73 17.79Q31.69 14.57 27.21 14.57Q23 14.57 19.38 16.91Q15.76 19.24 13.46 22.66Q13.05 23.3 12.41 23.86Q11.76 24.42 11.04 24.42Q10.48 24.42 9.98 23.58Q9.48 22.74 9.48 21.43Q9.48 19.98 11.12 17.57Q12.77 15.17 15.7 12.68Q18.63 10.2 22.6 8.47Q26.58 6.73 31.18 6.73Q36.63 6.73 40.5 9.08Q44.37 11.42 46.42 15.44Q48.47 19.47 48.47 24.63Q48.47 27.57 47.33 31.01Q46.2 34.45 44.36 37.7Q42.51 40.95 40.31 43.6Q38.85 45.36 36.2 48.06Q33.56 50.76 30.58 53.73Q27.6 56.7 24.93 59.33Q22.26 61.95 20.57 63.67Q18.87 65.39 18.87 65.6Q18.87 66.11 20.42 66.32Q22.31 66.46 24.77 66.57Q27.23 66.68 30 66.68Q32.88 66.68 35.76 66.57Q38.64 66.46 41.33 66.32Q44.21 66.08 45.95 65.63Q47.7 65.18 48.95 64.03Q50.19 62.88 51.4 60.72Q51.8 60.04 52.54 59.17Q53.29 58.3 54.22 58.3Q54.85 58.3 55.04 58.83Q55.22 59.35 55.13 59.91Q54.79 61.23 54.04 63.41Q53.29 65.58 52.53 67.99Q51.78 70.39 51.39 72.53Q51.16 74.07 50.33 74.56Q49.5 75.05 49.05 75.05ZM74.08 78.8Q71.63 78.8 68.92 78.02Q66.21 77.24 64.35 76.12Q62.48 74.99 62.48 73.94Q62.48 72.97 63.21 71.66Q63.93 70.34 64.97 69.38Q66.01 68.42 66.89 68.42Q67.98 68.42 68.84 68.98Q69.7 69.55 70.66 70.39Q72.09 71.76 74 73.14Q75.91 74.51 78.78 74.51Q82.58 74.51 85.88 72.53Q89.17 70.54 91.27 66.97Q93.37 63.4 93.37 58.85Q93.37 54.58 92.03 50.86Q90.69 47.13 88.08 44.8Q85.47 42.47 81.55 42.47Q79.83 42.47 77.94 42.97Q76.06 43.47 74.53 44.33Q73.9 44.69 73.07 44.26Q72.25 43.84 72.39 43.18Q72.85 41.52 73.87 40.43Q74.89 39.34 76.09 38.82Q77.85 37.95 80.08 36.57Q82.32 35.19 84.46 33.31Q86.61 31.44 88.01 29.09Q89.41 26.74 89.41 23.89Q89.41 19.81 86.72 16.73Q84.03 13.64 79.86 13.64Q76.03 13.64 73.6 15.05Q71.17 16.46 69.66 18.29Q68.15 20.11 67.34 21.52Q66.52 22.93 65.9 22.93Q65.38 22.93 64.68 22.21Q63.99 21.48 63.99 20.66Q63.99 19.42 65.17 17.13Q66.36 14.84 68.91 12.45Q71.46 10.06 75.46 8.41Q79.46 6.76 85.12 6.76Q88.93 6.76 92.26 8.34Q95.58 9.92 97.68 12.74Q99.77 15.55 99.77 19.34Q99.77 21.87 98.55 24.8Q97.33 27.73 94.59 30.48Q91.84 33.23 87.29 35.28Q86.96 35.38 86.96 35.65Q86.96 35.92 87.4 35.92Q92.38 35.92 96.28 38.15Q100.19 40.37 102.51 44.17Q104.83 47.97 104.83 52.83Q104.83 58.84 102.21 63.63Q99.59 68.41 95.19 71.81Q90.8 75.21 85.32 77Q79.84 78.8 74.08 78.8Z',
        garamondBold: 'M47.45 75.11Q46.78 75.11 44.28 75.05Q41.78 74.98 38.36 74.88Q34.94 74.77 31.41 74.71Q27.87 74.64 25.2 74.64Q22.62 74.64 19.69 74.71Q16.76 74.77 14.11 74.82Q11.46 74.88 9.66 74.94Q7.86 75 7.53 75Q7.28 75 6.33 74.44Q5.37 73.87 5.37 73.17Q5.37 72.28 5.86 71.75Q6.34 71.23 6.77 70.8L22.46 53.31Q28.12 47.08 30.92 40.79Q33.73 34.5 33.73 27.04Q33.73 22.47 31.43 19.41Q29.13 16.34 24.93 16.34Q21.13 16.34 17.8 18.5Q14.47 20.66 12.12 23.21Q11.53 23.86 10.6 24.65Q9.68 25.43 8.61 25.43Q7.82 25.43 7.49 24.74Q7.17 24.04 7.17 22.9Q7.17 21.17 8.95 18.51Q10.73 15.85 13.89 13.16Q17.05 10.48 21.19 8.64Q25.34 6.79 30.04 6.79Q35.61 6.79 39.45 9.27Q43.28 11.76 45.33 15.83Q47.38 19.91 47.38 24.84Q47.38 27.84 46.05 31.45Q44.71 35.05 42.76 38.33Q40.8 41.61 38.71 43.97Q37.14 45.78 34.7 48.2Q32.26 50.61 29.65 53.09Q27.05 55.58 24.72 57.77Q22.39 59.96 20.96 61.36Q19.52 62.76 19.52 62.98Q19.52 63.43 21.12 63.65Q23.13 63.84 25.85 63.95Q28.56 64.06 30.87 64.06Q33.29 64.06 35.71 63.95Q38.13 63.84 40.42 63.65Q42.84 63.34 44.61 62.67Q46.38 61.99 47.74 60.61Q49.1 59.23 50.13 57.08Q50.47 56.34 51.31 55.56Q52.14 54.77 53.18 54.77Q53.98 54.77 54.31 55.47Q54.64 56.17 54.38 56.95Q53.4 59.7 52.37 62.99Q51.33 66.27 50.58 68.94Q49.83 71.6 49.73 72.7Q49.67 73.9 48.69 74.5Q47.72 75.11 47.45 75.11ZM74.54 78.8Q72.15 78.8 69.61 78.08Q67.07 77.36 65.35 76.15Q63.63 74.93 63.63 73.54Q63.63 72.17 64.1 70.63Q64.56 69.09 65.43 68.04Q66.29 66.99 67.4 66.99Q68.32 66.99 68.98 67.42Q69.64 67.84 70.37 68.62Q71.86 70.33 73.91 72.25Q75.97 74.17 78.54 74.17Q82 74.17 84.81 72.3Q87.61 70.43 89.31 66.95Q91.01 63.47 91.01 58.7Q91.01 54.32 89.79 50.88Q88.56 47.44 86.21 45.48Q83.86 43.52 80.45 43.52Q77.76 43.52 76.08 44.45Q74.39 45.39 73.44 45.85Q72.64 46.26 71.59 45.75Q70.53 45.24 70.73 44.34Q71.25 42.24 72.55 40.83Q73.86 39.43 75.29 38.73Q78.47 37.07 80.84 35.43Q83.22 33.79 84.85 31.95Q86.48 30.11 87.28 27.9Q88.07 25.7 88.07 22.96Q88.07 19.52 85.7 17.01Q83.33 14.49 79.62 14.49Q76.48 14.49 74.25 15.82Q72.03 17.14 70.52 18.85Q69.01 20.56 68.08 21.88Q67.15 23.2 66.59 23.2Q65.89 23.2 65.11 22.34Q64.33 21.47 64.33 20.54Q64.33 19.58 65.52 17.35Q66.71 15.12 69.28 12.7Q71.86 10.29 75.97 8.58Q80.08 6.88 86.02 6.88Q90.29 6.88 93.87 8.48Q97.44 10.09 99.62 13.02Q101.8 15.94 101.8 19.96Q101.8 22.37 100.33 25.53Q98.85 28.68 95.62 31.43Q92.39 34.18 87.22 35.42Q86.89 35.53 86.89 35.8Q86.89 36.07 87.32 36.07Q92.93 36.07 97.26 38.12Q101.59 40.18 104.11 43.91Q106.63 47.65 106.63 52.91Q106.63 58.85 103.95 63.61Q101.27 68.36 96.74 71.76Q92.21 75.15 86.48 76.97Q80.74 78.8 74.54 78.8Z',
    },
} as const;

// the watch's [typeface][bold] face table, by TIME_PATHS key
const TIME_FACES = {
    avenir:   { light: 'avenir',    bold: 'avenirBold' },
    leco:     { light: 'lecoLight', bold: 'leco' },
    garamond: { light: 'garamond',  bold: 'garamondBold' },
} as const;

function getTimePaths(s: Pick<Settings, 'clockFont' | 'boldHours' | 'boldMinutes'>) {
    const faces = TIME_FACES[s.clockFont] ?? TIME_FACES.avenir;
    return {
        hourPath: TIME_PATHS.hours[s.boldHours ? faces.bold : faces.light],
        minutePath: TIME_PATHS.minutes[s.boldMinutes ? faces.bold : faces.light],
    };
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

interface RegionColors {
    background: string;  // region fill
    text: string;        // widget text color
    iconFill: string;
    iconStroke: string;
}

// region appearance, derived from the theme's three colors the same way
// the watch does (see utils/colorDerivation.ts)
const toRegionColors = (p: RegionPalette): RegionColors => ({
    background: getHex(p.background),
    text: getHex(p.text),
    iconFill: getHex(p.iconFill),
    iconStroke: getHex(p.iconStroke),
});

// "standard" regions wear the sidebar color
const standardRegionColors = (s: Settings) => toRegionColors(sidebarPalette(s));

// "dark" regions (the bottom bar) draw on the watchface background
const darkRegionColors = (s: Settings) => toRegionColors(darkRegionPalette(s));

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

// The day number sits inside the calendar body (below the binding rings),
// in the sidebar's current font like date_content_draw() draws it, scaled
// down a touch so it sits comfortably inside the preview's icon.
const CALENDAR_DAY_CENTER_Y = 14;
const CALENDAR_DAY_SCALE = 0.75;

// draw_date_plain_background(): large fonts swap the calendar outline for
// a plain box, an iconStroke border around an iconFill body
const DATE_BOX_BORDER = 2;
const DATE_BOX_RADIUS = 2;

const renderCalendar = (
    key: React.Key,
    day: string,
    x: number,
    y: number,
    colors: RegionColors,
    fonts: PreviewFonts
) => {
    const { w, h } = ICONS.date;
    // previewFonts() only promotes the large size to "current" in large-font mode
    const plain = fonts.current === fonts.lg;
    const fitted = fitText(day, 'current', fonts, w);
    const fontSize = fitted.fontSize * CALENDAR_DAY_SCALE;
    const centerY = plain ? h / 2 : CALENDAR_DAY_CENTER_Y;
    return (
        <g key={key}>
            {plain ? (
                <>
                    <rect x={x} y={y} width={w} height={h}
                          rx={DATE_BOX_RADIUS} fill={colors.iconStroke} />
                    <rect x={x + DATE_BOX_BORDER} y={y + DATE_BOX_BORDER}
                          width={w - DATE_BOX_BORDER * 2} height={h - DATE_BOX_BORDER * 2}
                          fill={colors.iconFill} />
                </>
            ) : (
                renderIconImage('bg', ICONS.date, x, y, w, h, colors)
            )}
            {textNode('day', day, x + w / 2,
                      y + centerY - (fontSize * PREVIEW_CAP_HEIGHT) / 2,
                      fontSize, colors.iconStroke)}
        </g>
    );
};

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
                return renderCalendar(i, item.day, cx - size.w / 2, itemTop, colors, fonts);
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
            nodes.push(renderCalendar('icon', parts.icon.calendar, cursor, top, colors, fonts));
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

    const { hourPath, minutePath } = getTimePaths(s);

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

    const { hourPath, minutePath } = getTimePaths(s);

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

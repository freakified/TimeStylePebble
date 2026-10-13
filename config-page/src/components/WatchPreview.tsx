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
    WATCH_BAR_PADDING_X,
    WATCH_BAR_PADDING_X_COMPACT,
    WATCH_BAR_VBAR_GAP,
    WATCH_BAR_WIDGET_GAP_COMPACT,
} from '../utils/widgetMetrics';

export interface WatchPreviewProps {
    overrideSettings?: Partial<Settings>;
}

// The preview shows 9:23 (09:23 with a leading zero), generated from the
// watch's own .ffont glyphs: 4/3 scale, baseline at y=74, each string
// centered on its advances at x=54.5 like GTextAlignmentCenter. A lone
// hour is " 9", as %k/%l space-pad it; the space is digit-wide in every
// clock font, so the hour lines up right with the minutes.
const TIME_PATHS = {
    hours: {
        avenir:       'M91.08 44.33L90.92 44.17Q89.33 45.33 86.58 46T81.17 46.67Q76.75 46.67 73 45.08T66.42 40.83T62 34.17T60.42 25.75Q60.42 21 62.08 17T66.67 10.25T73.67 5.92T82.5 4.33T91.33 5.92T98.33 10.25T103 17T104.58 25.75Q104.58 31 102.75 35.5T98.5 44.17L79.92 74H72ZM97.67 25.75Q97.67 22.42 96.58 19.58T93.5 14.58T88.67 11.33T82.5 10.17T76.42 11.33T71.58 14.58T68.5 19.58T67.33 25.75T68.42 31.83T71.42 36.67T76.25 39.92T82.5 41.08T88.83 39.92T93.58 36.67T96.67 31.83T97.67 25.75Z',
        avenirBold:   'M70.67 74L88.08 48.75L88 48.67Q86.58 49.33 84.5 49.58T80.67 49.92Q76.17 49.92 72.25 48.25T65.33 43.58T60.67 36.67T59 28.08Q59 22.75 60.92 18.33T66.17 10.75T74 6T83.75 4.33Q89.08 4.33 93.58 6.08T101.5 11T106.75 18.5T108.67 27.67Q108.67 30.5 108.17 33.08T106.75 37.92T104.58 42.67T101.67 47.67L84.67 74H70.67ZM96.92 27.33Q96.92 24.67 96 22.25T93.42 18T89.25 15T83.92 13.92Q78 13.92 74.42 17.67T70.83 27.5Q70.83 30.42 71.75 32.83T74.33 37T78.42 39.75T83.67 40.75T89 39.83T93.25 37.08T96 32.83T96.92 27.33Z',
        leco:         'M108.25 6.83V74H61.25V60.58H94.83V47.08H61.25V6.83H108.25ZM74.67 20.25V33.67H94.83V20.25H74.67Z',
        lecoLight:    'M108.25 6.83V74H61.25V67.25H101.58V43.75H61.25V6.83ZM101.58 37.08V13.58H68V37.08Z',
        garamond:     'M61.75 77.33Q60.83 77.33 60.58 77.08Q60.25 76.83 60.25 76.5Q60.25 75.92 60.67 75.67Q61.17 75.42 61.75 75.42Q65 75.42 69.25 73.33Q73.5 71.17 77.83 67.58Q82.25 64 85.67 59.5Q89.17 55 91.25 50.67Q93.33 46.33 94.33 41.75Q95.33 37.17 95.33 32Q95.33 28.17 94.08 24.25Q92.83 20.33 90.75 17.08Q88.58 13.92 85.83 11.92Q83.08 10 80.17 10Q76.75 10 74.33 11.92Q72 13.75 70.83 16.5Q69.58 19.25 69.58 22Q69.58 25.25 71.17 28.33Q72.75 31.5 75.33 33.5Q77.92 35.5 80.75 35.5Q84.08 35.5 85.67 34.58Q87.33 33.67 89.33 32.67Q89.83 32.42 90.08 33Q90.33 33.5 90.08 33.83Q89.58 34.92 88.5 35.92Q87.42 37 86.58 37.67Q85.83 38.17 84.17 39Q82.58 39.92 80.58 40.5Q78.67 41.17 76.67 41.17Q72.92 41.17 69.42 39.42Q65.92 37.67 63.75 34.58Q61.5 31.5 61.5 27.58Q61.5 23.92 63.17 20.17Q64.75 16.33 67.75 13.17Q70.67 10 74.83 8.08Q78.92 6.08 83.75 6.08Q87.67 6.08 91.33 8.25Q95.08 10.33 98.08 13.83Q101 17.33 102.75 21.75Q104.5 26.17 104.5 30.83Q104.5 37.5 102.67 43.17Q100.83 48.83 97.33 54Q93.92 59.25 88.92 64.58Q86 67.83 82.42 70.25Q78.92 72.58 75.25 74.25Q71.58 75.83 68.08 76.58Q64.5 77.33 61.75 77.33Z',
        garamondBold: 'M63.25 79.33Q61.67 79.33 61 78.83Q60.42 78.25 60.42 77.75Q60.42 76.5 61.33 75.83Q62.33 75.25 63.42 75.17Q67.33 74.67 71.42 72.33Q75.42 70.08 79.08 66.58Q82.83 63.17 85.67 59.58Q87.83 56.58 90.25 52.5Q92.67 48.33 94.42 43.17Q96.17 38 96.17 32.08Q96.17 28.42 95.17 24.83Q94.25 21.25 92.67 18.25Q91 15.25 89 13.5Q86.92 11.67 84.75 11.67Q82.17 11.67 80.33 13Q78.5 14.42 77.58 17Q76.67 19.67 76.67 23.75Q76.67 26.92 77.83 30Q79 33 80.83 35Q82.67 36.92 84.75 36.92Q86.33 36.92 87.42 36.58Q88.42 36.25 89.42 35.83Q91.67 34.92 92 36.08Q92.33 37.17 91.25 38.5Q90.75 39.33 90.17 39.92Q89.5 40.5 88.92 41Q88.17 41.75 86.42 42.75Q84.75 43.75 82.67 44.5Q80.67 45.17 78.75 45.17Q74.67 45.17 70.83 43.08Q66.92 41 64.33 37.42Q61.75 33.75 61.75 29.08Q61.75 25.58 63.75 21.58Q65.83 17.58 69.33 14.08Q72.92 10.58 77.5 8.42Q82.08 6.17 87.25 6.17Q91.92 6.17 96.08 8.25Q100.25 10.25 103.42 13.67Q106.58 17.17 108.42 21.58Q110.25 26.08 110.25 30.92Q110.25 37.67 107.83 43.75Q105.33 49.75 101.67 54.75Q98 59.75 94.25 63.42Q91.75 65.83 87.75 68.58Q83.83 71.33 79.33 73.83Q74.92 76.25 70.67 77.83Q66.42 79.33 63.25 79.33Z',
    },
    hoursLeadingZero: {
        avenir:       'M28.58 4.33Q34.92 4.33 39.17 7.42T46.08 15.58T49.75 27.08T50.83 40.08T49.75 53.08T46.08 64.58T39.17 72.67T28.58 75.75Q22.17 75.75 17.92 72.67T11 64.58T7.33 53.08T6.33 40.08T7.33 27.08T11 15.58T17.92 7.42T28.58 4.33ZM28.58 10.17Q24.08 10.17 21.08 12.92T16.42 20.08T14 29.67T13.33 40.08T14 50.5T16.42 60T21.08 67T28.58 69.75Q33 69.75 36 67T40.67 60T43.17 50.5T43.83 40.08T43.17 29.67T40.67 20.08T36 12.92T28.58 10.17ZM91.08 44.33L90.92 44.17Q89.33 45.33 86.58 46T81.17 46.67Q76.75 46.67 73 45.08T66.42 40.83T62 34.17T60.42 25.75Q60.42 21 62.08 17T66.67 10.25T73.67 5.92T82.5 4.33T91.33 5.92T98.33 10.25T103 17T104.58 25.75Q104.58 31 102.75 35.5T98.5 44.17L79.92 74H72ZM97.67 25.75Q97.67 22.42 96.58 19.58T93.5 14.58T88.67 11.33T82.5 10.17T76.42 11.33T71.58 14.58T68.5 19.58T67.33 25.75T68.42 31.83T71.42 36.67T76.25 39.92T82.5 41.08T88.83 39.92T93.58 36.67T96.67 31.83T97.67 25.75Z',
        avenirBold:   'M54 40Q54 48.25 52.42 54.92T47.83 66.17T40.33 73.25T30.17 75.75T19.83 73.25T12.25 66.17T7.58 54.92T6 40T7.58 25.17T12.25 13.92T19.83 6.83T30.17 4.33T40.33 6.83T47.83 13.92T52.42 25.17T54 40ZM42.25 40Q42.25 36.5 41.75 31.92T40.08 23.42T36.33 16.67T30.17 13.92Q26.17 13.92 23.75 16.67T20.08 23.42T18.33 31.92T17.83 40T18.33 48.08T20.08 56.5T23.75 63.17T30.17 65.92Q34 65.92 36.33 63.17T40.08 56.5T41.75 48.08T42.25 40ZM70.67 74L88.08 48.75L88 48.67Q86.58 49.33 84.5 49.58T80.67 49.92Q76.17 49.92 72.25 48.25T65.33 43.58T60.67 36.67T59 28.08Q59 22.75 60.92 18.33T66.17 10.75T74 6T83.75 4.33Q89.08 4.33 93.58 6.08T101.5 11T106.75 18.5T108.67 27.67Q108.67 30.5 108.17 33.08T106.75 37.92T104.58 42.67T101.67 47.67L84.67 74H70.67ZM96.92 27.33Q96.92 24.67 96 22.25T93.42 18T89.25 15T83.92 13.92Q78 13.92 74.42 17.67T70.83 27.5Q70.83 30.42 71.75 32.83T74.33 37T78.42 39.75T83.67 40.75T89 39.83T93.25 37.08T96 32.83T96.92 27.33Z',
        leco:         'M9.42 74V6.83H56.42V74H9.42ZM22.83 20.25V60.58H43V20.25H22.83ZM108.25 6.83V74H61.25V60.58H94.83V47.08H61.25V6.83H108.25ZM74.67 20.25V33.67H94.83V20.25H74.67Z',
        lecoLight:    'M9.42 74V6.83H56.42V74ZM16.17 13.58V67.25H49.75V13.58ZM108.25 6.83V74H61.25V67.25H101.58V43.75H61.25V6.83ZM101.58 37.08V13.58H68V37.08Z',
        garamond:     'M29.08 75.5Q23.58 75.5 19.58 72.5Q15.5 69.5 12.83 64.42Q10.08 59.42 8.83 53.42Q7.5 47.33 7.5 41.42Q7.5 35.25 8.83 29Q10.08 22.75 12.83 17.58Q15.58 12.33 19.92 9.17Q24.25 6 30.17 6Q35.67 6 39.75 9Q43.75 12.08 46.5 17.08Q49.17 22.08 50.5 28.08Q51.75 34.08 51.75 39.92Q51.75 45.92 50.5 52.17Q49.17 58.42 46.42 63.75Q43.67 69 39.42 72.25Q35.17 75.5 29.08 75.5ZM30.92 72.67Q35.25 72.67 37.92 68.33Q40.58 64 41.75 56.92Q42.92 49.92 42.92 41.83Q42.92 35.75 42 30Q41.08 24.17 39.17 19.42Q37.33 14.67 34.58 11.83Q31.83 9 28.17 9Q23.83 9 21.25 13.25Q18.75 17.42 17.58 24.42Q16.5 31.33 16.5 39.5Q16.5 45.5 17.42 51.42Q18.33 57.42 20.08 62.17Q21.92 67 24.58 69.83Q27.25 72.67 30.92 72.67ZM61.75 77.33Q60.83 77.33 60.58 77.08Q60.25 76.83 60.25 76.5Q60.25 75.92 60.67 75.67Q61.17 75.42 61.75 75.42Q65 75.42 69.25 73.33Q73.5 71.17 77.83 67.58Q82.25 64 85.67 59.5Q89.17 55 91.25 50.67Q93.33 46.33 94.33 41.75Q95.33 37.17 95.33 32Q95.33 28.17 94.08 24.25Q92.83 20.33 90.75 17.08Q88.58 13.92 85.83 11.92Q83.08 10 80.17 10Q76.75 10 74.33 11.92Q72 13.75 70.83 16.5Q69.58 19.25 69.58 22Q69.58 25.25 71.17 28.33Q72.75 31.5 75.33 33.5Q77.92 35.5 80.75 35.5Q84.08 35.5 85.67 34.58Q87.33 33.67 89.33 32.67Q89.83 32.42 90.08 33Q90.33 33.5 90.08 33.83Q89.58 34.92 88.5 35.92Q87.42 37 86.58 37.67Q85.83 38.17 84.17 39Q82.58 39.92 80.58 40.5Q78.67 41.17 76.67 41.17Q72.92 41.17 69.42 39.42Q65.92 37.67 63.75 34.58Q61.5 31.5 61.5 27.58Q61.5 23.92 63.17 20.17Q64.75 16.33 67.75 13.17Q70.67 10 74.83 8.08Q78.92 6.08 83.75 6.08Q87.67 6.08 91.33 8.25Q95.08 10.33 98.08 13.83Q101 17.33 102.75 21.75Q104.5 26.17 104.5 30.83Q104.5 37.5 102.67 43.17Q100.83 48.83 97.33 54Q93.92 59.25 88.92 64.58Q86 67.83 82.42 70.25Q78.92 72.58 75.25 74.25Q71.58 75.83 68.08 76.58Q64.5 77.33 61.75 77.33Z',
        garamondBold: 'M25.58 75.5Q19.17 75.5 14.33 72.5Q9.58 69.5 6.5 64.5Q3.33 59.42 1.83 53.42Q0.33 47.33 0.33 41.42Q0.33 35.33 1.83 29.08Q3.33 22.83 6.5 17.58Q9.75 12.33 14.83 9.17Q19.83 6 26.83 6Q33.33 6 38.08 9Q42.83 12 45.92 17.08Q49 22.08 50.58 28.08Q52.08 34 52.08 39.83Q52.08 46 50.58 52.25Q49.08 58.5 45.92 63.75Q42.67 69 37.67 72.25Q32.67 75.5 25.58 75.5ZM27.25 70.67Q30.67 70.67 32.75 66.58Q34.92 62.58 35.83 56Q36.83 49.42 36.83 41.75Q36.83 36.08 36.08 30.58Q35.33 25.08 33.83 20.58Q32.33 16.08 30.17 13.42Q28 10.75 25.08 10.75Q21.67 10.75 19.58 14.83Q17.5 18.92 16.67 25.5Q15.75 32 15.75 39.58Q15.75 45.25 16.42 50.83Q17.08 56.33 18.58 60.83Q20.08 65.33 22.17 68Q24.33 70.67 27.25 70.67ZM63.25 79.33Q61.67 79.33 61 78.83Q60.42 78.25 60.42 77.75Q60.42 76.5 61.33 75.83Q62.33 75.25 63.42 75.17Q67.33 74.67 71.42 72.33Q75.42 70.08 79.08 66.58Q82.83 63.17 85.67 59.58Q87.83 56.58 90.25 52.5Q92.67 48.33 94.42 43.17Q96.17 38 96.17 32.08Q96.17 28.42 95.17 24.83Q94.25 21.25 92.67 18.25Q91 15.25 89 13.5Q86.92 11.67 84.75 11.67Q82.17 11.67 80.33 13Q78.5 14.42 77.58 17Q76.67 19.67 76.67 23.75Q76.67 26.92 77.83 30Q79 33 80.83 35Q82.67 36.92 84.75 36.92Q86.33 36.92 87.42 36.58Q88.42 36.25 89.42 35.83Q91.67 34.92 92 36.08Q92.33 37.17 91.25 38.5Q90.75 39.33 90.17 39.92Q89.5 40.5 88.92 41Q88.17 41.75 86.42 42.75Q84.75 43.75 82.67 44.5Q80.67 45.17 78.75 45.17Q74.67 45.17 70.83 43.08Q66.92 41 64.33 37.42Q61.75 33.75 61.75 29.08Q61.75 25.58 63.75 21.58Q65.83 17.58 69.33 14.08Q72.92 10.58 77.5 8.42Q82.08 6.17 87.25 6.17Q91.92 6.17 96.08 8.25Q100.25 10.25 103.42 13.67Q106.58 17.17 108.42 21.58Q110.25 26.08 110.25 30.92Q110.25 37.67 107.83 43.75Q105.33 49.75 101.67 54.75Q98 59.75 94.25 63.42Q91.75 65.83 87.75 68.58Q83.83 71.33 79.33 73.83Q74.92 76.25 70.67 77.83Q66.42 79.33 63.25 79.33Z',
    },
    minutes: {
        avenir:       'M49.17 74H5.67V66.83L33.5 38.67Q35 37.17 36.5 35.33T39.17 31.67T41 27.58T41.67 23.08Q41.67 20.17 40.58 17.75T37.67 13.67T33.33 11.08T28.17 10.17Q22.75 10.17 19 13.58T14.17 22.25L7.33 20.92Q8.08 17.33 9.92 14.25T14.5 9T20.75 5.58T28.17 4.33Q32.33 4.33 36 5.5T42.5 9.08T47 14.92T48.67 23Q48.67 26.25 47.75 29.08T45.33 34.33T41.83 39.08T37.58 43.58L13.17 67.83H49.17V74ZM75.08 35.42H78Q81.25 35.42 84.17 34.75T89.17 32.58T92.58 28.67T93.83 22.92Q93.83 20.08 92.83 17.67T89.92 13.67T85.58 11.08T80.42 10.17Q76.08 10.17 72.75 12.17T67.5 18.17L61.58 15.08Q64.75 9.58 69.67 6.92T80.83 4.33Q84.83 4.33 88.42 5.5T94.75 9T99.08 14.75T100.67 22.58Q100.67 28.58 97.33 32.67T88.83 38.08V38.25Q94.75 39.5 98.67 43.92T102.58 55.17Q102.58 60.08 100.83 63.83T96.08 70.33T89.08 74.33T80.67 75.75Q73.25 75.75 67.67 72.17T59.17 61.83L65.75 59.42Q67.5 64.33 71.5 67T80.67 69.75Q83.58 69.75 86.33 68.75T91.08 65.92T94.42 61.33T95.67 55.17Q95.67 51.33 94.17 48.67T90.25 44.42T84.58 42.08T78 41.33H75.08V35.42Z',
        avenirBold:   'M7.33 63.42L33.58 37.75Q36.75 34.67 38.58 31.5T40.5 24.33Q40.5 19.58 37.33 16.75T29.67 13.92Q24.75 13.92 21.75 17.25T18.17 25.5L6.92 23.83Q7.33 19.58 9.33 16T14.17 9.83T21.17 5.83T29.83 4.33Q34.25 4.33 38.33 5.58T45.58 9.33T50.58 15.58T52.5 24.17Q52.5 27.42 51.67 30.25T49.25 35.58T45.83 40.33T41.58 44.75L22 63.42H52.58V74H7.33V63.42ZM74.83 34H77.92Q80.5 34 83.08 33.5T87.75 31.92T91.08 28.83T92.42 23.67Q92.42 21.42 91.58 19.58T89.17 16.5T85.75 14.58T81.67 13.92Q77.42 13.92 74.67 16.33T70.83 22.67L60 19.83Q60.83 16.42 62.92 13.58T67.92 8.67T74.5 5.42T82.17 4.33Q86.58 4.33 90.58 5.5T97.58 9.08T102.33 14.83T104 22.92Q104 28.42 100.75 32.67T91.83 38.25V38.5Q98.08 39.75 102 44.08T105.83 54.83Q105.83 60.08 103.83 64T98.33 70.58T90.67 74.42T82.17 75.75T74 74.83T66.92 72T61.25 67T57.5 59.67L68.25 56.33Q69.5 60.33 72.92 63.17T81.75 65.92Q84 65.92 86.25 65.33T90.25 63.33T93.17 59.92T94.25 54.67Q94.25 51.42 92.67 49.17T88.67 45.67T83.33 43.75T77.75 43.17H74.83V34Z',
        leco:         'M9.42 27V6.83H56.42V47.08H22.83V60.58H56.42V74H9.42V33.67H43V20.25H22.83V27H9.42ZM108.25 6.83V74H61.25V60.58H94.83V47.08H67.92V33.67H94.83V20.25H61.25V6.83H108.25Z',
        lecoLight:    'M9.42 20.25V6.83H56.42V43.75H16.17V67.17H56.42V74H9.42V37.08H49.75V13.58H16.17V20.25ZM108.25 6.83V74H61.25V67.25H101.58V43.75H68V37.08H101.58V13.58H61.25V6.83Z',
        garamond:     'M47.92 74.33Q47.17 74.33 45.5 74.25Q43.92 74.08 41.83 74Q39.83 73.92 37.58 73.75Q35.33 73.67 33.33 73.67Q31.25 73.67 27.5 73.75Q23.75 73.92 19.58 73.92Q15.42 74 12.25 74.08Q9.17 74.25 8.25 74.25Q7.83 74.25 7 73.5Q6.08 72.83 6.08 72.42Q6.08 71.83 6.33 71.67Q6.58 71.42 7 71L25.42 51.83Q30.83 46.25 34.42 39.33Q38 32.33 38 25.25Q38 19.5 34.5 16.17Q31.08 12.92 26.42 12.92Q22 12.92 18.25 15.33Q14.42 17.75 12.17 21.67Q11.83 22.33 11.33 22.75Q10.83 23.17 10.33 23.17Q9.92 23.17 9.33 22.25Q8.75 21.33 8.75 19.92Q8.75 18.67 10.25 16.33Q11.83 14.08 14.67 11.75Q17.42 9.33 21.33 7.67Q25.25 6 29.75 6Q35.17 6 39 8.25Q42.92 10.5 45 14.5Q47 18.5 47 23.83Q47 26.75 46 30.08Q45 33.42 43.17 36.67Q41.42 39.92 39.17 42.67Q37.75 44.42 35 47.25Q32.25 50.17 29.08 53.42Q25.83 56.58 23 59.5Q20.17 62.33 18.33 64.25Q16.5 66.08 16.5 66.33Q16.5 66.92 18 67.08Q19.83 67.17 22.17 67.33Q24.5 67.42 27.5 67.42Q30.58 67.42 33.75 67.33Q36.92 67.17 39.83 67.08Q42.92 66.92 44.67 66.58Q46.42 66.25 47.58 65.17Q48.75 64.17 50.08 62Q50.5 61.33 51.17 60.42Q51.92 59.5 52.75 59.5Q53.25 59.5 53.42 60Q53.5 60.42 53.5 60.83Q53.5 61.33 52.92 62.92Q52.33 64.5 51.58 66.75Q50.83 69 50.25 71.75Q49.92 73.5 49.17 73.92Q48.42 74.33 47.92 74.33ZM71.92 78.08Q69.42 78.08 66.58 77.33Q63.75 76.5 61.83 75.42Q59.92 74.33 59.92 73.5Q59.92 72.67 60.75 71.5Q61.58 70.33 62.75 69.42Q63.92 68.5 64.67 68.5Q65.83 68.5 66.83 69.17Q67.75 69.75 68.83 70.67Q70.25 71.83 72.08 72.92Q73.92 74 76.92 74Q80.92 74 84.5 71.92Q88.08 69.92 90.42 66.25Q92.75 62.67 92.75 58.25Q92.75 54 91.33 50.17Q89.92 46.25 87.17 43.75Q84.42 41.17 80.17 41.17Q79 41.17 77 41.42Q75 41.75 73.17 42.83Q72.67 43.17 71.92 42.75Q71.25 42.33 71.33 41.83Q71.75 40.42 72.67 39.5Q73.5 38.58 74.58 38.17Q75.58 37.75 77.67 36.5Q79.83 35.25 82.33 33.33Q84.75 31.5 86.42 29.08Q88.17 26.58 88.17 23.67Q88.17 19.25 85.33 15.92Q82.42 12.5 78 12.5Q73.83 12.5 71.25 13.92Q68.75 15.42 67.25 17.25Q65.75 19.17 65 20.67Q64.25 22.08 63.58 22.08Q63.17 22.08 62.5 21.42Q61.83 20.75 61.83 20Q61.83 18.67 63 16.33Q64.25 14 66.75 11.58Q69.25 9.25 73.25 7.58Q77.17 6 82.67 6Q86.25 6 89.42 7.58Q92.58 9.17 94.67 11.92Q96.67 14.67 96.67 18.33Q96.67 20.92 95.58 23.67Q94.58 26.5 92.08 29.25Q89.58 32 85.33 34.5Q85.08 34.58 85.08 34.83Q85.08 35.17 85.5 35.17Q90.08 35.17 93.75 37.5Q97.5 39.75 99.67 43.58Q101.92 47.42 101.92 52.08Q101.92 58.17 99.33 62.92Q96.67 67.75 92.42 71.17Q88.08 74.5 82.75 76.33Q77.42 78.08 71.92 78.08Z',
        garamondBold: 'M46 74.42Q45.33 74.42 42.83 74.33Q40.33 74.25 36.92 74.17Q33.5 74.08 30 74Q26.42 73.92 23.75 73.92Q21.17 73.92 18.25 74Q15.33 74.08 12.67 74.08Q10.08 74.17 8.25 74.25Q6.42 74.33 6.08 74.33Q5.83 74.33 4.92 73.75Q3.92 73.17 3.92 72.5Q3.92 71.58 4.42 71.08Q4.92 70.5 5.33 70.08L21 52.58Q26.67 46.42 29.5 40.08Q32.33 33.83 32.33 26.33Q32.33 21.75 30 18.67Q27.67 15.67 23.5 15.67Q19.75 15.67 16.42 17.83Q13.08 20 10.67 22.5Q10.08 23.17 9.17 23.92Q8.25 24.75 7.17 24.75Q6.42 24.75 6.08 24Q5.75 23.33 5.75 22.17Q5.75 20.5 7.5 17.83Q9.33 15.17 12.5 12.5Q15.67 9.75 19.75 7.92Q23.92 6.08 28.58 6.08Q34.17 6.08 38 8.58Q41.83 11.08 43.92 15.17Q46 19.25 46 24.17Q46 27.17 44.67 30.75Q43.33 34.33 41.33 37.67Q39.42 40.92 37.33 43.25Q35.75 45.08 33.25 47.5Q30.83 49.92 28.25 52.42Q25.67 54.92 23.33 57.08Q21 59.25 19.5 60.67Q18.08 62.08 18.08 62.25Q18.08 62.75 19.67 62.92Q21.75 63.17 24.42 63.25Q27.17 63.33 29.42 63.33Q31.83 63.33 34.25 63.25Q36.75 63.17 39 62.92Q41.42 62.67 43.17 62Q45 61.25 46.33 59.92Q47.67 58.5 48.75 56.42Q49.08 55.67 49.92 54.83Q50.75 54.08 51.75 54.08Q52.58 54.08 52.92 54.75Q53.25 55.5 52.92 56.25Q52 59 50.92 62.25Q49.92 65.58 49.17 68.25Q48.42 70.92 48.33 72Q48.25 73.17 47.25 73.83Q46.33 74.42 46 74.42ZM73.08 78.08Q70.67 78.08 68.17 77.42Q65.67 76.67 63.92 75.42Q62.17 74.25 62.17 72.83Q62.17 71.5 62.67 69.92Q63.08 68.42 64 67.33Q64.83 66.33 66 66.33Q66.92 66.33 67.5 66.75Q68.17 67.17 68.92 67.92Q70.42 69.67 72.5 71.58Q74.5 73.5 77.08 73.5Q80.58 73.5 83.33 71.58Q86.17 69.75 87.83 66.25Q89.58 62.75 89.58 58Q89.58 53.58 88.33 50.17Q87.08 46.75 84.75 44.75Q82.42 42.83 79 42.83Q76.33 42.83 74.67 43.75Q72.92 44.67 72 45.17Q71.17 45.58 70.17 45.08Q69.08 44.5 69.25 43.67Q69.83 41.5 71.08 40.17Q72.42 38.75 73.83 38Q77 36.33 79.42 34.75Q81.75 33.08 83.42 31.25Q85 29.42 85.83 27.17Q86.67 25 86.67 22.25Q86.67 18.83 84.25 16.33Q81.92 13.83 78.17 13.83Q75 13.83 72.83 15.08Q70.58 16.42 69.08 18.17Q67.58 19.83 66.67 21.17Q65.67 22.5 65.17 22.5Q64.42 22.5 63.67 21.67Q62.92 20.75 62.92 19.83Q62.92 18.92 64.08 16.67Q65.25 14.42 67.83 12Q70.42 9.58 74.5 7.92Q78.67 6.17 84.58 6.17Q88.83 6.17 92.42 7.75Q96 9.42 98.17 12.33Q100.33 15.25 100.33 19.25Q100.33 21.67 98.92 24.83Q97.42 28 94.17 30.75Q90.92 33.5 85.75 34.75Q85.42 34.83 85.42 35.08Q85.42 35.33 85.92 35.33Q91.5 35.33 95.83 37.42Q100.17 39.5 102.67 43.25Q105.17 46.92 105.17 52.17Q105.17 58.17 102.5 62.92Q99.83 67.67 95.33 71.08Q90.75 74.42 85 76.25Q79.33 78.08 73.08 78.08Z',
    },
} as const;

// the watch's [typeface][bold] face table, by TIME_PATHS key
const TIME_FACES = {
    avenir:   { light: 'avenir',    bold: 'avenirBold' },
    leco:     { light: 'lecoLight', bold: 'leco' },
    garamond: { light: 'garamond',  bold: 'garamondBold' },
} as const;

function getTimePaths(s: Pick<Settings, 'clockFont' | 'boldHours' | 'boldMinutes' | 'leadingZero'>) {
    const faces = TIME_FACES[s.clockFont] ?? TIME_FACES.avenir;
    const hours = s.leadingZero ? TIME_PATHS.hoursLeadingZero : TIME_PATHS.hours;
    return {
        hourPath: hours[s.boldHours ? faces.bold : faces.light],
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

    if (parts.lead) {
        const fitted = fitText(parts.lead, 'current', fonts);
        nodes.push(textNode('lead', parts.lead, cursor, centerY - fitted.capH / 2,
                            fitted.fontSize, colors.text, 'start'));
        cursor += fitted.w + INLINE_GAP;
    }

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
        const vPadding = m.vPadding;

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
                <svg x={timeX} y={timeY} width={SVG_W} height={SVG_H} viewBox="0 0 109 81" overflow="visible">
                    <path d={hourPath} fill={timeColor} />
                </svg>
                <svg x={timeX} y={timeY + SVG_H} width={SVG_W} height={SVG_H} viewBox="0 0 109 81" overflow="visible">
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
                                <svg x={timeX} y={timeY} width={SVG_W} height={SVG_H} viewBox="0 0 109 81" overflow="visible">
                                    <path d={hourPath} fill={timeColor} />
                                </svg>
                                <svg x={timeX} y={timeY + SVG_H} width={SVG_W} height={SVG_H} viewBox="0 0 109 81" overflow="visible">
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

export const WatchPreview: React.FC<WatchPreviewProps> = ({ overrideSettings }) => {
    const { settings } = useConfig();
    const capabilities = useCapabilities();

    const effectiveSettings = { ...settings, ...overrideSettings } as Settings;

    if (capabilities.ROUND) {
        return <RoundPreview effectiveSettings={effectiveSettings} capabilities={capabilities} />;
    }
    return <RectPreview effectiveSettings={effectiveSettings} capabilities={capabilities} />;
};

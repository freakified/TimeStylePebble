import React from 'react';
import { Modal, ModalOverlay, Dialog, Button, Heading } from 'react-aria-components';
import { useConfig, useCapabilities } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { PEBBLE_COLORS, getColorName } from '../data/colors';
import { FormItem } from './FormItem';

export type ColorMode = 'color' | 'bw' | 'bw-grey';

const ORDERED_COLOR_GRID: (string | null)[] = [
  '#000000',
  '#555555',
  '#AAAAAA',
  '#FFFFFF',
  null,
  null,
  null,
  null,
  null,
  '#FF0055',
  null,
  null,
  '#AA5555',
  '#550000',
  '#AA0000',
  '#FF0000',
  '#FF5555',
  '#FFAAAA',
  null,
  null,
  null,
  '#FF5500',
  null,
  null,
  null,
  null,
  '#AA5500',
  '#FFAA00',
  '#FFAA55',
  null,
  '#AAAA55',
  '#555500',
  '#AAAA00',
  '#FFFF00',
  '#FFFF55',
  '#FFFFAA',
  null,
  null,
  '#55AA00',
  '#AAFF00',
  '#AAFF55',
  null,
  null,
  null,
  null,
  '#55FF00',
  null,
  null,
  '#55AA55',
  '#005500',
  '#00AA00',
  '#00FF00',
  '#55FF55',
  '#AAFFAA',
  null,
  null,
  null,
  '#00FF55',
  null,
  null,
  null,
  null,
  '#00AA55',
  '#00FFAA',
  '#55FFAA',
  null,
  '#55AAAA',
  '#005555',
  '#00AAAA',
  '#00FFFF',
  '#55FFFF',
  '#AAFFFF',
  null,
  null,
  '#0055AA',
  '#00AAFF',
  '#55AAFF',
  null,
  null,
  null,
  null,
  '#0055FF',
  null,
  null,
  '#5555AA',
  '#000055',
  '#0000AA',
  '#0000FF',
  '#5555FF',
  '#AAAAFF',
  null,
  null,
  null,
  '#5500FF',
  null,
  null,
  null,
  null,
  '#5500AA',
  '#AA00FF',
  '#AA55FF',
  null,
  '#AA55AA',
  '#550055',
  '#AA00AA',
  '#FF00FF',
  '#FF55FF',
  '#FFAAFF',
  null,
  null,
  '#AA0055',
  '#FF00AA',
  '#FF55AA',
  null,
];

export const COLOR_PALETTES: Record<ColorMode, string[]> = {
  color: PEBBLE_COLORS,
  bw: ['000000', 'FFFFFF'],
  'bw-grey': ['000000', 'FFFFFF', 'AAAAAA'],
};

interface ColorPickerProps {
  label: string;
  description?: string;
  messageKey: keyof Settings;
  bwAllowGrey?: boolean;
}

export const ColorPicker: React.FC<ColorPickerProps> = ({
  label,
  description,
  messageKey,
  bwAllowGrey = true,
}) => {
  const { settings, updateSetting } = useConfig();
  const capabilities = useCapabilities();
  const [isOpen, setIsOpen] = React.useState(false);
  const rawValue = settings[messageKey];
  const value = (typeof rawValue === 'string' ? rawValue : '000000').toUpperCase().replace('#', '');

  const resolvedMode: ColorMode = capabilities.COLOR ? 'color' : (bwAllowGrey ? 'bw-grey' : 'bw');
  const palette = COLOR_PALETTES[resolvedMode];
  const useBlanks = resolvedMode === 'color';

  const colorGrid = useBlanks
    ? ORDERED_COLOR_GRID
    : ORDERED_COLOR_GRID.filter(
      (c): c is string => c !== null && palette.includes(c.replace('#', '')),
    );

  return (
    <FormItem
      label={label}
      description={description}
      className="halite-color-picker-v2"
    >
      <Button
        onPress={() => setIsOpen(true)}
        className="halite-color-trigger"
        aria-label={`Select ${label} color`}
      >
        <div className="halite-color-value">
          <span className="halite-color-name">{getColorName(value)}</span>
          <div className="halite-color-swatch" style={{ backgroundColor: `#${value}` }} />
        </div>
      </Button>

      <ModalOverlay
        isOpen={isOpen}
        onOpenChange={setIsOpen}
        className="halite-color-modal-overlay"
        isDismissable
      >
        <Modal className="halite-color-modal">
          <Dialog className="halite-color-dialog">
            {({ close }) => (
              <>
                <div className="halite-color-modal-header">
                  <Heading slot="title">{label}</Heading>
                  <Button className="halite-color-modal-close" onPress={close}>
                    ×
                  </Button>
                </div>
                <div className="halite-color-modal-grid">
                  {colorGrid.map((color, index) => {
                    if (color === null) {
                      return <div key={`blank-${index}`} className="halite-color-swatch-blank" />;
                    }
                    const colorHex = color.replace('#', '');
                    return (
                      <Button
                        key={colorHex}
                        className={`halite-color-swatch ${value === colorHex ? 'active' : ''}`}
                        style={{ backgroundColor: color }}
                        aria-label={getColorName(colorHex)}
                        onPress={() => {
                          updateSetting(messageKey, colorHex);
                          close();
                        }}
                      />
                    );
                  })}
                </div>
              </>
            )}
          </Dialog>
        </Modal>
      </ModalOverlay>
    </FormItem>
  );
};

import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { FormItem } from './FormItem';
import { GridList, GridListItem, Text } from 'react-aria-components';
import { WatchPreview, WatchPreviewProps } from './WatchPreview';

export const SelectCards: React.FC<{
  label: string;
  description?: string;
  messageKey: keyof Settings;
  options: { label: string; value: string | number }[];
  optionSettings?: Record<string | number, Partial<Settings>>;
  watchPreviewProps?: Partial<WatchPreviewProps>;
}> = ({ label, description, messageKey, options, optionSettings, watchPreviewProps }) => {
  const { settings, updateSetting } = useConfig();
  const currentValue = settings[messageKey];

  const items = React.useMemo(
    () => options.map((opt) => ({ ...opt, id: String(opt.value) })),
    [options]
  );

  return (
    <FormItem label={label} description={description} className="halite-select-cards">
      <GridList
        items={items}
        selectionMode="single"
        selectedKeys={[String(currentValue)]}
        onSelectionChange={(keys) => {
          const selected = Array.from(keys)[0] as string;
          if (!selected) return;
          const num = parseInt(selected, 10);
          updateSetting(messageKey, isNaN(num) ? selected : num);
        }}
        layout="grid"
        aria-label={label}
        className="halite-select-cards-grid"
      >
        {(item) => (
          <GridListItem
            id={item.id}
            className="halite-select-card"
            textValue={item.label}
          >
            <div className="halite-select-card-preview">
              <WatchPreview overrideSettings={optionSettings?.[item.value]} {...watchPreviewProps} />
            </div>
            <Text className="halite-select-card-label">{item.label}</Text>
          </GridListItem>
        )}
      </GridList>
    </FormItem>
  );
};

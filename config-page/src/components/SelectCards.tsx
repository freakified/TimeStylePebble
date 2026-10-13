import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { FormItem } from './FormItem';
import { GridList, GridListItem, Text } from 'react-aria-components';
import { WatchPreview } from './WatchPreview';

export const SelectCards: React.FC<{
  label: string;
  description?: string;
  // without a messageKey, each card stands for its optionSettings: the
  // card whose settings all match is selected, and picking one applies them
  messageKey?: keyof Settings;
  options: { label: string; value: string | number | boolean }[];
  optionSettings?: Record<string, Partial<Settings>>;
}> = ({ label, description, messageKey, options, optionSettings }) => {
  const { settings, updateSetting, updateSettings } = useConfig();
  const currentValue = messageKey
    ? settings[messageKey]
    : options.find((opt) => Object.entries(optionSettings?.[String(opt.value)] ?? {})
        .every(([key, value]) => settings[key as keyof Settings] === value))?.value;

  const items = React.useMemo(
    () => options.map((opt) => ({ ...opt, id: String(opt.value) })),
    [options]
  );

  return (
    <FormItem label={label} description={description} className="halite-select-cards">
      <GridList
        items={items}
        selectionMode="single"
        selectedKeys={currentValue === undefined ? [] : [String(currentValue)]}
        onSelectionChange={(keys) => {
          const selected = Array.from(keys)[0] as string;
          if (!selected) return;
          if (!messageKey) {
            const partial = optionSettings?.[selected];
            if (partial) updateSettings(partial);
            return;
          }
          const num = parseInt(selected, 10);
          const value = selected === 'true' ? true
                      : selected === 'false' ? false
                      : isNaN(num) ? selected : num;
          updateSetting(messageKey, value);
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
              <WatchPreview overrideSettings={optionSettings?.[String(item.value)]} />
            </div>
            <Text className="halite-select-card-label">{item.label}</Text>
          </GridListItem>
        )}
      </GridList>
    </FormItem>
  );
};

import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { FormItem } from './FormItem';

export const Select: React.FC<{
  label: string;
  description?: string;
  messageKey: keyof Settings;
  options: { label: string; value: string | number; category?: string }[];
}> = ({ label, description, messageKey, options }) => {
  const { settings, updateSetting } = useConfig();
  const value = settings[messageKey];
  const selectId = React.useId();

  const groupedOptions = React.useMemo(() => {
    const groups: { [key: string]: typeof options } = {};
    const noCategory: typeof options = [];

    options.forEach((opt) => {
      if (opt.category) {
        if (!groups[opt.category]) groups[opt.category] = [];
        groups[opt.category].push(opt);
      } else {
        noCategory.push(opt);
      }
    });

    return { groups, noCategory };
  }, [options]);

  return (
    <FormItem label={label} description={description} className="halite-select" htmlFor={selectId}>
      <select
        id={selectId}
        value={value}
        onChange={(e) => {
          const val = e.target.value;
          const num = parseInt(val, 10);
          updateSetting(messageKey, isNaN(num) ? val : num);
        }}
      >
        {groupedOptions.noCategory.map((opt) => (
          <option key={opt.value} value={opt.value}>
            {opt.label}
          </option>
        ))}
        {Object.entries(groupedOptions.groups).map(([category, opts]) => (
          <optgroup key={category} label={category}>
            {opts.map((opt) => (
              <option key={opt.value} value={opt.value}>
                {opt.label}
              </option>
            ))}
          </optgroup>
        ))}
      </select>
    </FormItem>
  );
};
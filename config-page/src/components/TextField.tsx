import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { FormItem } from './FormItem';

export const TextField: React.FC<{
  label: string;
  description?: string;
  messageKey: keyof Settings;
  maxLength?: number;
  transformValue?: (value: string) => string;
}> = ({ label, description, messageKey, maxLength, transformValue }) => {
  const { settings, updateSetting } = useConfig();
  const inputId = React.useId();
  const value = settings[messageKey];

  return (
    <FormItem label={label} description={description} className="halite-text-field" htmlFor={inputId}>
      <input
        id={inputId}
        type="text"
        className="halite-input"
        value={String(value ?? '')}
        maxLength={maxLength}
        onChange={(e) => {
          const nextValue = transformValue ? transformValue(e.target.value) : e.target.value;
          updateSetting(messageKey, nextValue as Settings[keyof Settings]);
        }}
      />
    </FormItem>
  );
};

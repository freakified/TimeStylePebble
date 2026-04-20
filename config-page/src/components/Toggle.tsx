import React from 'react';
import { Switch } from 'react-aria-components';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { FormItem, FormItemLabel } from './FormItem';

export const Toggle: React.FC<{
  label: string;
  description?: string;
  messageKey: keyof Settings;
  /** 'bool' emits 0/1 (default); 'yesno' emits 'yes'/'no' */
  valueType?: 'bool' | 'yesno';
}> = ({ label, description, messageKey, valueType = 'bool' }) => {
  const { settings, updateSetting } = useConfig();

  const isSelected = valueType === 'yesno'
    ? settings[messageKey] === 'yes'
    : !!settings[messageKey];

  return (
    <Switch
      isSelected={isSelected}
      onChange={(selected) =>
        updateSetting(messageKey, valueType === 'yesno' ? (selected ? 'yes' : 'no') : (selected ? 1 : 0))
      }
      className="halite-item halite-toggle"
    >
      <FormItemLabel label={label} description={description} />
      <div className="halite-switch">
        <div className="halite-switch-thumb" />
      </div>
    </Switch>
  );
};

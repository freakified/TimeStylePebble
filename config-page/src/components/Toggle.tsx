import React from 'react';
import { Switch } from 'react-aria-components';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { FormItemLabel } from './FormItem';

export const Toggle: React.FC<{
  label: string;
  description?: string;
  messageKey: keyof Settings;
  isDisabled?: boolean;
  // shown state, when it should differ from the stored setting
  isSelected?: boolean;
  // replaces the plain write of messageKey, for toggles that change more
  onChange?: (selected: boolean) => void;
}> = ({ label, description, messageKey, isDisabled, isSelected, onChange }) => {
  const { settings, updateSetting } = useConfig();

  const shown = isSelected ?? !!settings[messageKey];

  return (
    <Switch
      isSelected={shown}
      onChange={onChange ?? ((next) => updateSetting(messageKey, next))}
      isDisabled={isDisabled}
      className="halite-item halite-toggle"
    >
      <FormItemLabel label={label} description={description} />
      <div className="halite-switch">
        <div className="halite-switch-thumb" />
      </div>
    </Switch>
  );
};

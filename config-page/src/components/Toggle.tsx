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
  // replaces the plain write of messageKey, for toggles that change more
  onChange?: (selected: boolean) => void;
}> = ({ label, description, messageKey, isDisabled, onChange }) => {
  const { settings, updateSetting } = useConfig();

  const isSelected = !!settings[messageKey];

  return (
    <Switch
      isSelected={isSelected}
      onChange={onChange ?? ((selected) => updateSetting(messageKey, selected))}
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

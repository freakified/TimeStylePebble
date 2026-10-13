import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';
import { ALL_WIDGET_OPTIONS } from '../data/widgetTypes';
import { setShowChoice, showChoice, showOptions, ShowChoice } from '../utils/widgetParts';
import { FormItem } from './FormItem';

/*
 * One "Show" select per placed widget that has parts to hide, in the
 * order the widgets are placed. Each select is the same control as
 * Select, but its value is spread over two bitmask settings rather than
 * held in one key of its own.
 */
export const WidgetDisplayOptions: React.FC<{ widgetIds: number[] }> = ({ widgetIds }) => {
  const { settings, updateSettings } = useConfig();
  const baseId = React.useId();

  return (
    <>
      {widgetIds.map((widgetId) => {
        const options = showOptions(widgetId);
        const name = ALL_WIDGET_OPTIONS.find((w) => w.value === widgetId)?.label ?? 'Widget';
        const selectId = `${baseId}-${widgetId}`;

        return (
          <FormItem key={widgetId} label={name} className="halite-select" htmlFor={selectId}>
            <select
              id={selectId}
              value={showChoice(widgetId, settings)}
              onChange={(e) => updateSettings(
                setShowChoice(widgetId, Number(e.target.value) as ShowChoice, settings)
              )}
            >
              {options.map((opt) => (
                <option key={opt.value} value={opt.value}>{opt.label}</option>
              ))}
            </select>
          </FormItem>
        );
      })}
    </>
  );
};

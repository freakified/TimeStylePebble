import React from 'react';
import { useConfig, useCapabilities } from '../context/PebbleConfigContext';
import { Theme, ThemeSelectionKey } from '../utils/themeUtils';
import { FormItem } from './FormItem';
import { GridList, GridListItem, Text } from 'react-aria-components';
import { WatchPreview } from './WatchPreview';
import { SavedTheme } from '../hooks/useSavedThemes';
import customIconUrl from '../assets/custom-icon.svg';

export const ThemePicker: React.FC<{
  label?: string;
  messageKey: ThemeSelectionKey;
  themes: Theme[];
  savedThemes?: SavedTheme[];
  /** Image on the Custom card */
  customIcon?: string;
}> = ({ label, messageKey, themes, savedThemes = [], customIcon = customIconUrl }) => {
  const { settings, updateSetting, updateSettings } = useConfig();
  const capabilities = useCapabilities();
  const currentValue = settings[messageKey];

  const handleThemeChange = (themeId: string) => {
    if (themeId === 'custom') {
      updateSetting(messageKey, themeId);
      return;
    }

    // Check saved themes first
    const theme = savedThemes.find((t) => t.id === themeId) ?? themes[parseInt(themeId, 10)];

    // the selection and the theme's settings land together, so nothing
    // ever sees one without the other
    updateSettings({ ...theme?.settings, [messageKey]: themeId });
  };

  const themeList = React.useMemo(() => [
    { id: 'custom', name: 'Custom', settings: {}, isSaved: false },
    ...savedThemes.map((t) => ({
      id: t.id,
      name: t.name,
      settings: t.settings,
      isSaved: true,
    })),
    ...themes.map((theme, index) => ({
      id: index.toString(),
      name: theme.name,
      settings: theme.settings,
      isSaved: false,
    })),
  ], [themes, savedThemes]);

  return (
    <FormItem className="halite-theme-picker">
      <GridList
        items={themeList}
        selectionMode="single"
        selectedKeys={[currentValue as string]}
        onSelectionChange={(keys) => {
          const selected = Array.from(keys)[0] as string;
          if (selected) handleThemeChange(selected);
        }}
        layout='grid'
        aria-label={label}
        className="halite-theme-grid"
      >
        {(item) => (
          <GridListItem id={item.id} className="halite-theme-card" textValue={item.name}>
            <div className="halite-theme-card-preview">
              {item.id === 'custom' ? (
                <div className={`halite-custom-icon${capabilities.ROUND ? ' halite-custom-icon--round' : ''}`}>
                  <img src={customIcon} alt="" width={40} height={40} />
                </div>
              ) : (
                <WatchPreview overrideSettings={item.settings} />
              )}
            </div>
            <Text className="halite-theme-card-label">
              {item.name}
            </Text>
          </GridListItem>
        )}
      </GridList>
    </FormItem>
  );
};

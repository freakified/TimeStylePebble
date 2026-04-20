import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { FormItem } from './FormItem';
import { GridList, GridListItem, Text } from 'react-aria-components';
import { WatchPreview, WatchPreviewProps } from './WatchPreview';
import { SavedTheme } from '../hooks/useSavedThemes';
import customIconUrl from '../assets/custom-icon.svg';

export const ThemePicker: React.FC<{
  label?: string;
  messageKey: keyof Settings;
  themes: { name: string; settings: Record<string, string> }[];
  watchPreviewProps?: Partial<WatchPreviewProps>;
  savedThemes?: SavedTheme[];
}> = ({ label, messageKey, themes, watchPreviewProps, savedThemes = [] }) => {
  const { settings, updateSetting } = useConfig();
  const currentValue = settings[messageKey];

  const handleThemeChange = (themeId: string) => {
    updateSetting(messageKey, themeId);

    if (themeId === 'custom') return;

    // Check saved themes first
    const saved = savedThemes.find((t) => t.id === themeId);
    if (saved) {
      Object.entries(saved.settings).forEach(([key, value]) => {
        updateSetting(key, value);
      });
      return;
    }

    const theme = themes[parseInt(themeId, 10)];
    if (theme) {
      Object.entries(theme.settings).forEach(([key, value]) => {
        updateSetting(key, value);
      });
    }
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
        selectedKeys={[currentValue]}
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
                <div className="halite-custom-icon">
                  <img src={customIconUrl} alt="" width={40} height={38} />
                </div>
              ) : (
                <WatchPreview overrideSettings={item.settings} {...watchPreviewProps} />
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

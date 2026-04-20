import React, { useState } from 'react';
import { Button } from 'react-aria-components';
import { WatchPreview, WatchPreviewProps } from './WatchPreview';
import { ImportThemeModal } from './ImportThemeModal';
import { SavedTheme } from '../hooks/useSavedThemes';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { copyToClipboard } from '../utils/clipboard';

interface CustomThemePanelProps {
  /** The currently selected theme ID */
  themeId: string;
  /** The saved theme object, if a saved theme is selected */
  savedTheme?: SavedTheme;
  /** Props forwarded to WatchPreview */
  watchPreviewProps?: Partial<WatchPreviewProps>;
  /** Called to save current settings as a new theme */
  onSave: (settings: Settings) => SavedTheme;
  /** Called to delete a saved theme */
  onDelete: (id: string) => void;
  /** Called to import a theme from JSON */
  onImport: (json: string) => { success: boolean; error?: string };
  /** Called to export a theme to JSON string */
  onExport: (settings: Record<string, string>) => string;
  /** Called when themes change and we need to select a new one */
  onSelectTheme: (themeId: string) => void;
}

const ICONS = {
  SAVE: 'm352-293 128-76 129 76-34-144 111-95-147-13-59-137-59 137-147 13 112 95zM243-144l63-266L96-589l276-24 108-251 108 252 276 23-210 179 63 266-237-141zm237-333',
  SHARE: 'M360-240q-29.7 0-50.85-21.15T288-312v-480q0-29.7 21.15-50.85T360-864h384q29.7 0 50.85 21.15T816-792v480q0 29.7-21.15 50.85T744-240zm0-72h384v-480H360zM216-96q-29.7 0-50.85-21.15T144-168v-552h72v552h456v72zm144-216v-480z',
  IMPORT: 'M444-336v-342L339-573l-51-51 192-192 192 192-51 51-105-105v342zM263.72-192Q234-192 213-213.15T192-264v-72h72v72h432v-72h72v72q0 29.7-21.16 50.85T695.96-192z',
  DELETE: 'M312-144q-29.7 0-50.85-21.15T240-216v-480h-48v-72h192v-48h192v48h192v72h-48v479.57Q720-186 698.85-165T648-144zm336-552H312v480h336zM384-288h72v-336h-72zm120 0h72v-336h-72zM312-696v480z',
  PALETTE: 'M480-96q-79 0-149-30t-122.5-82.5T126-331 96-480q0-80 30.5-149.5t84-122 125-82.5T488-864q78 0 146.5 27T754-763t80.5 110T864-518q0 96-67 163t-163 67h-68q-8 0-14 5t-6 13q0 15 15 25t15 53q0 37-27 66.5T480-96M306.5-461.5Q324-479 324-504t-17.5-42.5T264-564t-42.5 17.5T204-504t17.5 42.5T264-444t42.5-17.5m120-144Q444-623 444-648t-17.5-42.5T384-708t-42.5 17.5T324-648t17.5 42.5T384-588t42.5-17.5m192 0Q636-623 636-648t-17.5-42.5T576-708t-42.5 17.5T516-648t17.5 42.5T576-588t42.5-17.5m120 144Q756-479 756-504t-17.5-42.5T696-564t-42.5 17.5T636-504t17.5 42.5T696-444t42.5-17.5M480-168q11 0 17.5-8.5T504-192q0-16-15-28t-15-50 26.5-64 64.5-26h69q66 0 112-46t46-112q0-115-88.5-194.5T488-792q-134 0-227 91t-93 221 91 221 221 91',
};

const Icon: React.FC<{ path: string }> = ({ path }) => (
  <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" viewBox="0 -960 960 960">
    <path d={path} />
  </svg>
);

export const CustomThemePanel: React.FC<CustomThemePanelProps> = ({
  themeId,
  savedTheme,
  watchPreviewProps,
  onSave,
  onDelete,
  onImport,
  onExport,
  onSelectTheme,
}) => {
  const { settings } = useConfig();
  const [importOpen, setImportOpen] = useState(false);
  const [copyFeedback, setCopyFeedback] = useState(false);

  const isCustom = themeId === 'custom';
  const isSaved = !!savedTheme;

  const handleSave = () => {
    const theme = onSave(settings);
    onSelectTheme(theme.id);
  };

  const handleShare = async () => {
    const settingsToExport = isSaved ? savedTheme!.settings : getVisibleSettings();
    const json = onExport(settingsToExport);
    const success = await copyToClipboard(json);
    if (success) {
      setCopyFeedback(true);
      setTimeout(() => setCopyFeedback(false), 2000);
    } else {
      // Ultimate fallback
      prompt('Copy this theme JSON:', json);
    }
  };

  const handleDelete = () => {
    if (savedTheme) {
      onDelete(savedTheme.id);
      onSelectTheme('custom');
    }
  };

  const handleImport = (json: string) => {
    const result = onImport(json);
    if (result.success) {
      // The imported theme is now in the list
    }
    return result;
  };

  /** Extract the 4 TimeStyle color settings from the current live settings */
  const getVisibleSettings = (): Record<string, string> => {
    const colorKeys: (keyof Settings)[] = ['color_bg', 'color_time', 'color_sidebar', 'sidebar_text_color'];
    const colorSettings: Record<string, string> = {};
    for (const key of colorKeys) {
      colorSettings[key] = String(settings[key]);
    }
    return colorSettings;
  };

  return (
    <div className="halite-custom-panel">
      <div className="halite-custom-panel-preview">
        <WatchPreview
          overrideSettings={isSaved ? savedTheme!.settings : undefined}
          {...watchPreviewProps}
        />
      </div>
      <div className="halite-custom-actions">
        {isCustom && (
          <>
            <Button className="halite-custom-action-btn halite-custom-action-btn--primary" onPress={handleSave}>
              <Icon path={ICONS.SAVE} />
              Save
            </Button>
            <Button className="halite-custom-action-btn" onPress={handleShare}>
              <Icon path={ICONS.SHARE} />
              {copyFeedback ? 'Copied!' : 'Share'}
            </Button>
            <Button className="halite-custom-action-btn" onPress={() => setImportOpen(true)}>
              <Icon path={ICONS.IMPORT} />
              Import
            </Button>
          </>
        )}
        {!isCustom && (
          <>
            <Button className="halite-custom-action-btn" onPress={() => onSelectTheme('custom')}>
              <Icon path={ICONS.PALETTE} />
              Customize
            </Button>
            <Button className="halite-custom-action-btn" onPress={handleShare}>
              <Icon path={ICONS.SHARE} />
              {copyFeedback ? 'Copied!' : 'Share'}
            </Button>
            {isSaved && (
              <Button className="halite-custom-action-btn halite-custom-action-btn--danger" onPress={handleDelete}>
                <Icon path={ICONS.DELETE} />
                Delete
              </Button>
            )}
          </>
        )}
      </div>
      <ImportThemeModal
        isOpen={importOpen}
        onOpenChange={setImportOpen}
        onImport={handleImport}
      />
    </div>
  );
};

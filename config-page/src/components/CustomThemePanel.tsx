import React, { useState } from 'react';
import { Button } from 'react-aria-components';
import { WatchPreview } from './WatchPreview';
import { ImportThemeModal } from './ImportThemeModal';
import { SavedTheme } from '../hooks/useSavedThemes';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { ThemeSettings } from '../utils/themeUtils';
import { copyToClipboard } from '../utils/clipboard';
import { Icon, ICONS } from './Icon';

interface CustomThemePanelProps {
  /** The currently selected theme ID */
  themeId: string;
  /** The saved theme object, if a saved theme is selected */
  savedTheme?: SavedTheme;
  /** Called to save current settings as a new theme */
  onSave: (settings: Settings) => SavedTheme;
  /** Called to delete a saved theme */
  onDelete: (id: string) => void;
  /** Called to import a theme from JSON */
  onImport: (json: string) => { success: boolean; error?: string };
  /** Called to export a theme to JSON string */
  onExport: (settings: ThemeSettings) => string;
  /** Called when themes change and we need to select a new one */
  onSelectTheme: (themeId: string) => void;
  /** Heading of the import dialog */
  importTitle?: string;
}

export const CustomThemePanel: React.FC<CustomThemePanelProps> = ({
  themeId,
  savedTheme,
  onSave,
  onDelete,
  onImport,
  onExport,
  onSelectTheme,
  importTitle,
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
    // the hook picks this kind's settings out of the live ones
    const json = onExport(isSaved ? savedTheme!.settings : settings);
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

  return (
    <div className="halite-custom-panel">
      <div className="halite-custom-panel-preview">
        <WatchPreview overrideSettings={isSaved ? savedTheme!.settings : undefined} />
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
              <Icon path={ICONS.EDIT} />
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
        title={importTitle}
      />
    </div>
  );
};

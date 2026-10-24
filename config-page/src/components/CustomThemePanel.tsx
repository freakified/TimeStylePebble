import React, { useState } from 'react';
import { Button } from 'react-aria-components';
import { WatchPreview } from './WatchPreview';
import { ImportThemeModal } from './ImportThemeModal';
import { SavedTheme } from '../hooks/useSavedThemes';
import { useConfig } from '../context/PebbleConfigContext';
import { Settings } from '../context/types';
import { ThemeSettings } from '../utils/themeUtils';
import { copyToClipboard } from '../utils/clipboard';

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
  /** Icon on the Customize button */
  customizeIcon?: 'SETTINGS' | 'EDIT';
}

const ICONS = {
  SAVE: 'm352-293 128-76 129 76-34-144 111-95-147-13-59-137-59 137-147 13 112 95zM243-144l63-266L96-589l276-24 108-251 108 252 276 23-210 179 63 266-237-141zm237-333',
  SHARE: 'M360-240q-29.7 0-50.85-21.15T288-312v-480q0-29.7 21.15-50.85T360-864h384q29.7 0 50.85 21.15T816-792v480q0 29.7-21.15 50.85T744-240zm0-72h384v-480H360zM216-96q-29.7 0-50.85-21.15T144-168v-552h72v552h456v72zm144-216v-480z',
  IMPORT: 'M444-336v-342L339-573l-51-51 192-192 192 192-51 51-105-105v342zM263.72-192Q234-192 213-213.15T192-264v-72h72v72h432v-72h72v72q0 29.7-21.16 50.85T695.96-192z',
  DELETE: 'M312-144q-29.7 0-50.85-21.15T240-216v-480h-48v-72h192v-48h192v48h192v72h-48v479.57Q720-186 698.85-165T648-144zm336-552H312v480h336zM384-288h72v-336h-72zm120 0h72v-336h-72zM312-696v480z',
  SETTINGS: 'm370-80-16-128q-13-5-24.5-12T307-235l-119 50L78-375l103-78q-1-7-1-13.5v-27q0-6.5 1-13.5L78-585l110-190 119 50q11-8 23-15t24-12l16-128h220l16 128q13 5 24.5 12t22.5 15l119-50 110 190-103 78q1 7 1 13.5v27q0 6.5-2 13.5l103 78-110 190-118-50q-11 8-23 15t-24 12L590-80H370Zm70-80h79l14-106q31-8 57.5-23.5T639-327l99 41 39-68-86-65q5-14 7-29.5t2-31.5q0-16-2-31.5t-7-29.5l86-65-39-68-99 42q-22-23-48.5-38.5T533-694l-13-106h-79l-14 106q-31 8-57.5 23.5T321-633l-99-41-39 68 86 64q-5 15-7 30t-2 32q0 16 2 31t7 30l-86 65 39 68 99-42q22 23 48.5 38.5T427-266l13 106Zm42-180q58 0 99-41t41-99q0-58-41-99t-99-41q-59 0-99.5 41T342-480q0 58 40.5 99t99.5 41Zm-2-140Z',
  EDIT: 'M200-200h57l391-391-57-57-391 391v57Zm-80 80v-170l528-527q12-11 26.5-17t30.5-6q16 0 31 6t26 18l55 56q12 11 17.5 26t5.5 30q0 16-5.5 30.5T817-647L290-120H120Zm640-584-56-56 56 56Zm-141 85-28-29 57 57-29-28Z',
};

const Icon: React.FC<{ path: string }> = ({ path }) => (
  <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" viewBox="0 -960 960 960">
    <path d={path} />
  </svg>
);

export const CustomThemePanel: React.FC<CustomThemePanelProps> = ({
  themeId,
  savedTheme,
  onSave,
  onDelete,
  onImport,
  onExport,
  onSelectTheme,
  importTitle,
  customizeIcon = 'SETTINGS',
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
              <Icon path={ICONS[customizeIcon]} />
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

import React, { createContext, useContext, useMemo } from 'react';
import { Settings, WatchInfo, Capabilities } from './types';
import { evaluateCapabilities } from '../utils/capabilities';
import { getDefaultSettings } from '../utils/defaultSettings';

interface ConfigContextType {
  settings: Settings;
  updateSetting: (key: keyof Settings, value: Settings[keyof Settings]) => void;
  save: () => void;
}

interface WatchInfoContextType {
  watchInfo: WatchInfo | null;
  capabilities: Capabilities;
}

const ConfigContext = createContext<ConfigContextType | undefined>(undefined);
const WatchInfoContext = createContext<WatchInfoContextType | undefined>(undefined);

export const useConfig = () => {
  const context = useContext(ConfigContext);
  if (!context) throw new Error('useConfig must be used within a PebbleConfigProvider');
  return context;
};

export const useCapabilities = (): Capabilities => {
  const context = useContext(WatchInfoContext);
  if (!context) throw new Error('useCapabilities must be used within a PebbleConfigProvider');
  return context.capabilities;
};

export const useWatchInfo = (): WatchInfo | null => {
  const context = useContext(WatchInfoContext);
  if (!context) throw new Error('useWatchInfo must be used within a PebbleConfigProvider');
  return context.watchInfo;
};

const parseWatchInfo = (): WatchInfo | null => {
  try {
    const params = new URLSearchParams(window.location.search);
    const watchInfoStr = params.get('watchInfo');
    if (watchInfoStr) {
      return JSON.parse(decodeURIComponent(watchInfoStr));
    }
  } catch {
    console.warn('Failed to parse watchInfo from URL');
  }
  return null;
};

export const PebbleConfigProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const watchInfo = useMemo(() => parseWatchInfo(), []);
  const capabilities = useMemo(() => evaluateCapabilities(watchInfo), [watchInfo]);

  const [settings, setSettings] = React.useState<Settings>(() => {
    try {
      const params = new URLSearchParams(window.location.search);
      const urlSettings = JSON.parse(params.get('settings') || '{}');
      return { ...getDefaultSettings(capabilities), ...urlSettings };
    } catch {
      return getDefaultSettings(capabilities);
    }
  });

  const updateSetting = (key: keyof Settings, value: Settings[keyof Settings]) => {
    setSettings((prev) => ({ ...prev, [key]: value }));
  };

  const save = () => {
    const params = new URLSearchParams(window.location.search);
    const returnTo = params.get('return_to') || 'pebblejs://close#';
    // Strip internal UI keys that aren't watch settings
    const { SETTING_THEME, SETTING_NIGHT_THEME, SETTING_USE_NIGHT_THEME, ...watchSettings } = settings;
    window.location.href = returnTo + encodeURIComponent(JSON.stringify(watchSettings));
  };

  return (
    <WatchInfoContext.Provider value={{ watchInfo, capabilities }}>
      <ConfigContext.Provider value={{ settings, updateSetting, save }}>
        {children}
      </ConfigContext.Provider>
    </WatchInfoContext.Provider>
  );
};

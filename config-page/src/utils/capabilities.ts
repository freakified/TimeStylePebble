import { WatchInfo, Capabilities } from '../context/types';

const CAPABILITY_MAP: Record<keyof Capabilities, string[]> = {
  APLITE: ['aplite'],
  BASALT: ['basalt'],
  CHALK: ['chalk'],
  DIORITE: ['diorite'],
  EMERY: ['emery'],
  FLINT: ['flint'],
  GABBRO: ['gabbro'],
  BW: ['aplite', 'diorite', 'flint'],
  COLOR: ['basalt', 'chalk', 'emery', 'gabbro'],
  ROUND: ['chalk', 'gabbro'],
  RECT: ['aplite', 'basalt', 'diorite', 'emery', 'flint'],
  DISPLAY_144x168: ['aplite', 'basalt', 'diorite', 'flint'],
  DISPLAY_180x180_ROUND: ['chalk'],
  DISPLAY_200x228: ['emery'],
  DISPLAY_260x260_ROUND: ['gabbro'],
  MICROPHONE: ['basalt', 'chalk', 'diorite', 'emery', 'flint', 'gabbro'],
  SMARTSTRAP: ['basalt', 'chalk', 'diorite', 'emery'],
  SMARTSTRAP_POWER: ['basalt', 'chalk', 'emery'],
  HEALTH: ['basalt', 'chalk', 'diorite', 'emery', 'flint', 'gabbro'],
};

const getCapabilitiesForPlatform = (platform: string): Capabilities => {
  const capabilities = {} as Capabilities;

  Object.entries(CAPABILITY_MAP).forEach(([key, platforms]) => {
    capabilities[key as keyof Capabilities] = platforms.includes(platform);
  });

  return capabilities;
};

export const DEFAULT_CAPABILITIES: Capabilities = getCapabilitiesForPlatform('emery');

export const evaluateCapabilities = (watchInfo: WatchInfo | null): Capabilities => {
  if (!watchInfo) {
    return { ...DEFAULT_CAPABILITIES };
  }

  return getCapabilitiesForPlatform(watchInfo.platform);
};

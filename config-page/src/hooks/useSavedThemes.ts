import { useState, useCallback, useEffect } from 'react';
import { Settings } from '../context/types';

export interface SavedTheme {
    id: string;
    name: string;
    settings: Record<string, string>;
}

const STORAGE_KEY = 'timestyle-saved-themes';

/** The color setting keys a theme holds */
const THEME_KEYS: (keyof Settings)[] = [
    'colorBackground',
    'colorTime',
    'colorSidebar',
];

function loadFromStorage(key: string): SavedTheme[] {
    try {
        const raw = localStorage.getItem(key);
        return raw ? JSON.parse(raw) : [];
    } catch {
        return [];
    }
}

function saveToStorage(key: string, themes: SavedTheme[]) {
    localStorage.setItem(key, JSON.stringify(themes));
}

function getNextName(themes: SavedTheme[]): string {
    let max = 0;
    for (const t of themes) {
        const match = t.name.match(/^Saved Theme (\d+)$/);
        if (match) max = Math.max(max, parseInt(match[1], 10));
    }
    return `Saved Theme ${max + 1}`;
}

function generateId(): string {
    return `saved_${Date.now()}_${Math.random().toString(36).slice(2, 8)}`;
}

export function useSavedThemes() {
    const [savedThemes, setSavedThemes] = useState<SavedTheme[]>(() =>
        loadFromStorage(STORAGE_KEY)
    );

    // Persist on change
    useEffect(() => {
        saveToStorage(STORAGE_KEY, savedThemes);
    }, [savedThemes]);

    const saveTheme = useCallback(
        (settings: Settings): SavedTheme => {
            const colorSettings: Record<string, string> = {};
            for (const key of THEME_KEYS) {
                colorSettings[key] = String(settings[key]);
            }
            const theme: SavedTheme = {
                id: generateId(),
                name: getNextName(savedThemes),
                settings: colorSettings,
            };
            setSavedThemes((prev) => [...prev, theme]);
            return theme;
        },
        [savedThemes]
    );

    const deleteTheme = useCallback((id: string) => {
        setSavedThemes((prev) => prev.filter((t) => t.id !== id));
    }, []);

    const importTheme = useCallback(
        (json: string): { success: boolean; error?: string; theme?: SavedTheme } => {
            try {
                const parsed = JSON.parse(json);

                // Accept either { settings: { ... } } or just { color_bg: ..., ... }
                const settings: Record<string, string> =
                    parsed.settings && typeof parsed.settings === 'object'
                        ? parsed.settings
                        : parsed;

                // Validate that at least some expected keys exist
                const validKeys = THEME_KEYS.filter((k) => k in settings);
                if (validKeys.length === 0) {
                    return {
                        success: false,
                        error: 'No valid theme settings found in the pasted JSON.',
                    };
                }

                const colorSettings: Record<string, string> = {};
                for (const key of THEME_KEYS) {
                    if (key in settings) {
                        colorSettings[key] = String(settings[key]);
                    }
                }

                const theme: SavedTheme = {
                    id: generateId(),
                    name: parsed.name || getNextName(savedThemes),
                    settings: colorSettings,
                };
                setSavedThemes((prev) => [...prev, theme]);
                return { success: true, theme };
            } catch {
                return { success: false, error: 'Invalid JSON. Please check and try again.' };
            }
        },
        [savedThemes]
    );

    const exportTheme = useCallback(
        (settings: Record<string, string>): string => {
            return JSON.stringify(settings, null, 2);
        },
        []
    );

    return { savedThemes, saveTheme, deleteTheme, importTheme, exportTheme };
}

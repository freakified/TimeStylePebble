import { useState, useCallback, useEffect, useMemo } from 'react';
import { Capabilities, Settings } from '../context/types';
import { ThemeKind, ThemeSettings } from '../utils/themeUtils';

export interface SavedTheme {
    id: string;
    name: string;
    settings: ThemeSettings;
    /** the kind's scope it was saved under; absent for unscoped kinds */
    scope?: string;
}

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

function getNextName(themes: SavedTheme[], prefix: string): string {
    let max = 0;
    for (const t of themes) {
        const match = t.name.match(new RegExp(`^${prefix} (\\d+)$`));
        if (match) max = Math.max(max, parseInt(match[1], 10));
    }
    return `${prefix} ${max + 1}`;
}

/** The user's saved themes of one kind, as last persisted, for this watch */
export function loadSavedThemes(kind: ThemeKind, capabilities: Capabilities): SavedTheme[] {
    const scope = kind.scope?.(capabilities);
    return loadFromStorage(kind.storageKey).filter((t) => t.scope === scope);
}

function generateId(): string {
    return `saved_${Date.now()}_${Math.random().toString(36).slice(2, 8)}`;
}

export function useSavedThemes(kind: ThemeKind, capabilities: Capabilities) {
    // every saved theme of this kind, including other scopes', so saving
    // never drops themes another watch can see
    const [allThemes, setAllThemes] = useState<SavedTheme[]>(() =>
        loadFromStorage(kind.storageKey)
    );

    // Persist on change
    useEffect(() => {
        saveToStorage(kind.storageKey, allThemes);
    }, [kind.storageKey, allThemes]);

    const scope = kind.scope?.(capabilities);
    const savedThemes = useMemo(
        () => allThemes.filter((t) => t.scope === scope),
        [allThemes, scope]
    );

    const addTheme = useCallback(
        (settings: ThemeSettings, name?: string): SavedTheme => {
            const theme: SavedTheme = {
                id: generateId(),
                name: name || getNextName(savedThemes, kind.savedNamePrefix),
                settings,
                ...(scope !== undefined && { scope }),
            };
            setAllThemes((prev) => [...prev, theme]);
            return theme;
        },
        [savedThemes, kind.savedNamePrefix, scope]
    );

    const saveTheme = useCallback(
        (settings: Settings): SavedTheme =>
            addTheme(kind.normalize({ ...settings }, capabilities) ?? {}),
        [addTheme, kind, capabilities]
    );

    const deleteTheme = useCallback((id: string) => {
        setAllThemes((prev) => prev.filter((t) => t.id !== id));
    }, []);

    const importTheme = useCallback(
        (json: string): { success: boolean; error?: string; theme?: SavedTheme } => {
            try {
                const parsed = JSON.parse(json);

                // Accept either { settings: { ... } } or just { colorBackground: ..., ... }
                const settings: Record<string, unknown> =
                    parsed.settings && typeof parsed.settings === 'object'
                        ? parsed.settings
                        : parsed;

                // a scoped theme only makes sense on the kind of watch it
                // was made for
                const theirScope = parsed.scope ?? settings.scope;
                if (theirScope !== undefined && theirScope !== scope) {
                    return {
                        success: false,
                        error: 'This theme was made for a different kind of watch.',
                    };
                }

                const themeSettings = kind.normalize(settings, capabilities);
                if (!themeSettings) {
                    return {
                        success: false,
                        error: 'No valid theme settings found in the pasted JSON.',
                    };
                }

                const theme = addTheme(themeSettings, parsed.name);
                return { success: true, theme };
            } catch {
                return { success: false, error: 'Invalid JSON. Please check and try again.' };
            }
        },
        [addTheme, kind, capabilities, scope]
    );

    const exportTheme = useCallback(
        (settings: ThemeSettings): string => {
            const themeSettings = kind.normalize({ ...settings }, capabilities) ?? {};
            return JSON.stringify(
                scope !== undefined ? { scope, ...themeSettings } : themeSettings,
                null,
                2
            );
        },
        [kind, capabilities, scope]
    );

    return { savedThemes, saveTheme, deleteTheme, importTheme, exportTheme };
}

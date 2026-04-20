import React, { useState, useEffect, useRef, useCallback } from 'react';

interface NominatimResult {
    display_name: string;
    lat: string;
    lon: string;
}

interface WeatherLocationPickerProps {
    currentLoc: string;
    onSelect: (loc: string, lat: string, lng: string) => void;
}

export const WeatherLocationPicker: React.FC<WeatherLocationPickerProps> = ({
    currentLoc,
    onSelect,
}) => {
    const [query, setQuery] = useState(currentLoc);
    const [results, setResults] = useState<NominatimResult[]>([]);
    const [isLoading, setIsLoading] = useState(false);
    const [showDropdown, setShowDropdown] = useState(false);
    const debounceTimer = useRef<ReturnType<typeof setTimeout> | null>(null);
    const containerRef = useRef<HTMLDivElement>(null);

    const search = useCallback(async (q: string) => {
        if (q.length < 3) {
            setResults([]);
            setShowDropdown(false);
            return;
        }
        setIsLoading(true);
        try {
            const url = `https://nominatim.openstreetmap.org/search?format=json&limit=5&q=${encodeURIComponent(q)}`;
            const res = await fetch(url, { headers: { Accept: 'application/json' } });
            const data: NominatimResult[] = await res.json();
            setResults(data);
            setShowDropdown(data.length > 0);
        } catch {
            setResults([]);
        } finally {
            setIsLoading(false);
        }
    }, []);

    const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        const value = e.target.value;
        setQuery(value);
        if (debounceTimer.current) clearTimeout(debounceTimer.current);
        debounceTimer.current = setTimeout(() => search(value), 300);
    };

    const handleSelect = (result: NominatimResult) => {
        setQuery(result.display_name);
        setShowDropdown(false);
        setResults([]);
        onSelect(result.display_name, result.lat, result.lon);
    };

    // Close dropdown when clicking outside
    useEffect(() => {
        const handler = (e: MouseEvent) => {
            if (containerRef.current && !containerRef.current.contains(e.target as Node)) {
                setShowDropdown(false);
            }
        };
        document.addEventListener('mousedown', handler);
        return () => document.removeEventListener('mousedown', handler);
    }, []);

    return (
        <div className="halite-item halite-location-picker" ref={containerRef}>
            <label className="halite-item-label">Location</label>
            <div className="halite-location-input-wrap">
                <input
                    type="text"
                    className="halite-location-input"
                    value={query}
                    onChange={handleChange}
                    onFocus={() => results.length > 0 && setShowDropdown(true)}
                    placeholder="Search for a city…"
                    autoComplete="off"
                />
                {isLoading && <span className="halite-location-loading">…</span>}
                {showDropdown && (
                    <ul className="halite-location-results">
                        {results.map((r, i) => (
                            <li
                                key={i}
                                className="halite-location-result"
                                onMouseDown={() => handleSelect(r)}
                            >
                                {r.display_name}
                            </li>
                        ))}
                    </ul>
                )}
            </div>
        </div>
    );
};

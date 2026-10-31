import React from 'react';
import { useConfig } from '../context/PebbleConfigContext';
import { FormItemLabel } from './FormItem';
import { Icon, ICONS } from './Icon';
import { Section } from './Section';
import { Toggle } from './Toggle';

const GEOCODING_URL = 'https://geocoding-api.open-meteo.com/v1/search';
const MAX_RESULTS = 6;
const MIN_QUERY_LENGTH = 2;
const SEARCH_DEBOUNCE_MS = 400;

interface GeocodingResult {
  id: number;
  name: string;
  latitude: number;
  longitude: number;
  admin1?: string;
  country?: string;
}

const formatResultName = (result: GeocodingResult) =>
  [result.name, result.admin1, result.country]
    .filter((part, i, parts) => part && parts.indexOf(part) === i)
    .join(', ');

const formatCoords = (lat: number, lng: number) =>
  `${Math.abs(lat).toFixed(2)}°${lat >= 0 ? 'N' : 'S'}, ${Math.abs(lng).toFixed(2)}°${lng >= 0 ? 'E' : 'W'}`;

export const LocationSection: React.FC = () => {
  const { settings, updateSettings } = useConfig();
  const hasManualLocation = !!settings.locationName;

  const [isChanging, setIsChanging] = React.useState(false);
  const [query, setQuery] = React.useState('');
  const [results, setResults] = React.useState<GeocodingResult[] | null>(null);
  const [status, setStatus] = React.useState<'idle' | 'loading' | 'error'>('idle');
  const inputId = React.useId();

  const showSearch = !hasManualLocation || isChanging;

  const resetSearch = () => {
    setIsChanging(false);
    setResults(null);
    setQuery('');
    setStatus('idle');
  };

  React.useEffect(() => {
    const name = query.trim();
    if (name.length < MIN_QUERY_LENGTH) {
      setResults(null);
      setStatus('idle');
      return;
    }

    setStatus('loading');
    const controller = new AbortController();
    const timer = setTimeout(async () => {
      try {
        const url = `${GEOCODING_URL}?name=${encodeURIComponent(name)}&count=${MAX_RESULTS}&format=json`;
        const response = await fetch(url, { signal: controller.signal });
        if (!response.ok) throw new Error(`status ${response.status}`);
        const data = await response.json();
        setResults(data.results ?? []);
        setStatus('idle');
      } catch {
        if (controller.signal.aborted) return;
        setResults(null);
        setStatus('error');
      }
    }, SEARCH_DEBOUNCE_MS);

    return () => {
      clearTimeout(timer);
      controller.abort();
    };
  }, [query]);

  const selectResult = (result: GeocodingResult) => {
    updateSettings({
      locationName: formatResultName(result),
      locationLat: result.latitude,
      locationLng: result.longitude,
    });
    resetSearch();
  };

  return (
    <Section title="Location">
      <Toggle
        label="Automatic location"
        description="Used for weather and sunrise/sunset times"
        messageKey="useGpsLocation"
      />
      {!settings.useGpsLocation && !showSearch && (
        <div className="halite-item">
          <FormItemLabel
            label={settings.locationName}
            description={formatCoords(settings.locationLat, settings.locationLng)}
          />
          <button
            type="button"
            className="halite-custom-action-btn halite-icon-btn"
            onClick={() => setIsChanging(true)}
            aria-label="Change location"
            title="Change location"
          >
            <Icon path={ICONS.EDIT} />
          </button>
        </div>
      )}
      {!settings.useGpsLocation && showSearch && (
        <div className="halite-item halite-location-search">
          <label htmlFor={inputId} className="halite-label-group">
            <span>Manual Location</span>
            {!hasManualLocation && (
              <span className="halite-description">
                Please select a city
              </span>
            )}
          </label>
          <div className="halite-location-search-row">
            <input
              id={inputId}
              className="halite-input"
              type="search"
              value={query}
              placeholder="Search for a city"
              autoFocus={isChanging}
              onChange={(event) => setQuery(event.target.value)}
            />
            {isChanging && (
              <button type="button" className="halite-custom-action-btn" onClick={resetSearch}>
                Cancel
              </button>
            )}
          </div>
          {status === 'loading' && <span className="halite-description">Searching…</span>}
          {status === 'error' && (
            <span className="halite-description">Search failed. Check your connection and try again.</span>
          )}
          {results && results.length === 0 && (
            <span className="halite-description">No matching places found.</span>
          )}
          {results && results.length > 0 && (
            <ul className="halite-location-results">
              {results.map((result) => (
                <li key={result.id}>
                  <button type="button" onClick={() => selectResult(result)}>
                    <span>{formatResultName(result)}</span>
                    <span className="halite-description">
                      {formatCoords(result.latitude, result.longitude)}
                    </span>
                  </button>
                </li>
              ))}
            </ul>
          )}
        </div>
      )}
    </Section>
  );
};

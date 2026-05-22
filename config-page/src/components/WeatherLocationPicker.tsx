import React from 'react';
import {
  Autocomplete,
  SearchField,
  Input,
  ListBox,
  ListBoxItem,
  useAsyncList,
} from 'react-aria-components';

interface NominatimResult {
  place_id: number;
  display_name: string;
  lat: string;
  lon: string;
}

interface WeatherLocationPickerProps {
  currentLoc: string;
  onSelect: (loc: string, lat: string, lng: string) => void;
  onClear: () => void;
}

export const WeatherLocationPicker: React.FC<WeatherLocationPickerProps> = ({
  currentLoc,
  onSelect,
  onClear,
}) => {
  const [selectedLocation, setSelectedLocation] = React.useState(currentLoc);
  const justSelectedRef = React.useRef(false);

  const list = useAsyncList<NominatimResult>({
    async load({ signal, filterText }) {
      if (!filterText?.trim() || filterText.trim().length < 3 || justSelectedRef.current) {
        justSelectedRef.current = false;
        return { items: [] };
      }

      await new Promise((resolve) => setTimeout(resolve, 300));
      if (signal.aborted) return { items: [] };

      try {
        const res = await fetch(
          `https://nominatim.openstreetmap.org/search?q=${encodeURIComponent(filterText)}&format=json&limit=5&addressdetails=1&accept-language=en`,
          { signal, headers: { Accept: 'application/json' } },
        );
        const data: NominatimResult[] = await res.json();
        return { items: data };
      } catch {
        return { items: [] };
      }
    },
  });

  React.useEffect(() => {
    setSelectedLocation(currentLoc);
  }, [currentLoc]);

  const handleSelect = (key: React.Key) => {
    const result = list.items.find((item) => String(item.place_id) === String(key));
    if (!result) return;

    const locationLabel = result.display_name;
    setSelectedLocation(locationLabel);
    justSelectedRef.current = true;
    list.setFilterText('');
    onSelect(locationLabel, result.lat, result.lon);
  };

  const handleClear = () => {
    setSelectedLocation('');
    list.setFilterText('');
    onClear();
  };

  const showSuggestions = (list.isLoading || list.items.length > 0) && list.filterText.trim().length >= 3;

  return (
    <div className="halite-item halite-location-picker">
      <label className="halite-item-label">Location</label>
      {selectedLocation ? (
        <div className="halite-location-confirmed">
          <span>{selectedLocation}</span>
          <button
            type="button"
            className="halite-location-clear"
            aria-label="Clear location"
            onClick={handleClear}
          >
            ×
          </button>
        </div>
      ) : (
        <div className="halite-location-combobox">
          <Autocomplete inputValue={list.filterText} onInputChange={list.setFilterText}>
            <SearchField aria-label="Search for a location" className="halite-location-search-field">
              <Input className="halite-input" placeholder="Search for a city…" autoComplete="off" />
            </SearchField>
            <ListBox
              items={list.items}
              aria-label="Location suggestions"
              onAction={handleSelect}
              data-open={showSuggestions || undefined}
              className="halite-combobox-popover halite-combobox-listbox"
              renderEmptyState={() =>
                list.isLoading ? (
                  <span className="halite-combobox-item halite-combobox-item-status">Searching…</span>
                ) : null
              }
            >
              {(item) => (
                <ListBoxItem
                  id={String(item.place_id)}
                  textValue={item.display_name}
                  className="halite-combobox-item"
                >
                  {item.display_name}
                </ListBoxItem>
              )}
            </ListBox>
          </Autocomplete>
        </div>
      )}
    </div>
  );
};

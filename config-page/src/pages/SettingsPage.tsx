import React from 'react';
import { useConfig, useCapabilities } from '../context/PebbleConfigContext';
import {
  Page,
  Section,
  Toggle,
  ColorPicker,
  Select,
  SelectCards,
  ThemePicker,
  CustomThemePanel,
  WidgetSelector,
  WeatherLocationPicker,
  FormItem,
} from '../components';
import { useSavedThemes } from '../hooks/useSavedThemes';
import lightThemes from '../data/light-themes.json';
import darkThemes from '../data/dark-themes.json';
import lightThemesBw from '../data/light-themes-bw.json';
import darkThemesBw from '../data/dark-themes-bw.json';
import { prepareThemes } from '../utils/themeUtils';

// Alt-clock offset options: -12 to +14
const ALT_CLOCK_OFFSETS = Array.from({ length: 27 }, (_, i) => {
  const offset = i - 12;
  const label = offset === 0 ? 'UTC±0' : offset > 0 ? `UTC+${offset}` : `UTC${offset}`;
  return { label, value: offset };
});

const LANGUAGE_OPTIONS = [
  { label: 'English', value: 0 },
  { label: 'Français', value: 1 },
  { label: 'Deutsch', value: 2 },
  { label: 'Español', value: 3 },
  { label: 'Italiano', value: 4 },
  { label: 'Nederlands', value: 5 },
  { label: 'Türkçe', value: 6 },
  { label: 'Čeština', value: 7 },
  { label: 'Português', value: 8 },
  { label: 'Ελληνικά', value: 9 },
  { label: 'Svenska', value: 10 },
  { label: 'Polski', value: 11 },
  { label: 'Slovenčina', value: 12 },
  { label: 'Tiếng Việt', value: 13 },
  { label: 'Română', value: 14 },
  { label: 'Català', value: 15 },
  { label: 'Norsk', value: 16 },
  { label: 'Русский', value: 17 },
  { label: 'Eesti', value: 18 },
  { label: 'Euskara', value: 19 },
  { label: 'Suomi', value: 20 },
  { label: 'Dansk', value: 21 },
  { label: 'Lietuvių', value: 22 },
  { label: 'Slovenščina', value: 23 },
  { label: 'Magyar', value: 24 },
  { label: 'Hrvatski', value: 25 },
  { label: 'Gaeilge', value: 26 },
  { label: 'Latviešu', value: 27 },
  { label: 'Srpski', value: 28 },
  { label: '中文', value: 29 },
  { label: 'Bahasa Indonesia', value: 30 },
  { label: 'Українська', value: 31 },
  { label: 'Cymraeg', value: 32 },
  { label: 'Galego', value: 33 },
  { label: '日本語', value: 34 },
  { label: '한국어', value: 35 },
  { label: 'עברית', value: 36 },
];

export const SettingsPage: React.FC = () => {
  const { settings, updateSetting } = useConfig();
  const capabilities = useCapabilities();

  const activeThemes = React.useMemo(() =>
    capabilities.BW
      ? prepareThemes(lightThemesBw, darkThemesBw, false)
      : prepareThemes(lightThemes, darkThemes, false),
    [capabilities.BW]
  );

  const activeNightThemes = React.useMemo(() =>
    capabilities.BW
      ? prepareThemes(darkThemesBw, lightThemesBw, true)
      : prepareThemes(darkThemes, lightThemes, true),
    [capabilities.BW]
  );

  const daySavedThemes = useSavedThemes(false);
  const nightSavedThemes = useSavedThemes(true);

  const getDaySavedTheme = () =>
    daySavedThemes.savedThemes.find((t) => t.id === settings.SETTING_THEME);

  const getNightSavedTheme = () =>
    nightSavedThemes.savedThemes.find((t) => t.id === settings.SETTING_NIGHT_THEME);

  const showWeatherSection =
    settings.widget_0_id === 7 || settings.widget_0_id === 8 || settings.widget_0_id === 13 ||
    settings.widget_1_id === 7 || settings.widget_1_id === 8 || settings.widget_1_id === 13 ||
    settings.widget_2_id === 7 || settings.widget_2_id === 8 || settings.widget_2_id === 13;

  const showAltClockSection =
    settings.widget_0_id === 3 || settings.widget_1_id === 3 || settings.widget_2_id === 3;

  const showBatterySection =
    settings.widget_0_id === 2 || settings.widget_1_id === 2 || settings.widget_2_id === 2;

  return (
    <Page title="TimeStyle Settings">
      {/* ── Theme ── */}
      <Section title="Theme">
        <ThemePicker
          messageKey="SETTING_THEME"
          themes={activeThemes}
          savedThemes={daySavedThemes.savedThemes}
          label="Theme preset"
        />
        <CustomThemePanel
          themeId={settings.SETTING_THEME}
          savedTheme={getDaySavedTheme()}
          onSave={daySavedThemes.saveTheme}
          onDelete={daySavedThemes.deleteTheme}
          onImport={daySavedThemes.importTheme}
          onExport={daySavedThemes.exportTheme}
          onSelectTheme={(id) => updateSetting('SETTING_THEME', id)}
        />
        {settings.SETTING_THEME === 'custom' && (
          <>
            <ColorPicker
              label="Background color"
              messageKey="color_bg"
            />
            <ColorPicker
              label="Time color"
              messageKey="color_time"
              bwAllowGrey={false}
            />
            <ColorPicker
              label="Sidebar color"
              messageKey="color_sidebar"
              bwAllowGrey={false}
            />
            <ColorPicker
              label="Sidebar text color"
              messageKey="sidebar_text_color"
              bwAllowGrey={false}
            />
          </>
        )}
      </Section>

      {/* ── Night Theme (commented out — TimeStyle does not yet support night themes) ──
      <Section title="Night Theme">
        <Toggle
          label="Enable night theme"
          description="If enabled, the selected color scheme will be shown after sunset."
          messageKey="SETTING_USE_NIGHT_THEME"
        />
        {settings.SETTING_USE_NIGHT_THEME === 1 && (
          <>
            <ThemePicker
              messageKey="SETTING_NIGHT_THEME"
              label="Night theme preset"
              themes={activeNightThemes}
              watchPreviewProps={{ isNight: true }}
              savedThemes={nightSavedThemes.savedThemes}
            />
            <CustomThemePanel
              themeId={settings.SETTING_NIGHT_THEME}
              savedTheme={getNightSavedTheme()}
              watchPreviewProps={{ isNight: true }}
              onSave={nightSavedThemes.saveTheme}
              onDelete={nightSavedThemes.deleteTheme}
              onImport={nightSavedThemes.importTheme}
              onExport={nightSavedThemes.exportTheme}
              onSelectTheme={(id) => updateSetting('SETTING_NIGHT_THEME', id)}
            />
          </>
        )}
      </Section>
      ── */}

      {/* ── Widgets ── */}
      <Section title="Widgets">
        <WidgetSelector />
      </Section>

      {/* ── Sidebar Display ── */}
      <Section title="Sidebar Display">
        {capabilities.RECT && (
          <SelectCards
            label="Position"
            messageKey="sidebar_position"
            options={[
              { label: 'Left', value: 'left' },
              { label: 'Right', value: 'right' },
            ]}
            optionSettings={{
              right: { sidebar_position: 'right' },
              left: { sidebar_position: 'left' },
            }}
          />
        )}
        <Toggle
          label="Use large sidebar text"
          description="Increases the size of widget text in the sidebar"
          messageKey="use_large_sidebar_font_setting"
        />
      </Section>

      {/* ── Time Display ── */}
      <Section title="Time Display">
        <Toggle
          label="Show leading zero"
          description="Display time as 09:30 instead of 9:30"
          messageKey="leading_zero_setting"
        />
        <SelectCards
          label="Clock font"
          messageKey="clock_font_setting"
          options={[
            { label: 'Default', value: 'default' },
            { label: 'Bold', value: 'bold' },
            { label: 'Bold (hours only)', value: 'bold-h' },
            { label: 'Bold (minutes only)', value: 'bold-m' },
            { label: 'LECO', value: 'leco' },
          ]}
          optionSettings={{
            default: { clock_font_setting: 'default' },
            bold: { clock_font_setting: 'bold' },
            'bold-h': { clock_font_setting: 'bold-h' },
            'bold-m': { clock_font_setting: 'bold-m' },
            leco: { clock_font_setting: 'leco' },
          }}
        />
      </Section>

      {/* ── Bluetooth ── */}
      <Section title="Bluetooth">
        <Toggle
          label="Show disconnection icon"
          description="Display an icon when the watch is disconnected from your phone"
          messageKey="disconnect_icon_setting"
        />
        <Toggle
          label="Vibrate on disconnect"
          messageKey="bluetooth_vibe_setting"
        />
      </Section>

      {/* ── Notifications ── */}
      <Section title="Notifications">
        <Select
          label="Hourly vibration"
          messageKey="hourly_vibe_setting"
          options={[
            { label: 'None', value: 'none' },
            { label: 'Every 30 minutes', value: 'everyHalfHour' },
            { label: 'Every hour', value: 'everyHour' },
          ]}
        />
      </Section>

      {/* ── Weather ── */}
      {showWeatherSection && (
        <Section title="Weather">
          <Select
            label="Location mode"
            messageKey="weather_setting"
            options={[
              { label: 'Automatic (GPS)', value: 'auto' },
              { label: 'Manual', value: 'manual' },
            ]}
          />
          {settings.weather_setting === 'manual' && (
            <WeatherLocationPicker
              currentLoc={settings.weather_loc}
              onSelect={(loc, lat, lng) => {
                updateSetting('weather_loc', loc);
                updateSetting('weather_loc_lat', lat);
                updateSetting('weather_loc_lng', lng);
              }}
            />
          )}
          <Select
            label="Temperature units"
            messageKey="units"
            options={[
              { label: 'Fahrenheit (°F)', value: 'f' },
              { label: 'Celsius (°C)', value: 'c' },
            ]}
          />
          <FormItem label="" description="Weather data provided by Open-Meteo (open-meteo.com)" className="halite-info">
            <span />
          </FormItem>
        </Section>
      )}

      {/* ── Battery ── */}
      {showBatterySection && (
        <Section title="Battery">
          <Select
            label="Auto battery display"
            messageKey="autobattery_setting"
            description="Automatically show battery in the sidebar when it drops to 10%"
            options={[
              { label: 'Show automatically at 10%', value: 'on' },
              { label: 'Never show automatically', value: 'off' },
            ]}
          />
          <Select
            label="Battery meter style"
            messageKey="battery_meter_setting"
            options={[
              { label: 'Icon & percent', value: 'icon-and-percent' },
              { label: 'Icon only', value: 'icon-only' },
            ]}
          />
        </Section>
      )}

      {/* ── Alternate Time Zone ── */}
      {showAltClockSection && (
        <Section title="Alternate Time Zone">
          <Select
            label="Time zone offset"
            messageKey="altclock_offset"
            description="Note: does not account for Daylight Saving Time"
            options={ALT_CLOCK_OFFSETS}
          />
          <FormItem label="Display name" description="Up to 4 characters shown above the time (e.g. NYC, LON)" className="halite-select">
            <input
              type="text"
              className="halite-text-input"
              value={settings.altclock_name}
              maxLength={4}
              onChange={(e) => updateSetting('altclock_name', e.target.value.toUpperCase())}
            />
          </FormItem>
        </Section>
      )}

      {/* ── Health ── */}
      {capabilities.HEALTH && (
        <Section title="Health">
          <Select
            label="Steps widget displays"
            messageKey="health_use_distance"
            options={[
              { label: 'Number of steps', value: 'steps' },
              { label: 'Distance walked', value: 'distance' },
            ]}
          />
          <Select
            label="Sleep widget uses"
            messageKey="health_use_restful_sleep"
            options={[
              { label: 'Total sleep time', value: 'total' },
              { label: 'Restful sleep only', value: 'restful' },
            ]}
          />
        </Section>
      )}

      {/* ── Regional ── */}
      <Section title="Regional">
        <Select
          label="Language"
          messageKey="language_id"
          options={LANGUAGE_OPTIONS}
        />
        <Select
          label="Decimal separator"
          messageKey="decimal_separator"
          options={[
            { label: 'Period  (1,234.5)', value: '.' },
            { label: 'Comma  (1.234,5)', value: ',' },
          ]}
        />
      </Section>
    </Page>
  );
};

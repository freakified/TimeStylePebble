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
  DonationLink,
  TextField,
} from '../components';
import { useSavedThemes } from '../hooks/useSavedThemes';
import lightThemes from '../data/light-themes.json';
import darkThemes from '../data/dark-themes.json';
import lightThemesBw from '../data/light-themes-bw.json';
import darkThemesBw from '../data/dark-themes-bw.json';
import { prepareThemes } from '../utils/themeUtils';
import { layoutHasSidebar, layoutHasBottomBar, layoutHasStackedBar } from '../utils/layout';
import { WORLD_TIME_CITY_OPTIONS } from '../data/cities';

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

  // Only the slots the current platform + layout actually renders
  const sidebarWidgetIds = capabilities.ROUND
    ? [settings.widget0Id, settings.widget1Id]
    : layoutHasSidebar(settings.layout)
      ? capabilities.EMERY
        ? [settings.widget0Id, settings.widget1Id, settings.widget2Id,
           settings.widget3Id, settings.widget4Id, settings.widget5Id]
        : [settings.widget0Id, settings.widget1Id, settings.widget2Id,
           settings.widget3Id]
      : [];
  const bottomBarWidgetIds = !capabilities.ROUND && layoutHasBottomBar(settings.layout)
    ? [
        settings.bottomBarWidget0Id, settings.bottomBarWidget1Id,
        settings.bottomBarWidget2Id, settings.bottomBarWidget3Id,
      ].slice(0, layoutHasStackedBar(settings.layout) ? 4 : capabilities.EMERY ? 3 : 2)
    : [];
  const activeWidgetIds = sidebarWidgetIds.concat(bottomBarWidgetIds);

  const showWeatherSection = activeWidgetIds.some((id) => [7, 8, 13, 15, 16, 17].includes(id));

  const showWorldTime1Section = activeWidgetIds.some((id) => id === 3);
  const showWorldTime2Section = activeWidgetIds.some((id) => id === 14);

  const showHealthSection = activeWidgetIds.some((id) => id === 9 || id === 10);


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
              messageKey="colorBackground"
            />
            <ColorPicker
              label="Time color"
              messageKey="colorTime"
              bwAllowGrey={false}
            />
            <ColorPicker
              label="Sidebar color"
              messageKey="colorSidebar"
              bwAllowGrey={false}
            />
            <ColorPicker
              label="Sidebar text color"
              messageKey="colorSidebarText"
              bwAllowGrey={false}
            />
            <ColorPicker
              label="Alt sidebar text color"
              messageKey="colorAltSidebarText"
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

      {/* ── Layout ── */}
      <Section title="Layout">
        {capabilities.RECT && (
          <SelectCards
            label="Layout"
            messageKey="layout"
            options={[
              { label: 'Left sidebar', value: 'left-sidebar' },
              { label: 'Right sidebar', value: 'right-sidebar' },
              { label: 'Left sidebar + bottom bar', value: 'left-sidebar-bottom-bar' },
              { label: 'Right sidebar + bottom bar', value: 'right-sidebar-bottom-bar' },
              { label: 'Bottom bar only', value: 'bottom-bar-only' },
            ]}
            optionSettings={{
              'left-sidebar': { layout: 'left-sidebar' },
              'right-sidebar': { layout: 'right-sidebar' },
              'left-sidebar-bottom-bar': { layout: 'left-sidebar-bottom-bar' },
              'right-sidebar-bottom-bar': { layout: 'right-sidebar-bottom-bar' },
              'bottom-bar-only': { layout: 'bottom-bar-only' },
            }}
          />
        )}
        {capabilities.RECT && (
          <SelectCards
            label="Sidebar"
            messageKey="seamlessSidebar"
            options={[
              { label: 'Standard', value: false },
              { label: 'Seamless', value: true },
            ]}
            optionSettings={{
              false: { seamlessSidebar: false, widget0Id: 10, widget1Id: 0, widget2Id: 4 },
              true: { seamlessSidebar: true, widget0Id: 10, widget1Id: 0, widget2Id: 4 },
            }}
          />
        )}
        <Toggle
          label="Use large sidebar text"
          description="Increases the size of widget text in the sidebar"
          messageKey="largeSidebarFont"
        />
      </Section>

      {/* ── Widgets ── */}
      <Section title="Widgets">
        <WidgetSelector />
      </Section>


      {/* ── Time Display ── */}
      <Section title="Time Display">
        <SelectCards
          label="Font"
          messageKey="clockFont"
          options={[
            { label: 'Default', value: 'default' },
            { label: 'Bold', value: 'bold' },
            { label: 'Bold (hours only)', value: 'bold-h' },
            { label: 'Bold (minutes only)', value: 'bold-m' },
            { label: 'LECO', value: 'leco' },
          ]}
          optionSettings={{
            default: { clockFont: 'default' },
            bold: { clockFont: 'bold' },
            'bold-h': { clockFont: 'bold-h' },
            'bold-m': { clockFont: 'bold-m' },
            leco: { clockFont: 'leco' },
          }}
        />
        <Toggle
          label="Show leading zero"
          description="Display time as 09:30 instead of 9:30"
          messageKey="leadingZero"
        />
      </Section>

      {/* ── Weather ── */}
      {showWeatherSection && (
        <Section title="Weather">
          <Toggle
            label="Determine location automatically"
            description="Use GPS location for weather data"
            messageKey="weatherMode"
          />
          {!settings.weatherMode && (
            <WeatherLocationPicker
              currentLoc={settings.weatherLocation}
              onSelect={(loc, lat, lng) => {
                updateSetting('weatherLocation', loc);
                updateSetting('weatherLocationLat', lat);
                updateSetting('weatherLocationLng', lng);
              }}
              onClear={() => {
                updateSetting('weatherLocation', '');
                updateSetting('weatherLocationLat', '');
                updateSetting('weatherLocationLng', '');
              }}
            />
          )}
          <Select
            label="Temperature units"
            messageKey="weatherUnits"
            options={[
              { label: 'Fahrenheit (°F)', value: 'f' },
              { label: 'Celsius (°C)', value: 'c' },
            ]}
          />
          Weather data provided by Open-Meteo (open-meteo.com)
        </Section>
      )}

      {/* ── World Time ── */}
      {showWorldTime1Section && (
        <Section title="World Time 1">
          <Select
            label="City"
            messageKey="worldTime1City"
            description="Daylight Saving Time is handled automatically"
            options={WORLD_TIME_CITY_OPTIONS}
          />
          <TextField
            label="Custom label"
            description="Up to 4 characters shown above the time (leave blank for no label)"
            messageKey="worldTime1Label"
            maxLength={4}
            transformValue={(value) => value.toUpperCase()}
          />
        </Section>
      )}
      {showWorldTime2Section && (
        <Section title="World Time 2">
          <Select
            label="City"
            messageKey="worldTime2City"
            description="Daylight Saving Time is handled automatically"
            options={WORLD_TIME_CITY_OPTIONS}
          />
          <TextField
            label="Custom label"
            description="Up to 4 characters shown above the time (leave blank for no label)"
            messageKey="worldTime2Label"
            maxLength={4}
            transformValue={(value) => value.toUpperCase()}
          />
        </Section>
      )}

      {/* ── Health ── */}
      {capabilities.HEALTH && showHealthSection && (
        <Section title="Health">
          <Select
            label="Steps widget displays"
            messageKey="healthActivityMetric"
            options={[
              { label: 'Number of steps', value: 'steps' },
              { label: 'Distance walked', value: 'distance' },
            ]}
          />
          <Select
            label="Sleep widget uses"
            messageKey="healthSleepType"
            options={[
              { label: 'Total sleep time', value: 'total' },
              { label: 'Restful sleep only', value: 'restful' },
            ]}
          />
        </Section>
      )}

      <Section title="Status Icon">
        <Toggle
          label="Show quiet time and watch disconnected status icon"
          description="Show a small icon at the top of the screen during Quiet Time or when your phone is disconnected"
          messageKey="statusBarEnabled"
        />
        {settings.statusBarEnabled && (
          <Toggle
            label="Always show connection status"
            description="Also show the icon while connected to your phone, not just when disconnected"
            messageKey="statusBarShowConnection"
          />
        )}
      </Section>

      <Section title="Vibration">
        <Toggle
          label="Vibrate on bluetooth disconnection"
          description="Vibrate if the watch loses its connection to your phone"
          messageKey="bluetoothVibe"
        />
        <Select
          label="Vibrate periodically"
          description="Vibrate every hour or half hour"
          messageKey="hourlyVibe"
          options={[
            { label: 'None', value: 'none' },
            { label: 'Every 30 minutes', value: 'everyHalfHour' },
            { label: 'Every hour', value: 'everyHour' },
          ]}
        />
      </Section>

      {/* ── Regional ── */}
      <Section title="Regional">
        <Select
          label="Language"
          description="Controls the language used in widgets"
          messageKey="languageId"
          options={LANGUAGE_OPTIONS}
        />
        <Select
          label="Decimal separator"
          messageKey="decimalSeparator"
          options={[
            { label: 'Period  (1,234.5)', value: '.' },
            { label: 'Comma  (1.234,5)', value: ',' },
          ]}
        />
      </Section>
      <DonationLink />
    </Page>
  );
};

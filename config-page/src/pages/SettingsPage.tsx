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
  WidgetDisplayOptions,
  WeatherLocationPicker,
  FormItem,
  DonationLink,
  TextField,
} from '../components';
import { useSavedThemes } from '../hooks/useSavedThemes';
import { COLOR_THEMES, WIDGET_THEMES } from '../utils/themeKinds';
import { barSlotCap } from '../utils/layout';
import { Layout } from '../context/types';
import { worldTimeOptions, withWorldTimeValue, worldTimeDefaultLabel } from '../data/worldTimes';
import { WIDGET_PARTS, hiddenParts } from '../utils/widgetParts';

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
  const { settings, updateSetting, updateSettings } = useConfig();
  const capabilities = useCapabilities();

  const activeThemes = React.useMemo(
    () => COLOR_THEMES.presets(capabilities),
    [capabilities]
  );

  const daySavedThemes = useSavedThemes(COLOR_THEMES, capabilities);

  const getDaySavedTheme = () =>
    daySavedThemes.savedThemes.find((t) => t.id === settings.SETTING_THEME);

  const widgetThemes = React.useMemo(
    () => WIDGET_THEMES.presets(capabilities),
    [capabilities]
  );

  const savedWidgetThemes = useSavedThemes(WIDGET_THEMES, capabilities);

  const getSavedWidgetTheme = () =>
    savedWidgetThemes.savedThemes.find((t) => t.id === settings.SETTING_WIDGET_THEME);

  // Only the slots the current platform actually renders. Round uses all
  // six: its two arcs split the list between them.
  const sidebarWidgetIds = capabilities.ROUND
    ? [settings.widget0Id, settings.widget1Id, settings.widget2Id,
    settings.widget3Id, settings.widget4Id, settings.widget5Id]
    : capabilities.EMERY
      ? [settings.widget0Id, settings.widget1Id, settings.widget2Id,
      settings.widget3Id, settings.widget4Id, settings.widget5Id]
      : [settings.widget0Id, settings.widget1Id, settings.widget2Id,
      settings.widget3Id];
  // the bottom bar's slots are always live: holding a widget is what turns
  // the bar on, so its widgets always count as active
  const bottomBarWidgetIds = [
    settings.bottomBarWidget0Id, settings.bottomBarWidget1Id,
    settings.bottomBarWidget2Id,
  ].slice(0, barSlotCap(capabilities));
  const activeWidgetIds = sidebarWidgetIds.concat(bottomBarWidgetIds);

  // the layout picks which side the accent-colored band sits on and
  // nothing else. On rect that places the sidebar (whether there's a
  // bottom bar follows from the bar's own widgets); on round both arcs
  // are always there, so it picks which of them is filled.
  const layoutOptions: { label: string; value: Layout }[] = capabilities.ROUND
    ? [
      { label: 'Color left arc', value: 'left-sidebar' },
      { label: 'Color right arc', value: 'right-sidebar' },
    ]
    : [
      { label: 'Left sidebar', value: 'left-sidebar' },
      { label: 'Right sidebar', value: 'right-sidebar' },
    ];
  const layoutOptionSettings: Record<string, { layout: Layout }> = {};
  layoutOptions.forEach((option) => {
    layoutOptionSettings[option.value] = { layout: option.value };
  });

  const showWeatherSection = activeWidgetIds.some((id) => [7, 8, 13, 15, 17].includes(id));

  const showWorldTime1Section = activeWidgetIds.some((id) => id === 3);
  const showWorldTime2Section = activeWidgetIds.some((id) => id === 14);
  // the custom label only matters while the widget shows its label
  const showWorldTime1Label = !hiddenParts(3, settings).marker;
  const showWorldTime2Label = !hiddenParts(14, settings).marker;
  // picking a zone relabels the widget with its default label, which the
  // user can then edit; a blank label still falls back to it on the phone
  const worldTimeZoneOptions = React.useMemo(() => worldTimeOptions(), []);

  const showHealthSection = activeWidgetIds.some((id) => id === 9);

  // placed widgets that have parts to hide, each listed once
  const displayWidgetIds = activeWidgetIds.filter(
    (id, i) => WIDGET_PARTS[id] && activeWidgetIds.indexOf(id) === i
  );


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
            />
            <ColorPicker
              label="Sidebar color"
              messageKey="colorSidebar"
            />
          </>
        )}
      </Section>

      {/* ── Time Display ── */}
      <Section title="Time Display">
        <SelectCards
          label="Font"
          messageKey="clockFont"
          options={[
            { label: 'Default', value: 'avenir' },
            { label: 'LECO', value: 'leco' },
            { label: 'Serif', value: 'garamond' },
          ]}
          optionSettings={{
            avenir: { clockFont: 'avenir' },
            leco: { clockFont: 'leco' },
            garamond: { clockFont: 'garamond' },
          }}
        />
        <SelectCards
          label="Font weight"
          options={[
            { label: 'Regular', value: 'regular' },
            { label: 'Bold', value: 'bold' },
            { label: 'Bold hours', value: 'bold-hours' },
            { label: 'Bold minutes', value: 'bold-minutes' },
          ]}
          optionSettings={{
            regular: { boldHours: false, boldMinutes: false },
            bold: { boldHours: true, boldMinutes: true },
            'bold-hours': { boldHours: true, boldMinutes: false },
            'bold-minutes': { boldHours: false, boldMinutes: true },
          }}
        />
        <Toggle
          label="Show leading zero"
          description="Display time as 09:30 instead of 9:30"
          messageKey="leadingZero"
        />
      </Section>

      {/* ── Layout ── */}
      <Section title="Layout">
        <SelectCards
          label="Layout"
          messageKey="layout"
          options={layoutOptions}
          optionSettings={layoutOptionSettings}
        />
        {capabilities.ROUND && (
          <Toggle
            label="Show sidebar background color on both sides"
            description="Fill both arcs instead of letting one blend into the watchface"
            messageKey="colorBothArcs"
          />
        )}
        <Toggle
          label="Use large sidebar text"
          description="Increases the size of widget text in the sidebar"
          messageKey="largeSidebarFont"
          // large text needs the room: it turns the wide sidebar on, which
          // stays on (and locked) for as long as large text is
          onChange={(selected) => updateSettings(selected && capabilities.RECT
            ? { largeSidebarFont: true, wideSidebar: true }
            : { largeSidebarFont: selected })}
        />
        {capabilities.RECT && (
          <Toggle
            label="Wider sidebar"
            description={settings.largeSidebarFont
              ? "Always on with large sidebar text"
              : "Give your widgets slightly more breathing room"}
            messageKey="wideSidebar"
            isDisabled={settings.largeSidebarFont}
          />
        )}
      </Section>

      {/* ── Widgets ── */}
      <Section title="Widgets">
        <ThemePicker
          messageKey="SETTING_WIDGET_THEME"
          themes={widgetThemes}
          savedThemes={savedWidgetThemes.savedThemes}
          label="Widget preset"
        />
        <CustomThemePanel
          themeId={settings.SETTING_WIDGET_THEME}
          savedTheme={getSavedWidgetTheme()}
          onSave={savedWidgetThemes.saveTheme}
          onDelete={savedWidgetThemes.deleteTheme}
          onImport={savedWidgetThemes.importTheme}
          onExport={savedWidgetThemes.exportTheme}
          onSelectTheme={(id) => updateSetting('SETTING_WIDGET_THEME', id)}
          importTitle="Import Widgets"
          customizeIcon="EDIT"
        />
        {/* the panel above already previews the live widgets. Which parts
            each widget shows is part of the preset too, so it's edited
            alongside the widgets themselves */}
        {settings.SETTING_WIDGET_THEME === 'custom' && (
          <>
            <WidgetSelector showPreview={false} />
            {displayWidgetIds.length > 0 && (
              <>
                <h3 className="halite-subsection-title">Widget display</h3>
                <WidgetDisplayOptions widgetIds={displayWidgetIds} />
              </>
            )}
          </>
        )}
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
          <p className="halite-section-note">
            Weather data provided by Open-Meteo (open-meteo.com)
          </p>
        </Section>
      )}

      {/* ── World Time ── */}
      {showWorldTime1Section && (
        <Section title="World Time 1">
          <Select
            label="Time zone"
            messageKey="worldTime1Zone"
            description="Daylight Saving Time is handled automatically"
            options={withWorldTimeValue(worldTimeZoneOptions, settings.worldTime1Zone)}
            onValueChange={(tz) => updateSetting('worldTime1Label', worldTimeDefaultLabel(String(tz)))}
          />
          {showWorldTime1Label && (
            <TextField
              label="Custom label"
              description="Up to 4 characters shown above the time"
              messageKey="worldTime1Label"
              maxLength={4}
              placeholder={worldTimeDefaultLabel(settings.worldTime1Zone)}
              transformValue={(value) => value.toUpperCase()}
            />
          )}
        </Section>
      )}
      {showWorldTime2Section && (
        <Section title="World Time 2">
          <Select
            label="Time zone"
            messageKey="worldTime2Zone"
            description="Daylight Saving Time is handled automatically"
            options={withWorldTimeValue(worldTimeZoneOptions, settings.worldTime2Zone)}
            onValueChange={(tz) => updateSetting('worldTime2Label', worldTimeDefaultLabel(String(tz)))}
          />
          {showWorldTime2Label && (
            <TextField
              label="Custom label"
              description="Up to 4 characters shown above the time"
              messageKey="worldTime2Label"
              maxLength={4}
              placeholder={worldTimeDefaultLabel(settings.worldTime2Zone)}
              transformValue={(value) => value.toUpperCase()}
            />
          )}
        </Section>
      )}

      {/* ── Health ── */}
      {capabilities.HEALTH && showHealthSection && (
        <Section title="Health">
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

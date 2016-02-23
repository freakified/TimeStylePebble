var CURRENT_SETTINGS_VERSION = 7;

// if we have any persistent data saved, load it in
$(document).ready(function() {
  loadPreviousSettings();
  loadLastUsedColors();
  showCustomPresets();
  showCorrectedColors();
  updateSidebarPreview();

  checkVersion();
});

function loadSettingCheckbox(elementID, setting) {
  if(setting) {
    $('#' + elementID + ' label[data-setting="' + setting + '"] input').attr('checked', true);
    $('#' + elementID + ' label[data-setting="' + setting + '"]').addClass('active');
  }
}

function checkVersion() {
  var appVersion = getQueryParam('appversion');

  // if the app version is not present, or is less than the current version show the warning
  if(!appVersion || parseInt(appVersion, 10) < CURRENT_SETTINGS_VERSION) {
    $('#version_warning').removeClass('hidden');
  }
}

// populates the sidebar widgets for someone with pre-widget saved settings
function migrateLegacySettings(config) {

  // stick in the new defaults
  config.health_use_distance = 'no';
  config.health_use_restful_sleep = 'no';
  config.decimal_separator = '.';
  config.autobattery_setting = 'on';

  return config;
}

function loadPreviousSettings() {
  // load the previous settings
  var savedSettings = JSON.parse(window.localStorage.getItem('savedSettings'));

  // if savedsettings exists but doesn't contain a version flag, it must be upgraded
  if(savedSettings && !savedSettings.settings_version) {
    savedSettings = migrateLegacySettings(savedSettings);
  }

  if(!savedSettings) {
    // if there are no settings set, load the default settings
    savedSettings = {
      // color settings
      color_bg: '000000',
      color_sidebar: 'FF5500',
      color_time: 'FF5500',
      sidebar_text_color: '000000',

      // general settings
      language_id: 0, // english
      leading_zero_setting: 'no',
      clock_font_setting: 'default',

      // vibration settings
      bluetooth_vibe_setting: 'no',
      hourly_vibe_setting: 'no',

      // sidebar settings
      widget_0_id: '7', // current weather
      widget_1_id: '0', // empty
      widget_2_id: '4', // today's date
      sidebar_position: 'right',
      use_large_sidebar_font_setting: 'no',

      // weather widget settings
      units: 'f',
      weather_loc: '',
      weather_setting: 'auto',

      // battery widget settings
      battery_meter_setting: 'icon-only',
      autobattery_setting: 'on',

      // alt timezone widget settings
      altclock_name: 'ALT',
      altclock_offset: 0,

      // health widget settings
      health_use_distance: 'no',
      health_use_restful_sleep: 'no',
      decimal_separator: '.',

      // version key used for migrations
      settings_version: CURRENT_SETTINGS_VERSION
    };

    // load the color settings
    window.localStorage.setItem('time-color', savedSettings.color_time);
    window.localStorage.setItem('time-bg-color', savedSettings.color_bg);
    window.localStorage.setItem('sidebar-color', savedSettings.color_sidebar);
    window.localStorage.setItem('sidebar-text-color', savedSettings.sidebar_text_color);

  }

  // try to highlight the selected preset, if possible
  $("#preset_selector label[data-time-color='#" + savedSettings.color_time.toLowerCase() + "']" +
                          "[data-time-bg-color='#" + savedSettings.color_bg.toLowerCase() + "']" +
                          "[data-sidebar-color='#" + savedSettings.color_sidebar.toLowerCase() + "']"
  ).each(function() {
    $(this).addClass('active');
    $(this).children('input').attr('checked', true);
  });

  // load checkbox settings
  loadSettingCheckbox('sidebar_position_setting', savedSettings.sidebar_position);
  loadSettingCheckbox('units_setting', savedSettings.units);
  loadSettingCheckbox('bluetooth_vibe_setting', savedSettings.bluetooth_vibe_setting);
  loadSettingCheckbox('hourly_vibe_setting', savedSettings.hourly_vibe_setting);
  loadSettingCheckbox('battery_meter_setting', savedSettings.battery_meter_setting);
  loadSettingCheckbox('autobattery_setting', savedSettings.autobattery_setting);
  loadSettingCheckbox('time_leading_zero_setting', savedSettings.leading_zero_setting);
  loadSettingCheckbox('clock_font_setting', savedSettings.clock_font_setting);
  loadSettingCheckbox('use_large_sidebar_font_setting', savedSettings.use_large_sidebar_font_setting);
  loadSettingCheckbox('weather_setting', savedSettings.weather_setting);
  loadSettingCheckbox('decimal_separator', savedSettings.decimal_separator);
  loadSettingCheckbox('health_use_distance', savedSettings.health_use_distance);
  loadSettingCheckbox('health_use_restful_sleep', savedSettings.health_use_restful_sleep);

  // load weather location
  $('#weather_loc').val(savedSettings.weather_loc);

  if(savedSettings.weather_setting == 'manual') {
    $('#manual_weather_loc_setting_area').collapse('show');
  }

  // load language selector
  if(savedSettings.language_id !== undefined) {
    $('#language_selection option[data-setting="' + savedSettings.language_id + '"]').prop('selected', true);
  }

  // load sidebar widget settings
  $('#widget_0_selector').val(savedSettings.widget_0_id);
  $('#widget_1_selector').val(savedSettings.widget_1_id);
  $('#widget_2_selector').val(savedSettings.widget_2_id);

  // load alt timezone widget settings
  $('#altclock_name').val(savedSettings.altclock_name);
  $('#altclock_offset_hour option[data-setting="' + savedSettings.altclock_offset + '"]').prop('selected', true);
  $('#altclock_offset_hour option[data-setting="' + savedSettings.altclock_offset + '"]').prop('selected', true);

  // update the widget settings sections to only show ones that are relevant
  showOnlySelectedWidgetSettings();
}

function loadLastUsedColors() {
  // try to load each of the four colors
  var colorNames = ['time-color', 'time-bg-color', 'sidebar-color', 'sidebar-text-color'];

  for(i = 0; i < colorNames.length; i++) {
    colorName = colorNames[i];

    var storedColor = window.localStorage.getItem(colorName);

    if(storedColor) {
      $('#' + colorName).spectrum("set", storedColor);
      $('#' + colorName).data('storedColor', storedColor);
    } else  {
      $('#' + colorName).spectrum("set", null);
    }
  }

  // in case anything changed
  updateCustomPreview();
}

$('#time-color').on('change', customColorChanged);
$('#time-bg-color').on('change', customColorChanged);
$('#sidebar-color').on('change', customColorChanged);
$('#sidebar-text-color').on('change', customColorChanged);

$('label.btn').on('change', setFormHasChanges);
$('select').on('change', setFormHasChanges);
$('#weather_loc').on('input', setFormHasChanges);
$('#altclock_name').on('input', setFormHasChanges);

$('#use_large_sidebar_font_setting').on('change', updateSidebarPreview);
$('#battery_meter_setting').on('change', updateSidebarPreview);

function customColorChanged() {
  setFormHasChanges();
  updateCustomPreview();

  $('#preset_selector label.btn').removeClass('active');
}

function presetSelected() {
  $("#time-color").spectrum("set", $(this).data('time-color'));
  $("#time-bg-color").spectrum("set", $(this).data('time-bg-color'));
  $("#sidebar-color").spectrum("set", $(this).data('sidebar-color'));

  var sidebarColor = $(this).data('sidebar-text-color');

  // since the sidebar text color is almost always black, just assume that this
  // is the case
  if(sidebarColor) {
    $("#sidebar-text-color").spectrum("set", sidebarColor);
  } else {
    $("#sidebar-text-color").spectrum("set", "#000000");
  }

  setFormHasChanges();
  updateCustomPreview();
}

$('#preset_selector label.btn').on('click', presetSelected);

// handle changes to the sidebar layout dropdowns
function sidebarWidgetSelectionChanged() {
  var newSelection = $(this).val();

  // find out what the other selections were
  var otherSelections;
  var changedID = $(this).attr('id');

  var otherSelections = $('#sidebar_layout_section select:not(#' + changedID + ')');

  // when a widget is selected, make sure that no other dropdowns also contain
  // that widget. if one does, set it to "none"
  if(newSelection != 0) {
    for (var i = 0; i < otherSelections.length; i++) {
      if($(otherSelections[i]).val() == newSelection) {
        $(otherSelections[i]).val(0);
      }
    }
  }

  showOnlySelectedWidgetSettings();
  updateSidebarPreview(); // in case a setting impacts this
}

function showOnlySelectedWidgetSettings() {
  // show only the settings that pertain to the selected widget set

  var selections = Array();

  $('#sidebar_layout_section select').each(function() {
    selections.push($(this).val());
  });

  // if the battery widget isn't there, show the autobattery info
  if(selections.indexOf('2') == -1) {
    $('#autobattery_setting').show();
  } else {
    $('#autobattery_setting').hide();
  }

  // if any of the weather widgets are there, show the weather-related settings
  if(selections.indexOf('7') != -1 || selections.indexOf('8') != -1) {
    $('#widget_weather_settings').show();
  } else {
    $('#widget_weather_settings').hide();
  }

  // alt tz widget
  if(selections.indexOf('3') != -1) {
    $('#widget_altclock_settings').show();
  } else {
    $('#widget_altclock_settings').hide();
  }

  // heath widget
  if(selections.indexOf('10') != -1) {
    $('#widget_health_settings').show();
  } else {
    $('#widget_health_settings').hide();
  }
}


function widgetsShouldBeCompact() {
  var useLargeFonts = ($('#use_large_sidebar_font_setting .btn.active').data('setting') == 'yes') ? true : false;
  var showBatteryPct = ($('#battery_meter_setting .btn.active').data('setting') == 'icon-and-percent') ? true : false;

  var totalHeight = 0;

  var widgetHeights;

  if(useLargeFonts) {
    widgetHeights = {
      '0' : 0,
      '2' : (showBatteryPct) ? 33 : 14,
      '3' : 29,
      '4' : 62,
      '5' : 14,
      '6' : 29,
      '7' : 44,
      '8' : 63,
      '9' : 31,
      '10' : 32
    }
  } else {
    widgetHeights = {
      '0' : 0,
      '2' : (showBatteryPct) ? 27 : 14,
      '3' : 26,
      '4' : 58,
      '5' : 14,
      '6' : 26,
      '7' : 42,
      '8' : 60,
      '9' : 31,
      '10' : 32
    }
  }

  for(var i = 0; i < 3; i++) {
    var widget_id = $('#widget_' + i + '_selector').val();
    totalHeight += widgetHeights[widget_id];
  }

  return (totalHeight > 142) ? true : false;
}

function updateSidebarPreview() {

  var useLargeFonts = ($('#use_large_sidebar_font_setting .btn.active').data('setting') == 'yes') ? true : false;

  for(var i = 0; i < 3; i++) {
    var widget_id = $('#widget_' + i + '_selector').val();

    var image_url = 'images/sidebar_widgets/';

    if(useLargeFonts) {
      image_url += 'large_font/';
    }

    image_url += widget_id + '-';

    // set the first image
    switch(widget_id) {
      case '2':
        image_url += 'BATTERY';
        if($('#battery_meter_setting .btn.active').data('setting') == 'icon-and-percent') {
          image_url += '_WITH_PCT';
        }
        break;
      case '3':
        image_url += 'ALT_TZ';
        break;
      case '4':
        image_url += 'DATE';
        if(widgetsShouldBeCompact()) {
          image_url += '_COMPACT';
        }
        break;
      case '5':
        image_url += 'SECONDS';
        break;
      case '6':
        image_url += 'WEEK_NUMBER';
        break;
      case '7':
        image_url += 'WEATHER_CURRENT';
        break;
      case '8':
        image_url += 'WEATHER_TODAY';
        break;
      case '9':
        image_url += 'TIME';
        break;
      case '10':
        image_url += 'HEALTH';
        break;
      case '11':
        image_url += 'BEATS';
        break;
      case '0':
        image_url += 'NONE';
        break;
    }

    image_url += '.png';

    $('#widget_' + i + '_preview img').attr("src", image_url);
  }

}

$('#sidebar_layout_section select').on('change', sidebarWidgetSelectionChanged);

function setFormHasChanges() {
  $('#toolbar_nochanges').addClass('hidden');
  $('#toolbar_haschanges').removeClass('hidden');

  $('#number_of_changes').text("Changes made");
}

function setFormIsUnchanged() {
  $('#toolbar_nochanges').removeClass('hidden');
  $('#toolbar_haschanges').addClass('hidden');
}


function updateCustomPreview() {
  $('#saveNewPresetButton').show();
  $('#savedIndication').hide();

  if($('#time-color').val() && $('#time-bg-color').val() && $('#sidebar-color').val()) {
    $('#custom_preview_help').addClass('hidden');
    $('#custom_preview_container').removeClass('hidden');
  } else {
    $('#custom_preview_help').removeClass('hidden');
    $('#custom_preview_container').addClass('hidden');
  }

  $('#custom_preview').css('color', $('#time-color').val());
  $('#custom_preview').css('background', $('#time-bg-color').val());
  $('#custom_preview').css('border-color', $('#sidebar-color').val());

  showCorrectedColors();
}


function resetSettings() {
  $('label.btn').removeClass('active');
  $(':radio').prop('checked', false);

  $('#manual_weather_loc_setting_area').collapse('hide');

  $('#language_selection').val('(No change)');
  $('#altclock_offset_hour option[data-setting="0"]').prop('selected', true);

  loadLastUsedColors();
  loadPreviousSettings();
  updateSidebarPreview();
  setFormIsUnchanged();
}

$('#close_button').on('click', cancelAndClose);
$('#reset_button').on('click', resetSettings);
$('#save_button').on('click', sendSettingsToWatch);

// And finally, the actual pebble configuration stuff
function getQueryParam(variable, defaultValue) {
  // Find all URL parameters
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');

    // If the query variable parameter is found, decode it to use and return it for use
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}

function sendSettingsToWatch() {
  // stick all the settings into a JSON object
  var config = {};

  // for each setting, check if we need to send it

  // color settings
  if($('#time-bg-color').val()) {
    config.color_bg = $('#time-bg-color').val().substr(1);
    window.localStorage.setItem('time-bg-color', config.color_bg);
  }

  if($('#sidebar-color').val()) {
    config.color_sidebar = $('#sidebar-color').val().substr(1);
    window.localStorage.setItem('sidebar-color', config.color_sidebar);
  }

  if($('#time-color').val()) {
    config.color_time = $('#time-color').val().substr(1);
    window.localStorage.setItem('time-color', config.color_time);
  }

  if($('#sidebar-text-color').val()) {
    config.sidebar_text_color = $('#sidebar-text-color').val().substr(1);
    window.localStorage.setItem('sidebar-text-color', config.sidebar_text_color);
  }

  // general options
  if($('#language_selection option:selected').data('setting') != -1) {
    config.language_id = $('#language_selection option:selected').data('setting');
  }

  if($('#time_leading_zero_setting .btn.active')) {
    config.leading_zero_setting = $('#time_leading_zero_setting .btn.active').data('setting');
  }

  if($('#clock_font_setting .btn.active')) {
    config.clock_font_setting = $('#clock_font_setting .btn.active').data('setting');
  }

  // vibration settings
  if($('#bluetooth_vibe_setting .btn.active')) {
    config.bluetooth_vibe_setting = $('#bluetooth_vibe_setting .btn.active').data('setting');
  }

  if($('#hourly_vibe_setting .btn.active')) {
    config.hourly_vibe_setting = $('#hourly_vibe_setting .btn.active').data('setting');
  }

  // sidebar settings
  config.widget_0_id = parseInt($('#widget_0_selector').val(), 10);
  config.widget_1_id = parseInt($('#widget_1_selector').val(), 10);
  config.widget_2_id = parseInt($('#widget_2_selector').val(), 10);

  if($('#sidebar_position_setting .btn.active').size() > 0) {
    config.sidebar_position = $('#sidebar_position_setting .btn.active').data('setting');
  }

  if($('#use_large_sidebar_font_setting .btn.active')) {
    config.use_large_sidebar_font_setting = $('#use_large_sidebar_font_setting .btn.active').data('setting');
  }

  // weather widget settings
  if($('#units_setting .btn.active').size() > 0) {
    config.units = $('#units_setting .btn.active').data('setting');
  }

  if($('#weather_setting .btn.active').size() > 0) {
    var weather_setting = $('#weather_setting .btn.active').data('setting');

    config.weather_setting = weather_setting;

    if(weather_setting == 'auto') {
      config.weather_loc = '';
    } else if(weather_setting == 'manual') {
      config.weather_loc = $('#weather_loc').val();
    }
  }

  // battery widget settings
  if($('#battery_meter_setting .btn.active')) {
    config.battery_meter_setting = $('#battery_meter_setting .btn.active').data('setting');
  }

  if($('#autobattery_setting .btn.active')) {
    config.autobattery_setting = $('#autobattery_setting .btn.active').data('setting');
  }

  if($('#decimal_separator .btn.active')) {
    config.decimal_separator = $('#decimal_separator .btn.active').data('setting');
  }

  if($('#health_use_distance .btn.active')) {
    config.health_use_distance = $('#health_use_distance .btn.active').data('setting');
  }

  if($('#health_use_restful_sleep .btn.active')) {
    config.health_use_restful_sleep = $('#health_use_restful_sleep .btn.active').data('setting');
  }

  // alt clock widgets
  config.altclock_name = $('#altclock_name').val();
  config.altclock_offset = $('#altclock_offset_hour option:selected').data('setting');

  // add the version, in case we need to do more migrations
  config.settings_version = CURRENT_SETTINGS_VERSION;

  // Save the configuration data to localStorage
  window.localStorage.setItem('savedSettings', JSON.stringify(config));

  // Send the config data to the tracking service for SCIENCE
  trackSettings(config);

  // Set the return URL depending on the runtime environment
  var return_to = getQueryParam('return_to', 'pebblejs://close#');
  document.location.href = return_to + encodeURIComponent(JSON.stringify(config));
}

function trackSettings(config) {
  // track the color theme
  var colorTheme = 'TimeStyleTheme,#' + config.color_time + ',#' + config.color_bg + ',#' + config.color_sidebar + ',#' + config.sidebar_text_color;
  ga('set', 'dimension1', colorTheme);

  // track the sidebar layout
  var widgetNames = {
    '0': 'None',
    '2': 'Battery',
    '3': 'Alt-TZ',
    '4': 'Date',
    '5': 'Seconds',
    '6': 'WeekNum',
    '7': 'WeatherCurrent',
    '8': 'WeatherToday',
    '9': 'CurrentTime',
    '10': 'Health'
  };

  // track the sidebar layout selections
  var sidebarLayout = widgetNames[config.widget_0_id] + ', ' +
                      widgetNames[config.widget_1_id] + ', ' +
                      widgetNames[config.widget_2_id];
  ga('set', 'dimension2', sidebarLayout);

  // sidebar options
  ga('set', 'dimension3', config.sidebar_position);
  ga('set', 'dimension4', config.use_large_sidebar_font_setting);

  // clock options
  ga('set', 'dimension5', config.clock_font_setting);
  ga('set', 'dimension6', config.leading_zero_setting);

  // vibration options
  ga('set', 'dimension7', config.bluetooth_vibe_setting);
  ga('set', 'dimension8', config.hourly_vibe_setting);

  // language
  var languageName = $('#language_selection option:selected').html();
  ga('set', 'dimension9', languageName);

  // save the config info
  ga('send', {
    hitType: 'event',
    eventCategory: 'Settings',
    eventAction: 'save',
  });
}

function cancelAndClose() {
  var return_to = getQueryParam('return_to', 'pebblejs://close');
  document.location.href = return_to;
}

$('#weather_setting input').on('change', function(){
  $target = $('#manual_weather_loc_setting_area');

  if ($('#weather_setting_manual').is(':checked')) {
    $target.collapse('show');
  } else {
    $target.collapse('hide');
  }
});

/* stuff for the preset saving/loading */


/* saves the preset and adds it to the "your presets" section */
function saveNewPreset() {
  $('#saveNewPresetButton').hide();
  $('#savedIndication').show();

  var savedPresets = window.localStorage.getItem('savedPresets');

  if(savedPresets) {
    savedPresets = JSON.parse(savedPresets);
  } else {
    savedPresets = [];
  }

  // loop through the colors and save them
  var colorNames = ['time-color', 'time-bg-color', 'sidebar-color', 'sidebar-text-color'];

  var newPreset = {};

  for(i = 0; i < colorNames.length; i++) {
    colorName = colorNames[i];

    if($('#' + colorName).val() ) {
      newPreset[colorName] = $('#' + colorName).val();
    } else {
      newPreset[colorName] = '#000000';
    }
  }

  savedPresets.push(newPreset);

  window.localStorage.setItem('savedPresets', JSON.stringify(savedPresets));

  // finally, update the preset area
  showCustomPresets();
}

var template = '\
<label class="btn btn-default" data-time-color="{{time-color}}" data-time-bg-color="{{time-bg-color}}" data-sidebar-color="{{sidebar-color}}" data-sidebar-text-color="{{sidebar-text-color}}">\
  <input type="radio" name="options" id="option1" autocomplete="off">\
  <div class="example_face" style="color: {{time-color}}; background-color: {{time-bg-color}}; border-color: {{sidebar-color}};">6<br>32</div>\
</label>\
<div class="theme_actions">\
<button class="btn btn-danger btn-sm" onclick="deletePreset({{preset-id}})" type="button"><span class="glyphicon glyphicon-trash"></span> Delete</button>\
<button class="btn btn-info btn-sm" type="button" data-toggle="modal" data-target="#shareThemeModal" data-themestring="TimeStyleTheme,{{time-color}},{{time-bg-color}},{{sidebar-color}},{{sidebar-text-color}}">\
  <span class="glyphicon glyphicon-share-alt"></span> Share</span>\
</button>\
</div>\
<br>';

function showCustomPresets() {
  var savedPresets = window.localStorage.getItem('savedPresets');
  var htmlToInsert = '';

  if(savedPresets) {
    savedPresets = JSON.parse(savedPresets);

    for(i = 0; i < savedPresets.length; i++) {
      var preset = savedPresets[i];
      var htmlString = template;

      htmlString = htmlString.replace(/\{\{time-color\}\}/g, preset['time-color']);
      htmlString = htmlString.replace(/\{\{time-bg-color\}\}/g, preset['time-bg-color']);
      htmlString = htmlString.replace(/\{\{sidebar-color\}\}/g, preset['sidebar-color']);
      htmlString = htmlString.replace(/\{\{sidebar-text-color\}\}/g, preset['sidebar-text-color']);
      htmlString = htmlString.replace(/\{\{preset-id\}\}/g, i)

      htmlToInsert += htmlString;
    }

    $('#saved_themes_area').html(htmlToInsert);

    if( savedPresets.length > 0) {
      $('#custom_theme_empty').hide();
    } else {
      $('#custom_theme_empty').show();
    }
  } else {
    $('#custom_theme_empty').show();
  }

  // finally, re-register event handlers if needed
  $('#saved_themes_area label.btn').on('click', presetSelected);

  showCorrectedColors();
}

function deletePreset(presetId) {
  var savedPresets = window.localStorage.getItem('savedPresets');

  if(savedPresets) {
    savedPresets = JSON.parse(savedPresets);

    savedPresets.splice(presetId, 1);

    // if the current preset was deleted, it may no longer be saved
    $('#saveNewPresetButton').show();
    $('#savedIndication').hide();
  }

  window.localStorage.setItem('savedPresets', JSON.stringify(savedPresets));

  //redraw the custom presets
  showCustomPresets();
}

function loadPastedTheme() {
  var pastedTheme = $('#pasted_theme').val();

  if(pastedTheme) {
    var themeComponents = pastedTheme.split(',');

    if(themeComponents.length == 5) {
      // use the colors from the theme
      $("#time-color").spectrum("set", themeComponents[1]);
      $("#time-bg-color").spectrum("set", themeComponents[2]);
      $("#sidebar-color").spectrum("set", themeComponents[3]);
      $("#sidebar-text-color").spectrum("set", themeComponents[4]);

      // now shove it into local storage
      saveNewPreset();

      updateCustomPreview();
      setFormHasChanges();

      // now wipe the field
      $('#pasted_theme').val('');
    }
  }
}

// support the individual "share" entry point from the preset selector
$('#shareThemeModal').on('show.bs.modal', function (event) {
  var button = $(event.relatedTarget); // Button that triggered the modal
  var themeString = button.data('themestring');

  var mailToParams = {};

  mailToParams.address = "";
  mailToParams.subject = "TimeStyle Theme";
  mailToParams.body    = "To try this theme, copy and paste the following text into TimeStyle's settings:\n\n" + themeString;

  $('#share_via_email').attr('href', generateMailString(mailToParams));

  mailToParams.address = "freakified+themesuggestion@gmail.com";
  mailToParams.subject = "TimeStyle Theme Suggestion";
  mailToParams.body    = themeString;

  $('#suggest_as_preset').attr('href', generateMailString(mailToParams));


  $('#sharable_theme').html(themeString);

});

function generateMailString(mailToParams) {
  var mailToURL = "mailto:";

  mailToURL += encodeURIComponent(mailToParams.address);
  mailToURL += "?subject=";
  mailToURL += encodeURIComponent(mailToParams.subject);
  mailToURL += "&body=";
  mailToURL += encodeURIComponent(mailToParams.body);

  return mailToURL;
}

var colorMappingSunlight = {
  "#000000": "#000000",
  "#000055": "#001e41",
  "#0000aa": "#004387",
  "#0000ff": "#0068ca",
  "#005500": "#2b4a2c",
  "#005555": "#27514f",
  "#0055aa": "#16638d",
  "#0055ff": "#007dce",
  "#00aa00": "#5e9860",
  "#00aa55": "#5c9b72",
  "#00aaaa": "#57a5a2",
  "#00aaff": "#4cb4db",
  "#00ff00": "#8ee391",
  "#00ff55": "#8ee69e",
  "#00ffaa": "#8aebc0",
  "#00ffff": "#84f5f1",
  "#550000": "#4a161b",
  "#550055": "#482748",
  "#5500aa": "#40488a",
  "#5500ff": "#2f6bcc",
  "#555500": "#564e36",
  "#555555": "#545454",
  "#5555aa": "#4f6790",
  "#5555ff": "#4180d0",
  "#55aa00": "#759a64",
  "#55aa55": "#759d76",
  "#55aaaa": "#71a6a4",
  "#55aaff": "#69b5dd",
  "#55ff00": "#9ee594",
  "#55ff55": "#9de7a0",
  "#55ffaa": "#9becc2",
  "#55ffff": "#95f6f2",
  "#aa0000": "#99353f",
  "#aa0055": "#983e5a",
  "#aa00aa": "#955694",
  "#aa00ff": "#8f74d2",
  "#aa5500": "#9d5b4d",
  "#aa5555": "#9d6064",
  "#aa55aa": "#9a7099",
  "#aa55ff": "#9587d5",
  "#aaaa00": "#afa072",
  "#aaaa55": "#aea382",
  "#aaaaaa": "#ababab",
  "#ffffff": "#ffffff",
  "#aaaaff": "#a7bae2",
  "#aaff00": "#c9e89d",
  "#aaff55": "#c9eaa7",
  "#aaffaa": "#c7f0c8",
  "#aaffff": "#c3f9f7",
  "#ff0000": "#e35462",
  "#ff0055": "#e25874",
  "#ff00aa": "#e16aa3",
  "#ff00ff": "#de83dc",
  "#ff5500": "#e66e6b",
  "#ff5555": "#e6727c",
  "#ff55aa": "#e37fa7",
  "#ff55ff": "#e194df",
  "#ffaa00": "#f1aa86",
  "#ffaa55": "#f1ad93",
  "#ffaaaa": "#efb5b8",
  "#ffaaff": "#ecc3eb",
  "#ffff00": "#ffeeab",
  "#ffff55": "#fff1b5",
  "#ffffaa": "#fff6d3"
};

function showCorrectedColors() {
  $('.example_face').each(function() {
    var me = $(this);

    me.css('color', colorMappingSunlight[rgb2hex(me.css('color'))]);
    me.css('background-color', colorMappingSunlight[rgb2hex(me.css('background-color'))]);
    me.css('border-color', colorMappingSunlight[rgb2hex(me.css('border-color'))]);
  });
}

// rgb to hex conversions from
// http://stackoverflow.com/questions/1740700/how-to-get-hex-color-value-rather-than-rgb-value
var hexDigits = new Array
        ("0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f");

function rgb2hex(rgb) {


  if(rgb) {
    rgb = rgb.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);
    return "#" + hex(rgb[1]) + hex(rgb[2]) + hex(rgb[3]);
  }
}

function hex(x) {
  return isNaN(x) ? "00" : hexDigits[(x - x % 16) / 16] + hexDigits[x % 16];
}

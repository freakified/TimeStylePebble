
// if we have any persistent data saved, load it in
$(document).ready(function() {
  loadLastUsedColors();
  showCustomPresets();

  loadPreviousSettings();
});

function loadSettingCheckbox(elementID, setting) {
  if(setting) {
    $('#' + elementID + ' label[data-setting="' + setting + '"] input').attr('checked', true);
    $('#' + elementID + ' label[data-setting="' + setting + '"]').addClass('active');
  }
}

function loadPreviousSettings() {
  // load the previous settings
  var savedSettings = JSON.parse(window.localStorage.getItem('savedSettings'));

  console.log(savedSettings);

  loadSettingCheckbox('sidebar_position_setting', savedSettings.sidebar_position);
  loadSettingCheckbox('units_setting', savedSettings.units);
  loadSettingCheckbox('bluetooth_vibe_setting', savedSettings.bluetooth_vibe_setting);
  loadSettingCheckbox('hourly_vibe_setting', savedSettings.hourly_vibe_setting);
  loadSettingCheckbox('battery_meter_setting', savedSettings.battery_meter_setting);
  loadSettingCheckbox('only_show_battery_when_low_setting', savedSettings.only_show_battery_when_low_setting);
  loadSettingCheckbox('time_leading_zero_setting', savedSettings.leading_zero_setting);
  loadSettingCheckbox('clock_font_setting', savedSettings.clock_font_setting);
  loadSettingCheckbox('use_large_sidebar_font_setting', savedSettings.use_large_sidebar_font_setting);
  loadSettingCheckbox('disable_weather', savedSettings.disable_weather);
  loadSettingCheckbox('weather_setting', savedSettings.weather_setting);

  $('#weather_loc').val(savedSettings.weather_loc);

  if(savedSettings.weather_setting == 'manual') {
    $('#manual_weather_loc_setting_area').collapse('show');
  }

  if(savedSettings.language_id !== undefined) {
    $('#language_selection option[data-setting="' + savedSettings.language_id + '"]').prop('selected', true);
  }


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

$('label.btn').on('change', updateToolbar);
$('select').on('change', updateToolbar);

function customColorChanged() {
  updateToolbar();
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

  updateToolbar();
  updateCustomPreview();
}

$('#preset_selector label.btn').on('click', presetSelected);

function updateToolbar() {
  var counter = 0;

  var colorNames = ['time-color', 'time-bg-color', 'sidebar-color', 'sidebar-text-color'];

  // count all color changes as a single change
  for(i = 0; i < colorNames.length; i++) {
    colorName = colorNames[i];

    var storedColor = window.localStorage.getItem(colorName);

    if(storedColor) {
      if($('#' + colorName).val() != ("#" + storedColor)) {
        counter = 1;
      }
    } else  {
      if($('#' + colorName).val()) {
        counter = 1;
      }
    }
  }

  if(counter > 0) {
    $('#toolbar_nochanges').addClass('hidden');
    $('#toolbar_haschanges').removeClass('hidden');

    var countText = counter + ' item';
    countText += (counter != 1) ? 's' : '';
    countText += ' set';

    $('#number_of_changes').text(countText);
  } else {
    $('#toolbar_nochanges').removeClass('hidden');
    $('#toolbar_haschanges').addClass('hidden');
  }
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
}

function resetSettings() {
  loadLastUsedColors();

  $('label.btn').removeClass('active');
  $(':radio').prop('checked', false);

  $('#manual_weather_loc_setting_area').collapse('hide');

  $('#language_selection').val('(No change)');

  updateToolbar();
  updateCustomPreview();
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
  if($('#time-color').val()) {
    config.color_time = $('#time-color').val().substr(1);
    window.localStorage.setItem('time-color', config.color_time);
  }

  if($('#time-bg-color').val()) {
    config.color_bg = $('#time-bg-color').val().substr(1);
    window.localStorage.setItem('time-bg-color', config.color_bg);
  }

  if($('#sidebar-color').val()) {
    config.color_sidebar = $('#sidebar-color').val().substr(1);
    window.localStorage.setItem('sidebar-color', config.color_sidebar);
  }

  if($('#sidebar-text-color').val()) {
    config.sidebar_text_color = $('#sidebar-text-color').val().substr(1);
    window.localStorage.setItem('sidebar-text-color', config.sidebar_text_color);
  }

  if($('#sidebar_position_setting .btn.active').size() > 0) {
    config.sidebar_position = $('#sidebar_position_setting .btn.active').data('setting');
  }

  if($('#units_setting .btn.active').size() > 0) {
    config.units = $('#units_setting .btn.active').data('setting');
  }

  if($('#weather_setting .btn.active').size() > 0) {
    var weather_setting = $('#weather_setting .btn.active').data('setting');

    config.weather_setting = weather_setting;

    if(weather_setting == 'auto') {
      config.disable_weather = 'no';
      config.weather_loc = '';
    } else if(weather_setting == 'manual') {
      config.disable_weather = 'no';
      config.weather_loc = $('#weather_loc').val();
    } else if(weather_setting == 'disable') {
      config.disable_weather = 'yes';
    }
  }

  if($('#bluetooth_vibe_setting .btn.active')) {
    config.bluetooth_vibe_setting = $('#bluetooth_vibe_setting .btn.active').data('setting');
  }

  if($('#language_selection option:selected').data('setting') != -1) {
    config.language_id = $('#language_selection option:selected').data('setting');
  }

  if($('#battery_meter_setting .btn.active')) {
    config.battery_meter_setting = $('#battery_meter_setting .btn.active').data('setting');
  }

  if($('#only_show_battery_when_low_setting .btn.active')) {
    config.only_show_battery_when_low_setting = $('#only_show_battery_when_low_setting .btn.active').data('setting');
  }

  if($('#time_leading_zero_setting .btn.active')) {
    config.leading_zero_setting = $('#time_leading_zero_setting .btn.active').data('setting');
  }

  if($('#clock_font_setting .btn.active')) {
    config.clock_font_setting = $('#clock_font_setting .btn.active').data('setting');
  }

  if($('#use_large_sidebar_font_setting .btn.active')) {
    config.use_large_sidebar_font_setting = $('#use_large_sidebar_font_setting .btn.active').data('setting');
  }

  if($('#hourly_vibe_setting .btn.active')) {
    config.hourly_vibe_setting = $('#hourly_vibe_setting .btn.active').data('setting');
  }

  // Save the configuration data to localStorage
  window.localStorage.setItem('savedSettings', JSON.stringify(config));

  // Set the return URL depending on the runtime environment
  var return_to = getQueryParam('return_to', 'pebblejs://close#');
  document.location.href = return_to + encodeURIComponent(JSON.stringify(config));
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
      updateToolbar();
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
  $('#preset_selector label').each(function() {
    var me = $(this);

    $('.example_face', me).css('color', colorMappingSunlight[me.data('time-color').toLowerCase()]);
    $('.example_face', me).css('background-color', colorMappingSunlight[me.data('time-bg-color').toLowerCase()]);
    $('.example_face', me).css('border-color', colorMappingSunlight[me.data('sidebar-color').toLowerCase()]);
  });
}

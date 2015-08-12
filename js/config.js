
// if we have any persistent data saved, load it in
$(document).ready(function() {
  loadLastUsedColors();
  showCustomPresets();
});

function loadLastUsedColors() {
  // try to load each of the four colors
  var colorNames = ['time-color', 'time-bg-color', 'sidebar-color', 'sidebar-text-color'];

  colorNames.forEach(function(colorName, index, array) {
    var storedColor = window.localStorage.getItem(colorName);

    if(storedColor) {
      $('#' + colorName).spectrum("set", storedColor);
      $('#' + colorName).data('storedColor', storedColor);
    } else {
      $('#' + colorName).spectrum("set", null);
    }
  });

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
  var counter = $('label.btn.active').size();

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

  if($('#language_selection option:selected').data('setting') != -1) {
    counter++;
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
<button class="btn btn-primary btn-sm" type="button" data-toggle="modal" data-target="#shareThemeModal" data-themestring="TimeStyleTheme,{{time-color}},{{time-bg-color}},{{sidebar-color}},{{sidebar-text-color}}">\
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
  var button = $(event.relatedTarget) // Button that triggered the modal
  var themeString = button.data('themestring')

  $('#sharable_theme').html(themeString);
})

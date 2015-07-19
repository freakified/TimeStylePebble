$('#time-color').on('change', customColorChanged);
$('#time-bg-color').on('change', customColorChanged);
$('#sidebar-color').on('change', customColorChanged);
$('#sidebar-text-color').on('change', customColorChanged);

$('label.btn').on('change', updateToolbar);
$('select').on('change', updateToolbar);

function customColorChanged() {
  updateToolbar();
  updateCustomPreview();
}

$('#preset_selector .btn').on('click', function() {
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

  updateCustomPreview();
});


function updateToolbar() {
  var counter = $('label.btn.active').size();

  if($('#preset_selector .btn.active').size() == 0) {
    if($('#time-color').val()) {
      counter++;
    }

    if($('#time-bg-color').val()) {
      counter++;
    }

    if($('#sidebar-color').val()) {
      counter++;
    }

    if($('#sidebar-text-color').val()) {
      counter++;
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
  if($('#time-color').val() && $('#time-bg-color').val() && $('#sidebar-color').val()) {
    $('#custom_preview_help').addClass('hidden');
    $('#custom_preview').removeClass('hidden');
  } else {
    $('#custom_preview_help').removeClass('hidden');
    $('#custom_preview').addClass('hidden');
  }

  $('#custom_preview').css('color', $('#time-color').val());
  $('#custom_preview').css('background', $('#time-bg-color').val());
  $('#custom_preview').css('border-color', $('#sidebar-color').val());
}

function resetSettings() {
  $("#time-color").spectrum("set", null);
  $("#time-bg-color").spectrum("set", null);
  $("#sidebar-color").spectrum("set", null);
  $("#sidebar-text-color").spectrum("set", null);

  $('label.btn').removeClass('active');
  $(':radio').prop('checked', false);

  $('#manual_weather_loc_setting_area').collapse('hide');
  $('input').val(null);

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
  }

  if($('#time-bg-color').val()) {
    config.color_bg = $('#time-bg-color').val().substr(1);
  }

  if($('#sidebar-color').val()) {
    config.color_sidebar = $('#sidebar-color').val().substr(1);
  }

  if($('#sidebar-text-color').val()) {
    config.sidebar_text_color = $('#sidebar-text-color').val().substr(1);
  }

  if($('#sidebar_position_setting .btn.active').size() > 0) {
    config.sidebar_position = $('#sidebar_position_setting .btn.active').data('setting');
  }

  if($('#units_setting .btn.active').size() > 0) {
    config.units = $('#units_setting .btn.active').data('setting');
  }

  if($('#weather_loc_setting .btn.active').size() > 0) {
    if($('#weather_setting_manual').is(':checked')) {
      config.weather_loc = $('#weather_loc').val();
    } else {
      config.weather_loc = '';
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

  if($('#time_leading_zero_setting .btn.active')) {
    config.leading_zero_setting = $('#time_leading_zero_setting .btn.active').data('setting');
  }

  // Set the return URL depending on the runtime environment
  var return_to = getQueryParam('return_to', 'pebblejs://close#');
  document.location.href = return_to + encodeURIComponent(JSON.stringify(config));
}

function cancelAndClose() {
  var return_to = getQueryParam('return_to', 'pebblejs://close');
  document.location.href = return_to;
}

$('#weather_loc_setting input').on('change', function(){
  $target = $('#manual_weather_loc_setting_area');

  if ($('#weather_setting_manual').is(':checked')) {
    $target.collapse('show');
  } else {
    $target.collapse('hide');
  }
});

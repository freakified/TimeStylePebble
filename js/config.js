$('#time-color').on('change', customColorChanged);
$('#time-bg-color').on('change', customColorChanged);
$('#sidebar-color').on('change', customColorChanged);

function customColorChanged() {
  updateToolbar();
  updateCustomPreview();
}


$('#preset_selector .btn').on('click', function() {
  $("#time-color").spectrum("set", $(this).data('time-color'));
  $("#time-bg-color").spectrum("set", $(this).data('time-bg-color'));
  $("#sidebar-color").spectrum("set", $(this).data('sidebar-color'));

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

$('label.btn').on('change', updateToolbar);

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

  $('label.btn').removeClass('active');
  $(':radio').prop('checked', false);

  $('#manual_weather_loc_setting_area').collapse('hide');
  $('input').val(null);

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

  if($('#sidebar_position_setting .btn.active')) {
    config.sidebar_position = $('#sidebar_position_setting .btn.active').data('setting');
  }

  if($('#units_setting .btn.active')) {
    config.units = $('#units_setting .btn.active').data('setting');
  }

  if($('#weather_loc_setting .btn.active')) {
    if($('#weather_setting_manual').is(':checked')) {
      config.weather_loc = $('#weather_loc').val();
    } else {
      config.weather_loc = '';
    }
  }

  if($('#bluetooth_vibe_setting .btn.active')) {
    config.bluetooth_vibe_setting = $('#bluetooth_vibe_setting .btn.active').data('setting');
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

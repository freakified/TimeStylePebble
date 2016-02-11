#include <pebble.h>
#include "util.h"


bool recolor_iterator_cb(GDrawCommand *command, uint32_t index, void *context) {
  GColor *colors = (GColor *)context;

  gdraw_command_set_fill_color(command, colors[0]);
  gdraw_command_set_stroke_color(command, colors[1]);

  return true;
}

void gdraw_command_image_recolor(GDrawCommandImage *img, GColor fill_color, GColor stroke_color) {
  GColor colors[2];
  colors[0] = fill_color;
  colors[1] = stroke_color;

  gdraw_command_list_iterate(gdraw_command_image_get_command_list(img),
                             recolor_iterator_cb, &colors);
}

#ifdef PBL_HEALTH

  bool activity_search_cb(HealthActivity activity, time_t time_start, time_t time_end, void *context) {
    bool *result = (bool *)context;
    *result = true;

    // if this gets called, we know that the activity we're searching for exists, so we call off the search
    return false;
  }

  bool health_service_is_activity_in_range(HealthActivityMask activity_mask, time_t time_start, time_t time_end) {
    bool result = false;

    health_service_activities_iterate(activity_mask, time_start, time_end,
      HealthIterationDirectionPast, activity_search_cb, &result);

    return result;
  }

#endif

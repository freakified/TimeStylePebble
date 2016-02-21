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

int time_get_beats(const struct tm *tm) {
  // code from https://gist.github.com/insom/bf40b91fd25ae1d84764

  time_t t = mktime((struct tm *)tm);
  t = t + 3600; // Add an hour to make into BMT

  struct tm *bt = gmtime(&t);
  double sex = (bt->tm_hour * 3600) + (bt->tm_min * 60) + bt->tm_sec;
  int beats = (int)(10 * (sex / 864)) % 1000;

  return beats;
}

#ifdef PBL_HEALTH
   bool is_health_activity_accessible(HealthActivityMask activity_mask) {
     time_t start = time_start_of_today();
     time_t end = time(NULL);

     // Check step data is available
     HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricStepCount, start, end);
     bool result = mask & HealthServiceAccessibilityMaskAvailable;

     return result;
   }
 #endif

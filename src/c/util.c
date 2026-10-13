#include <pebble.h>
#include "util.h"


bool recolor_iterator_cb(GDrawCommand *command, uint32_t index, void *context) {
  GColor *colors = (GColor *)context;

  gdraw_command_set_fill_color(command, colors[0]);
  gdraw_command_set_stroke_color(command, colors[1]);

  return true;
}

// a command with no stroke is ink drawn as a fill (the distance icon's
// dashes), so it takes the stroke color; everything else as above
static bool recolor_icon_iterator_cb(GDrawCommand *command, uint32_t index, void *context) {
  GColor *colors = (GColor *)context;

  if (gdraw_command_get_stroke_width(command) == 0) {
    gdraw_command_set_fill_color(command, colors[1]);
    return true;
  }

  return recolor_iterator_cb(command, index, context);
}

void gdraw_command_image_recolor_icon(GDrawCommandImage *img, GColor fill_color, GColor stroke_color) {
  GColor colors[2];
  colors[0] = fill_color;
  colors[1] = stroke_color;

  gdraw_command_list_iterate(gdraw_command_image_get_command_list(img),
                             recolor_icon_iterator_cb, &colors);
}

void gdraw_command_image_recolor(GDrawCommandImage *img, GColor fill_color, GColor stroke_color) {
  GColor colors[2];
  colors[0] = fill_color;
  colors[1] = stroke_color;

  gdraw_command_list_iterate(gdraw_command_image_get_command_list(img),
                             recolor_iterator_cb, &colors);
}

int time_get_beats(const struct tm *tm) {
  // get utc time
  time_t utc_time = mktime((struct tm *)tm);

  // convert to "biel mean time" (BMT) by adding an hour
  time_t bmt_time = utc_time + 3600;

  // use gmtime to get the BMT components
  struct tm *bmt_tm = gmtime(&bmt_time);

  // how many seconds have passed since midnight?
  int seconds_since_midnight = bmt_tm->tm_hour * 3600 + bmt_tm->tm_min * 60 + bmt_tm->tm_sec;
  int beats = (seconds_since_midnight * 1000 + 43200) / 86400;

  return beats % 1000;
}

#ifdef PBL_HEALTH
bool is_health_metric_accessible(HealthMetric metric) {
  time_t end = time(NULL);

  // heart rate is an instantaneous metric: the accessibility check only
  // supports a zero-length time range ending now (a day-long range always
  // reports unavailable, which left the widget stuck on "...")
  time_t start =
      (metric == HealthMetricHeartRateBPM) ? end : time_start_of_today();

  HealthServiceAccessibilityMask mask =
      health_service_metric_accessible(metric, start, end);

  return mask & HealthServiceAccessibilityMaskAvailable;
}

bool is_user_sleeping() {
  // currently disabled
  return false;
  //
  //  time_t now = time(NULL);
  //
    //  HealthActivityMask activities = HealthActivitySleep | HealthActivityRestfulSleep;
  //
    //  HealthServiceAccessibilityMask mask = health_service_any_activity_accessible(activities, now, now);
    //  bool sleep_access_available = mask & HealthServiceAccessibilityMaskAvailable;
  //
  //  if(sleep_access_available) {
  //    uint32_t current_activities = health_service_peek_current_activities();
    //    bool sleeping = current_activities & HealthActivitySleep || current_activities & HealthActivityRestfulSleep;
  //
  //    return sleeping;
  //  } else {
  //    return false;
  //  }
}
#endif

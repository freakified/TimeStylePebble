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


// DIT calc defines
#define MS_PER_SEC 1000
#define MS_PER_MIN (MS_PER_SEC * 60)
#define MS_PER_HOUR (MS_PER_MIN * 60)
#define DESECS_PER_DESIM 100
#define DESIMS_PER_DECA 100
#define DECAS_PER_DAY 10
#define DESECS_PER_DECA (DESECS_PER_DESIM * DESIMS_PER_DECA)
#define DESECS_PER_DAY (DESECS_PER_DECA * DECAS_PER_DAY)
// 86400 seconds in a day; 100000 desecs in a day; therefore 864 milliseconds per desec, computed by compiler.
#define MS_PER_DESEC ((1000 * 86400)/DESECS_PER_DAY)

int time_get_dit(const struct tm *tm) {
    // From https://gitea.s0.is/s0/Dit/src/branch/main/Echo%20scripts/dit-s0.c

    time_t t = mktime((struct tm *)tm);
    struct tm *gmtm = gmtime(&t);

    // Portable code shouldn't directly use time_t type, but access the struct tm members.
	// Discard day, month, year values.
	// Shift hour value by 12 to match UTC-12, the reference meridian for DIT.
	// uint_fast32_t is a type that is defined to be the fastest 32-bit uint on the platform -- might be >32b in implementation.
	uint_fast32_t current_utc12_milliseconds = (((gmtm->tm_hour + 12) % 24) * MS_PER_HOUR) +
											   (gmtm->tm_min * MS_PER_MIN) +
											   (gmtm->tm_sec * MS_PER_SEC);

	// Because milliseconds divide neatly into desecs, we can use integer arithmetic only, provided we have a 32-bit uint type available.
	uint_fast32_t current_dit_desecs = current_utc12_milliseconds / MS_PER_DESEC;

	return current_dit_desecs / DESECS_PER_DESIM;
}

#ifdef PBL_HEALTH
   bool is_health_metric_accessible(HealthMetric metric) {
     time_t start = time_start_of_today();
     time_t end = time(NULL);

     // Check step data is available
     HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
     bool result = mask & HealthServiceAccessibilityMaskAvailable;

     return result;
   }

   bool is_user_sleeping() {
     // temporarily disable sleep function until Pebble gets their act together
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

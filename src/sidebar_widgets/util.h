#pragma once
#include <pebble.h>
#include "time.h"


/*
 * For the specified GDrawCommandImage, recolors it with
 * the specified fill and stroke colors
 */
extern void gdraw_command_image_recolor(GDrawCommandImage *img, GColor fill_color, GColor stroke_color);

/*
 * Returns the current time in Swatch Internet Time "beats"
 */
extern int time_get_beats(const struct tm *tm);

#ifdef PBL_HEALTH
  /*
   * Checks if any of the specified health activites exist in the specified time range
   */
  extern bool is_health_activity_accessible(HealthActivityMask activity_mask);
#endif

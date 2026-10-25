#pragma once
#include <pebble.h>
#include "time.h"


/*
 * For the specified GDrawCommandImage, recolors it with
 * the specified fill and stroke colors
 */
extern void gdraw_command_image_recolor(GDrawCommandImage *img, GColor fill_color, GColor stroke_color);

/*
 * Recolors a widget icon: like gdraw_command_image_recolor, except that a
 * command with no stroke is ink drawn as a fill (the distance icon's
 * dashes) and takes the stroke color. Icons that use white strokes as
 * paper, like the battery's charging bolt halo, need the plain recolor.
 */
extern void gdraw_command_image_recolor_icon(GDrawCommandImage *img, GColor fill_color, GColor stroke_color);

/*
 * Returns the current time in Swatch Internet Time "beats"
 */
extern int time_get_beats(const struct tm *tm);

#ifdef PBL_HEALTH
  /*
   * Checks if any of the specified health activites exist in the specified time range
   */
  extern bool is_health_metric_accessible(HealthMetric metric);
#endif

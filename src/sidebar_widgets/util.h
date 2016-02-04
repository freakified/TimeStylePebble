#pragma once
#include <pebble.h>

/*
 * For the specified GDrawCommandImage, recolors it with
 * the specified fill and stroke colors
 */
extern void gdraw_command_image_recolor(GDrawCommandImage *img, GColor fill_color, GColor stroke_color);

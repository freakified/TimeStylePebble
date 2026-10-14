#pragma once
#include <pebble.h>

// the next-alarm widget (alarm_service_peek_next) is only offered on the
// newer watches
#if defined(PBL_PLATFORM_FLINT) || defined(PBL_PLATFORM_EMERY) || \
    defined(PBL_PLATFORM_GABBRO)
#define SIDEBAR_HAS_ALARMS
#endif

typedef enum {
  SIDEBAR_LAYOUT_STACKED = 0,
  SIDEBAR_LAYOUT_INLINE = 2
} SidebarLayoutMode;

typedef enum {
  SIDEBAR_CONTENT_EMPTY = 0,
  SIDEBAR_CONTENT_TEXT,
  SIDEBAR_CONTENT_ICON,
  SIDEBAR_CONTENT_ICON_VALUE,
  SIDEBAR_CONTENT_LABEL_VALUE,
  SIDEBAR_CONTENT_ICON_VALUE_PAIR_DIVIDED,
  SIDEBAR_CONTENT_ICON_VALUE_WITH_SUBTEXT,
  SIDEBAR_CONTENT_DATE,
  SIDEBAR_CONTENT_CUSTOM
} SidebarContentKind;

typedef enum {
  SIDEBAR_ICON_NONE = 0,
  SIDEBAR_ICON_IMAGE
} SidebarIconKind;

/*
 * The different types of sidebar widgets:
 * we'll give them numbers so that we can index them in settings
 */
typedef enum {
  EMPTY                     = 0,
  // 1 was BLUETOOTH_DISCONNECT, retired in favor of the status bar
  BATTERY_METER             = 2,
  WORLD_TIME_1              = 3,
  DATE                      = 4,
  SECONDS                   = 5,
  WEEK_NUMBER               = 6,
  WEATHER_CURRENT           = 7,
  WEATHER_FORECAST_TODAY    = 8,
  SLEEP_TIMER               = 9,
  STEP_COUNTER              = 10,
  BEATS                     = 11,
  HEARTRATE                 = 12,
  WEATHER_UV_INDEX          = 13,
  WORLD_TIME_2              = 14,
  NEXT_SUN_EVENT            = 15,
  // 16 was WEATHER_WIND, retired
  WEATHER_PRECIP_CHANCE     = 17,
  DISTANCE_WALKED           = 18,
  ACTIVE_CALORIES           = 19,
  NEXT_ALARM                = 20
} SidebarWidgetType;

// whether a stored or received widget id names a real widget (EMPTY
// included); extend this alongside the enum
static inline bool SidebarWidgets_isKnownType(uint8_t type) {
  return type == EMPTY ||
         (type >= BATTERY_METER && type <= NEXT_ALARM && type != 16);
}

typedef struct {
  // full is larger because it can carry a label prefix and/or day suffix
  char full[24];
  char compact[16];
  char tiny[16];
} SidebarTextCandidates;

typedef struct {
  SidebarContentKind kind;
  SidebarIconKind iconKind;
  GDrawCommandImage *icon;
  const char *label;
  SidebarTextCandidates primary;
  SidebarTextCandidates secondary;
  bool isLoading;
} SidebarContent;

// the colors a sidebar region draws itself with; regions come in a
// "standard" and a "dark" appearance, each described by one of these
typedef struct {
  GColor background;
  GColor text;
  GColor iconFill;
  GColor iconStroke;
} SidebarPalette;

typedef struct {
  SidebarLayoutMode mode;
  int contentWidth;
} SidebarRenderSpec;

typedef struct {
  int w;
  int h;
} SidebarMeasure;

typedef struct SidebarWidgetDefinition SidebarWidgetDefinition;

struct SidebarWidgetDefinition {
  SidebarWidgetType type;
  SidebarContentKind kind;
  SidebarContent (*getContent)();
  SidebarMeasure (*measureSpecial)(SidebarRenderSpec *spec);
  void (*drawSpecial)(GContext *ctx, GRect frame, SidebarRenderSpec *spec);
};

void SidebarWidgets_init();
void SidebarWidgets_deinit();
void SidebarWidgets_updateFonts();
void SidebarWidgets_updateTime(struct tm* timeInfo);

SidebarRenderSpec SidebarWidgets_makeRenderSpec(SidebarLayoutMode mode,
                                                int contentWidth);
void SidebarWidgets_setActivePalette(const SidebarPalette *palette);
SidebarWidgetDefinition getSidebarWidgetByType(SidebarWidgetType type);
SidebarMeasure SidebarWidgets_measure(SidebarWidgetDefinition *widget,
                                      SidebarRenderSpec *spec);
void SidebarWidgets_draw(GContext *ctx, SidebarWidgetDefinition *widget,
                         GRect frame, SidebarRenderSpec *spec);

#include "utils.h"
#include <pebble.h>

void to_uppercase(char *str) {
  while (*str) {
    *str = toupper((unsigned char)*str);
    str++;
  }
}

struct tm *getCurrentTime() {
  static struct tm timeInfo;
  time_t rawTime;

  time(&rawTime);
  timeInfo = *localtime(&rawTime);

  return &timeInfo;
}

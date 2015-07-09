#include "daypart.h"

bool daypart_containsTime(const Daypart* this, int minute) {
  // check inclusively for the start minute
  // this ensures that dayparts start on their hour, and not the minute after
  return (this->startMinute <= minute && this->endMinute > minute);
}
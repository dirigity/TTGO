#ifndef ttgoGlovalDeclarations
#define ttgoGlovalDeclarations

// Must be defined before include LilyGoWatch librarie
// Uncomment the line corresponding your T-Watch
// #define LILYGO_WATCH_2019_WITH_TOUCH     // To use T-Watch2019 with touchscreen, please uncomment this line
// #define LILYGO_WATCH_2019_NO_TOUCH    // To use T-Watch2019 Not touchscreen , please uncomment this line
#define LILYGO_WATCH_2020_V1 //To use T-Watch2020, please uncomment this line


#define LILYGO_WATCH_HAS_MOTOR
#define LILYGO_WATCH_LVGL

#include <LilyGoWatch.h>

TTGOClass *ttgo;

struct tPermanent
{
    int brightness = 255;
    bool carillon = true;
};

RTC_DATA_ATTR tPermanent permanent;

bool drawn = false;

#endif


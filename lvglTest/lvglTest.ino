#define LILYGO_WATCH_2020_V1 //To use T-Watch2020, please uncomment this line
#define LILYGO_WATCH_LVGL

#include <LilyGoWatch.h>
#include "RickAstley.c"

TTGOClass *ttgo;

LV_IMG_DECLARE(RickAstley);

void setup(){
    ttgo = TTGOClass::getWatch();
    ttgo->begin();

    ttgo->lvgl_begin();
    ttgo->openBL();

    lv_obj_t *img1 = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img1, &RickAstley);
    lv_img_set_zoom(img1, 128);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, -60, -60);
}
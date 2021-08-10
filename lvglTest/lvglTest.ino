#define LILYGO_WATCH_2020_V1 //To use T-Watch2020, please uncomment this line
#define LILYGO_WATCH_LVGL

#include <LilyGoWatch.h>
#include "RickAstley.cpp"

TTGOClass *ttgo;

int w, h;
lv_obj_t *img1;

LV_IMG_DECLARE(RickAstley);

double zoom = 0.25;

void setup()
{
    Serial.begin(115200);

    ttgo = TTGOClass::getWatch();
    ttgo->begin();

    ttgo->lvgl_begin();
    ttgo->openBL();

    w = ttgo->tft->width();
    h = ttgo->tft->height();

    img1 = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img1, &RickAstley);
    
    
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 0);
    //lv_img_set_pivot(img1, h/2, w/2)
    //lv_align_t
}

double d (double x0, double y0, double x1, double y1){
    int a = x0 - x1;
    int b = y0 - y1;
    return sqrt(a*a + b*b);
}

void loop()
{
    int16_t touchX, touchY;
    bool touching = ttgo->getTouch(touchX, touchY);
    if (touching)
    {
        zoom = 0.25 + (d(touchX, touchY, w/2, h/2)) / 100;
        lv_img_set_zoom(img1, LV_IMG_ZOOM_NONE * zoom);
        lv_obj_align(img1, NULL, LV_ALIGN_CENTER, touchX - RickAstley.header.w / 8, touchY - RickAstley.header.h / 8);
    }

    lv_task_handler();
}
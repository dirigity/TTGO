/*

    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!
    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!
    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!
    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!
    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!
    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!
    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!
    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!
    NO EJECUTAR CERCA DE LAS 12 DE LA NOCHE!!!!

*/


#define LILYGO_WATCH_2020_V1 

#include <LilyGoWatch.h>

TTGOClass *ttgo;

static uint8_t conv2d(const char *p)
{
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}


void getCompilationTime (int &h, int &m, int &s){
    h = conv2d(__TIME__);
    m = conv2d(__TIME__ + 3);
    s = conv2d(__TIME__ + 6);
}

void setup() {
  ttgo = TTGOClass::getWatch();
  ttgo->begin();

  int h,m,s;
  getCompilationTime(h,m,s);

  s+= 16; // lo que tarda en arrancar, dont ask why, it is just what it is equisde

  
  if(s >= 60){
    s -= 60;
    m++;
  }

  if(m >= 60){
    m -= 60;
    h++;
  }
  
  
  int year = 2021;
  int month = 8;
  int day = 4;
  ttgo->rtc->setDateTime(year, month, day, h, m, s);

  Serial.begin(115200);
  
  
}

void loop(){
  Serial.println(ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_HMS) );
  //Serial.println(ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_DD_MM_YYYY) );
}

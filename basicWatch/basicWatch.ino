// Must be defined before include LilyGoWatch librarie
// Uncomment the line corresponding your T-Watch
// #define LILYGO_WATCH_2019_WITH_TOUCH     // To use T-Watch2019 with touchscreen, please uncomment this line
// #define LILYGO_WATCH_2019_NO_TOUCH    // To use T-Watch2019 Not touchscreen , please uncomment this line
#define LILYGO_WATCH_2020_V1 //To use T-Watch2020, please uncomment this line
#define LILYGO_WATCH_HAS_MOTOR

#include <LilyGoWatch.h>

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

TTGOClass *ttgo;

const int SleepingHours = 10;
const int timeToSleep = 23;
const int timeToWakeUp = 9;

const double secondsMstart = .15;
const double secondsMend = .3;

const double minuteMstart = .35;
const double minuteMend = .5;

const double hourMstart = .55;
const double hourMend = .75;

const double battMstart = .8;
const double battMend = .9;

const double ringMstart = .95;
const double ringMend = .99;

// calculations

const double midSecondsM = (secondsMstart + secondsMend) / 2;
const double secondsMThickness = secondsMend - secondsMstart;

const double midMinuteM = (minuteMstart + minuteMend) / 2;
const double minuteMThickness = minuteMend - minuteMstart;

const double midHourM = (hourMstart + hourMend) / 2;
const double hourMThickness = hourMend - hourMstart;

const double midBattM = (battMstart + battMend) / 2;
const double battMThickness = battMend - battMstart;

int planedDeepSleepTime = 0;
int planedScreenSleepTime = 0;
int planedButtonCoolDown = 0;
int w, h;

typedef enum
{
  none,
  button
} tInterrupt;
tInterrupt interrupt = none;

bool operator>(RTC_Date a, RTC_Date b)
{
  if (a.year > b.year)
  {
    return true;
  }
  else if (a.year == b.year)
  {
    if (a.month > b.month)
    {
      return true;
    }
    else if (a.month == b.month)
    {
      if (a.day > b.day)
      {
        return true;
      }
      else if (a.day == b.day)
      {
        if (a.hour > b.hour)
        {
          return true;
        }
        else if (a.hour == b.hour)
        {
          if (a.minute > b.minute)
          {
            return true;
          }
          else
          {
            if (a.minute == b.minute)
            {
              if (a.second > b.second)
              {
                return true;
              }
            }
          }
        }
      }
    }
  }

  return false;
}

typedef enum
{
  laucher,
  flashLight,
  watch
} tApp;
tApp app = watch;

int createRGB(int r, int g, int b)
{

  return ttgo->tft->color565(r, g, b);
  //return ((r & 31) << 11) + ((g & 31) << 6) + (b & 31);
}

bool drawn = false;

void setup()
{

  Serial.begin(115200);

  // Get Watch object and set up the display
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->motor_begin();
  ttgo->openBL();

  ttgo->bma->enableAccel();
  ttgo->bma->enableWakeupInterrupt();
  ttgo->bma->enableTiltInterrupt();
  // variable initialitation

  w = ttgo->tft->width();
  h = ttgo->tft->height();

  // turn on/off

  pinMode(AXP202_INT, INPUT_PULLUP); // button
  attachInterrupt(
      AXP202_INT, []
      { interrupt = button; },
      FALLING);

  // pinMode(RTC_INT_PIN, INPUT_PULLUP); // timer alarm
  // attachInterrupt(
  //     RTC_INT_PIN, []
  //     {
  //       interrupt = alarm;
  //     },
  //     FALLING);

  // TOUCH SCREEN  Wakeup source
  //esp_sleep_enable_ext1_wakeup(GPIO_SEL_38, ESP_EXT1_WAKEUP_ALL_LOW);
  // PEK KEY  Wakeup source
  esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);

  //!Clear IRQ unprocessed  first

  ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
  ttgo->power->clearIRQ();

  interaction();
  planedButtonCoolDown = getUsableTime() + 5;
  drawn = false;

  Serial.printf("Setup done!! \n");
}

bool asleep(int h)
{
  return !(h > timeToWakeUp && h < timeToSleep);
}

void enterDeepSleep()
{ // manage next automatic wakeup

  Serial.println("starting deep sleep");

  int year, month, day, hour, minute, seconds;
  getTime(year, month, day, hour, minute, seconds);

  int waitTime = 0;

  if (!asleep(hour + 1))
  { // wake up next hour o'Clock
    waitTime = ((60 - minute) * 60 - 60);
  }
  else
  { // wake up tomorrow
    waitTime = ((60 - minute) * 60 + SleepingHours * 3600 - 60);
  }

  Serial.printf("Saldre del deep sleep %u \n", waitTime);

  esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * waitTime);

  esp_deep_sleep_start();
}

void drawText(String t, int x, int y, int size, int font, int col)
{
  ttgo->tft->setTextColor(col);

  ttgo->tft->setTextFont(size);
  ttgo->tft->setTextSize(font);

  ttgo->tft->setCursor(x, y);
  ttgo->tft->println(t);
}

void getTime(int &year, int &month, int &day, int &h, int &m, int &s)
{
  RTC_Date ret = ttgo->rtc->getDateTime();

  year = ret.year;
  month = ret.month;
  day = ret.day;
  h = ret.hour;
  m = ret.minute;
  s = ret.second;
}

void ToggleOnOff()
{
  ttgo->power->readIRQ();
  if (ttgo->power->isPEKShortPressIRQ())
  { // if short click turn off screen
    ttgo->power->clearIRQ();
    ttgo->bl->reverse();

    if (ttgo->bl->isOn())
    {
      setCpuFrequencyMhz(240);
      planedButtonCoolDown = getUsableTime() + 5;
      drawn = false;
    }
    else
    {
      setCpuFrequencyMhz(2);
    }
  }
  else
  { // if long click enter deepSleep
    ttgo->power->clearIRQ();
    enterDeepSleep();
  }
}

void carillon(int h)
{
  while (h > 0)
  {

    Serial.println(h);

    h--;
    ttgo->motor->onec(1000);
    delay(1000);
    interaction();
  }
}

int getUsableTime()
{
  int year, month, day, hour, minute, seconds;
  getTime(year, month, day, hour, minute, seconds);
  return seconds + 60 * (minute + 60 * (hour + 24 * (day + 30 * (month + 12 * (year - 2020)))));
}

void interaction()
{
  int time = getUsableTime();
  planedScreenSleepTime = time + 30;
  planedDeepSleepTime = time + 45;
}

void drawPolarSegment(double angle, double startM, double endM, int col) //, int darkerCol)
{
  int x0 = w / 2 + sin(angle) * startM;
  int y0 = h / 2 - cos(angle) * startM;
  int x1 = w / 2 + sin(angle) * endM;
  int y1 = h / 2 - cos(angle) * endM;

  ttgo->tft->drawLine(x0, y0, x1, y1, col);
  ttgo->tft->drawLine(x0 + 1, y0, x1 + 1, y1, col);
  ttgo->tft->drawLine(x0, y0 + 1, x1, y1 + 1, col);

  // const double ditheringStripes = 5.;
  // const double dithering = .5; // debug thing, leave at 1

  // if (col != darkerCol)
  //   for (double i = .5 + sin(angle * 12345.5678) / 2; i < ditheringStripes * dithering; i++)
  //   {
  //     double m = startM + ((endM - startM) * i / ditheringStripes);
  //     int x = w / 2 + sin(angle) * m;
  //     int y = h / 2 - cos(angle) * m;
  //     ttgo->tft->drawPixel(x, y, darkerCol);
  //     ttgo->tft->drawPixel(x + 1, y, darkerCol);
  //     ttgo->tft->drawPixel(x, y + 1, darkerCol);
  //   }

  //ttgo->tft->drawLine(x0, y0, x1, y1, col);

  // int x0_ = x0 + (x0 < x1 ? 1: -1);
  // int x1_ = x1 + (x1 < x0 ? 1: -1);

  // ttgo->tft->drawLine(x0_, y0, x1_, y1, col);

  // int y0_ = y0 + (y0 < y1 ? 1 : -1);
  // int y1_ = y1 + (y1 < y0 ? 1 : -1);

  // ttgo->tft->drawLine(x0, y0_, x1, y1_, col);
}

double angle(double a)
{
  while (a > 2 * PI)
  {
    a -= 2 * PI;
  }
  while (a < 0)
  {
    a += 2 * PI;
  }
  return a;
}

double capRoundFunctionCeroToOne(double i)
{
  return sqrt(1 - (i * i));
}

double CapRoundness(double in, double midRad, double Thickness)
{

  double perimeter = 2 * PI * midRad;
  double perimeterRadiousRelation = 1 - (Thickness / perimeter);

  if (in < perimeterRadiousRelation)
  {
    return 1;
  }
  else
  {
    //return .5;
    double i = (in - perimeterRadiousRelation) / (Thickness / perimeter);

    return capRoundFunctionCeroToOne(i);
  }
}

double max(double a, double b)
{
  return a > b ? a : b;
}

void manageDisc(double clockAngle, double timeAngle, double midsM, double MThickness, int r, int g, int b)
{
  double Intensity = angle(clockAngle - timeAngle) / (2 * PI);
  //double Intensity_ = max(0., Intensity - 0.01);
  double CurrentCapRoundness = CapRoundness(Intensity, midsM, MThickness);
  double Mstart_ = midsM - MThickness / 2 * CurrentCapRoundness;
  double Mend_ = midsM + MThickness / 2 * CurrentCapRoundness;

  drawPolarSegment(clockAngle, h / 2 * Mstart_, h / 2 * Mend_, createRGB(Intensity * r, Intensity * g, Intensity * b)); //, createRGB(Intensity_ * r, Intensity_ * g, Intensity_ * b));
}

double secondDrawingAngle = 0;
double minuteDrawingAngle = 0;
double hourDrawingAngle = 0;
double battDrawingAngle = 0;

double battAngle = 0;

int startClickX = -1;
int startClickY = -1;

bool finguerDown = false;

void onFinguerDown()
{
  Serial.println("finguer down");
}

void onFinguerUp()
{
  Serial.println("finguer up");
}

void loop()
{
  int year, month, day, hour, minute, seconds, UsableTime = getUsableTime();
  getTime(year, month, day, hour, minute, seconds);
  // interrupt manager
  {
    switch (interrupt)
    {
    case none:
      break;
    case button:
      if (planedButtonCoolDown < UsableTime)
      {
        Serial.printf("click \n");
        interaction();
        ToggleOnOff();
      }

      break;
    }

    ttgo->power->clearIRQ();
    interrupt = none;
  }
  // touch manager
  {
    int16_t touchX, touchY;
    bool touching = ttgo->getTouch(touchX, touchY);
    if (touching)
    {
      interaction();
      Serial.printf("x: %u, y: %u \n", touchX, touchY);

      if (!finguerDown)
      {
        onFinguerDown();
        finguerDown = true;
      }
    }
    else
    {
      if (finguerDown)
      {
        finguerDown = false;
        onFinguerUp();
      }
    }
  }

  // app managing and ploting
  if (ttgo->bl->isOn())
  {
    switch (app)
    {
    case laucher:

      // batteryLevel
      // time
      // steps??
      // notifications maybe kind of probably???

      break;

    case watch:

      // sleep after some time without activity;
      {
        if (planedScreenSleepTime < UsableTime)
        {
          ttgo->bl->off();
          setCpuFrequencyMhz(2);
        }

        if (planedDeepSleepTime < UsableTime)
        { // 30s despues de apagar la pantalla entrar en sueño profundo
          enterDeepSleep();
        }
      }

      // draw corona
      if (!drawn)
      {
        // Serial.println("[START] drawing corona");
        int ringMmid = (ringMend + ringMstart) / 2 * h / 2;

        ttgo->tft->fillCircle(w / 2, h / 2, h * ringMend / 2, createRGB(50, 50, 50));
        ttgo->tft->fillCircle(w / 2, h / 2, h * ringMstart / 2, 0x000000);

        for (int i = 0; i < 60; i++)
        {
          double angle = 2 * PI / 60 * i;
          ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 2, 0x000000);
          // Serial.println(double(h) * sin(angle));
        }

        for (int i = 0; i < 12; i++)
        {
          double angle = 2 * PI / 12 * i;
          if (i % 3 == 0)
            ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 4, createRGB(200, 200, 200));
          else
            ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 4, createRGB(150, 150, 150));

          // Serial.println(double(h) * sin(angle));
        }

        // Serial.println("[DONE] drawing corona");
      }

      double secondAngle = double(seconds) / 60. * 2. * PI;
      double minuteAngle = double(minute) / 60. * 2. * PI + secondAngle / 60.;
      double hourAngle = double(hour) / 12. * 2. * PI + minuteAngle / 12.;
      if (battAngle == 0)
      {
        battAngle = (ttgo->power->getBattPercentage() / 100. * 2. * PI);
      }
      else
      {
        battAngle = battAngle * 0.95 + (ttgo->power->getBattPercentage() / 100. * 2. * PI) * 0.05;
      }

      // draw gradient circles
      {
        for (int i = 0; i < 200; i++)
        {
          secondDrawingAngle += 0.03123123;
          secondDrawingAngle = angle(secondDrawingAngle);
          manageDisc(secondDrawingAngle, secondAngle, midSecondsM, secondsMThickness, 0, 0, 255);
        }

        for (int i = 0; i < (drawn ? 40 : 300); i++)
        {
          minuteDrawingAngle += 0.02123123;
          minuteDrawingAngle = angle(minuteDrawingAngle);
          manageDisc(minuteDrawingAngle, minuteAngle, midMinuteM, minuteMThickness, 0, 0, 255);
        }

        for (int i = 0; i < (drawn ? 40 : 700); i++)
        {
          hourDrawingAngle += 0.01123123;
          hourDrawingAngle = angle(hourDrawingAngle);
          manageDisc(hourDrawingAngle, hourAngle, midHourM, hourMThickness, 0, 0, 255);
        }

        for (int i = 0; i < (drawn ? 40 : 700); i++)
        {
          battDrawingAngle += 0.01123123;
          battDrawingAngle = angle(battDrawingAngle);
          if (battAngle < 1)
            manageDisc(battDrawingAngle, battAngle, midBattM, battMThickness, 255, 0, 0);
          else
            manageDisc(battDrawingAngle, battAngle, midBattM, battMThickness, 0, 0, 255);
        }
      }

      drawn = true;

      break;
    }
  }

  // carillón

  if (minute == 59)
  {
    interaction();
  }

  if (seconds % 20 == 0)
  {
    int dongs = hour;
    if (dongs > 13)
    {
      dongs -= 12;
    }
    if (dongs == 0)
    {
      dongs = 12;
    }
    carillon(dongs);
  }

  //Serial.printf(ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_HMS));
}

//TFT_BLACK(#000000)
//TFT_NAVY (#000080)
//TFT_DARKGREEN (#008000)
//TFT_DARKCYAN (#008080)
//TFT_MAROON (#80000)
//TFT_PURPLE #800080)
//TFT_OLIVE (#808000)
//TFT_LIGHTGREY (#D3D3D3)
//TFT_DARKGREY (#808080)
//TFT_BLUE (#0000FF)
//TFT_GREEN (#00FF00)
//TFT_CYAN (#00FFFF)
//TFT_RED (#FF0000)
//TFT_MAGENTA (#FF00FF)
//TFT_YELLOW (#FFFF00)
//TFT_WHITE (#FFFFFF)
//TFT_ORANGE (#FFB400)
//TFT_GREENYELLOW (#B4FF00)
//TFT_PINK (#FFC0CB)
//TFT_BROWN (#964B00)
//TFT_GOLD (#FFD700)
//TFT_SILVER (#C0C0C0)
//TFT_SKYBLUE (#87CEEB)
//TFT_VIOLET (#B42EE2)

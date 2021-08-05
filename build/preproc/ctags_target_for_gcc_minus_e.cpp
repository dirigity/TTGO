# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWathc\\basicWathc.ino"
// Must be defined before include LilyGoWatch librarie
// Uncomment the line corresponding your T-Watch
// #define LILYGO_WATCH_2019_WITH_TOUCH     // To use T-Watch2019 with touchscreen, please uncomment this line
// #define LILYGO_WATCH_2019_NO_TOUCH    // To use T-Watch2019 Not touchscreen , please uncomment this line



# 9 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWathc\\basicWathc.ino" 2



const double ANGLE_TO_COLOR = 1 / (2 * 3.1415926535897932384626433832795) * 32;

TTGOClass *ttgo;

const int SleepingHours = 10;
const int timeToSleep = 23;
const int timeToWakeUp = 9;

// const double secondsMstart = 0.2;
// const double secondsMend = 0.4;
// const double minuteMstart = 0.4;
// const double minuteMend = 0.7;
// const double hourMstart = 0.7;
// const double hourMend = 1;

const double secondsMstart = 0.7;
const double secondsMend = 1;
const double minuteMstart = 0.4;
const double minuteMend = 0.7;
const double hourMstart = 0.2;
const double hourMend = 0.4;

const double midSecondsM = (secondsMstart + secondsMend) / 2;
const double secondsMThickness = secondsMend - secondsMstart;

const double midMinuteM = (minuteMstart + minuteMend) / 2;
const double minuteMThickness = minuteMend - minuteMstart;

const double midHourM = (hourMstart + hourMend) / 2;
const double hourMThickness = hourMend - hourMstart;

int planedDeepSleepTime = 0;
int planedScreenSleepTime = 0;
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

unsigned long createRGB(int r, int g, int b)
{
  return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

bool drawn = false;

void setup()
{

  Serial.begin(115200);

  // Get Watch object and set up the display
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();

  ttgo->bma->enableAccel();
  ttgo->bma->enableWakeupInterrupt();
  ttgo->bma->enableTiltInterrupt();
  // variable initialitation

  w = ttgo->tft->width();
  h = ttgo->tft->height();

  // turn on/off

  pinMode(35, 0x05); // button
  attachInterrupt(
      35, []
      { interrupt = button; },
      0x02);

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
  esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1)<<35)) /*!< Pin 35 selected */, ESP_EXT1_WAKEUP_ALL_LOW);

  //!Clear IRQ unprocessed  first

  ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
  ttgo->power->clearIRQ();

  interaction();
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
    waitTime = ((60 - minute) * 60 - 15);
  }
  else
  { // wake up tomorrow
    waitTime = ((60 - minute) * 60 + SleepingHours * 3600 - 15);
  }

  Serial.printf("Saldre del deep sleep %u \n", waitTime);

  esp_sleep_enable_timer_wakeup(1000000ULL /* Conversion factor for micro seconds to seconds */ * waitTime);

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
  while (h != 0)
  {
    h--;
    ttgo->motor->onec(1000);
    delay(1000);
  }
  interaction();
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
  planedScreenSleepTime = time + 15;
  planedDeepSleepTime = time + 30;
}

void drawPolarSegment(double angle, double startM, double endM, int col)
{
  int a = w / 2 + sin(angle) * startM;
  int b = h / 2 - cos(angle) * startM;
  int c = w / 2 + sin(angle) * endM;
  int d = h / 2 - cos(angle) * endM;

  ttgo->tft->drawLine(a, b, c, d, col);

  ttgo->tft->drawLine(a + 1, b, c + 1, d, col);

  ttgo->tft->drawLine(a - 1, b, c - 1, d, col);

  ttgo->tft->drawLine(a, b + 1, c, d + 1, col);

  ttgo->tft->drawLine(a, b - 1, c, d - 1, col);
}

double i = 0;

double clockAngle = 0;

double angle(double a)
{
  while (a > 2 * 3.1415926535897932384626433832795)
  {
    a -= 2 * 3.1415926535897932384626433832795;
  }
  while (a < 0)
  {
    a += 2 * 3.1415926535897932384626433832795;
  }
  return a;
}

double CapRoundness(double in)
{
  Serial.println(in);
  if (in < 30)
  {
    return 1;
  }
  else
  {
    double i = (in - 30) / 2;

    return sqrt(1 - (i * i));
  }
}

void manageDisc(double clockAngle, double timeAngle, double midsM, double MThickness)
{
  double Intensity = angle(clockAngle - timeAngle) * ANGLE_TO_COLOR;
  double CurrentCapRoundness = CapRoundness(Intensity);
  // Serial.println(CapRoundness);
  double Mstart_ = midsM - MThickness / 2 * CurrentCapRoundness;
  double Mend_ = midsM + MThickness / 2 * CurrentCapRoundness;
  drawPolarSegment(clockAngle, h / 2 * Mstart_, h / 2 * Mend_, createRGB(0, 0, Intensity));
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
      Serial.printf("click \n");
      interaction();
      ToggleOnOff();
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
    }
  }
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

      double seccondAngle = double(seconds) / 60. * 2. * 3.1415926535897932384626433832795;
      double minuteAngle = double(minute) / 60. * 2. * 3.1415926535897932384626433832795 + seccondAngle / 60.;
      double hourAngle = double(hour) / 12. * 2. * 3.1415926535897932384626433832795 + minuteAngle / 60.;

      //hour, minute, seconds
      // Serial.println(seconds);
      // Serial.println(seccondAngle);

      // Serial.println(minute);
      // Serial.println(minuteAngle);

      // Serial.println(hour);
      // Serial.println(hourAngle);
      // Serial.println("---------");
      //int fullBlue = createRGB(0, 0, 31);

      // int a_ = w / 2 + w / 2 * sin(seccondAngle) * ((secondsMstart + secondsMend) / 2.);
      // int b_ = h / 2. - h / 2 * cos(seccondAngle) * ((secondsMstart + secondsMend) / 2.);
      // int c_ = h / 4 * (secondsMend - secondsMstart) - 1;

      // int d_ = w / 2 + w / 2 * sin(minuteAngle) * ((minuteMstart + minuteMend) / 2.);
      // int e_ = h / 2. - h / 2 * cos(minuteAngle) * ((minuteMstart + minuteMend) / 2.);
      // int f_ = h / 4 * (minuteMend - minuteMstart) - 1;

      // int g_ = w / 2 + w / 2 * sin(hourAngle) * ((hourMstart + hourMend) / 2.);
      // int h_ = h / 2. - h / 2 * cos(hourAngle) * ((hourMstart + hourMend) / 2.);
      // int i_ = h / 4 * (hourMend - hourMstart) - 1;

      for (int j = 0; j < 10 || !drawn; j++)
      {
        clockAngle += 0.01112345678;

        clockAngle = angle(clockAngle);

        if (j > 700)
        {
          drawn = true;
        }

        manageDisc(clockAngle, seccondAngle, midSecondsM, secondsMThickness);
        manageDisc(clockAngle, minuteAngle, midMinuteM, minuteMThickness);
        manageDisc(clockAngle, hourAngle, midHourM, hourMThickness);

        // double secondIntensity = angle(clockAngle - seccondAngle) * ANGLE_TO_COLOR;
        // double secondCapRoundness = CapRoundness(secondIntensity);
        // // Serial.println(secondCapRoundness);
        // double secondsMstart_ = midSecondsM - secondsMThickness / 2 * secondCapRoundness;
        // double secondsMend_ = midSecondsM + secondsMThickness / 2 * secondCapRoundness;

        // double minuteIntensity = angle(clockAngle - minuteAngle) * ANGLE_TO_COLOR;
        // double minuteCapRoundness = CapRoundness(minuteIntensity);
        // double minuteMstart_ = midMinuteM - minuteMThickness / 2 * minuteCapRoundness;
        // double minuteMend_ = midMinuteM + minuteMThickness / 2 * minuteCapRoundness;

        // double hourIntensity = angle(clockAngle - hourAngle) * ANGLE_TO_COLOR;
        // double hourCapRoundness = CapRoundness(hourIntensity);
        // double hourMstart_ = midHourM - hourMThickness / 2 * hourCapRoundness;
        // double hourMend_ = midHourM + hourMThickness / 2 * hourCapRoundness;

        // drawPolarSegment(clockAngle, h / 2 * secondsMstart_, h / 2 * secondsMend_, createRGB(0, 0, secondIntensity));
        // drawPolarSegment(clockAngle, h / 2 * minuteMstart_, h / 2 * minuteMend_, createRGB(0, 0, minuteIntensity));
        // drawPolarSegment(clockAngle, h / 2 * hourMstart_, h / 2 * hourMend_, createRGB(0, 0, hourIntensity));

        // ttgo->tft->fillCircle(a_, b_, c_, fullBlue);
        // ttgo->tft->fillCircle(d_, e_, f_, fullBlue);
        // ttgo->tft->fillCircle(g_, h_, i_, fullBlue);

        // int a = w / 2 + sin(angle) * startM;
        // int b = h / 2 - cos(angle) * startM;
        // int c = w / 2 + sin(angle) * endM;
        // int d = h / 2 - cos(angle) * endM;

        // ttgo->tft->drawLine(a, b, c, d, col);
      }

      break;
    }
  }

  // carillón

  if (minute == 59)
  {
    interaction();
  }

  if (minute == 0 && seconds == 0)
  {
    carillon(hour);
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

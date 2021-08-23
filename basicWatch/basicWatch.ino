// Must be defined before include LilyGoWatch librarie
// Uncomment the line corresponding your T-Watch
// #define LILYGO_WATCH_2019_WITH_TOUCH     // To use T-Watch2019 with touchscreen, please uncomment this line
// #define LILYGO_WATCH_2019_NO_TOUCH    // To use T-Watch2019 Not touchscreen , please uncomment this line
#define LILYGO_WATCH_2020_V1 //To use T-Watch2020, please uncomment this line
#define LILYGO_WATCH_HAS_MOTOR
#define LILYGO_WATCH_LVGL

#include <LilyGoWatch.h>

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

TTGOClass *ttgo;

LV_IMG_DECLARE(RickAstley);

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

const int MaxBrightness = 255;
const int MinBrightness = 10;
const int brightnessBarMargin = 10;

struct tPermanent
{
  int brightness = 255;
  bool carillon = true;
};

RTC_DATA_ATTR tPermanent permanent;

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
  launcher,

  watch,
  flashLight,
  calculator,
  countdown,
  timer,
  teamScores,
  controlPannel, // brigtness(rtc_mem), carillon(rtc_mem), battery stats
  unitConversor,

} tApp;
tApp app = watch;

const int appCount = 9;

const String AppToString[appCount] = {
    "launcher",

    "watch",
    "flashLight",
    "calculator", // demos and scientific
    "countdown",
    "timer",
    "teamScores",
    "controlPannel", // brigtness(rtc_mem), carillon(rtc_mem), battery stats
    "unitConversor",
};

struct tBox
{
  int x0;
  int y0;
  int x1;
  int y1;
};

struct tButton
{
  tBox box;
  void (*function)();
  int color;
  String text;
  int textColor;
  bool pressed;
};

const int MAX_ONSCREEN_BUTTONS = 5;
struct tButtonList
{
  tButton buttons[MAX_ONSCREEN_BUTTONS];
  int counter = 0;
};

tButtonList buttonList;

void createButton(struct tBox box, void (*function)(), int color, String text, int textColor)
{
  struct tButton wip = {box, function, color, text, textColor, false};
  buttonList.buttons[buttonList.counter] = wip;
  buttonList.counter++;
}

void clearButtons()
{
  buttonList.counter = 0;
}

bool insideBox(int x, int y, tBox box)
{
  return x > box.x0 && x < box.x1 && y > box.y0 && y < box.y1;
}

int createRGB(int r, int g, int b)
{
  return ttgo->tft->color565(r, g, b);
  //return ((r & 31) << 11) + ((g & 31) << 6) + (b & 31);
}

const int RED_CANCEL = createRGB(255, 70, 70);
const int GREEN_ALLOW = createRGB(70, 255, 70);

void destructurateRGB(int col, int &r, int &g, int &b)
{
  b = col & 0x1F;
  col >> 5;
  g = col & 0x3F;
  col >> 6;
  r = col & 0x1F;

  b = b << 3;
  g = g << 2;
  r = r << 3;
}

bool drawn = false;

void setup()
{

  Serial.begin(115200);

  // Get Watch object and set up the display
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->motor_begin();
  // ttgo->lvgl_begin();
  ttgo->openBL();

  //ttgo->bma->enableAccel();
  ttgo->bma->enableWakeupInterrupt();
  //ttgo->bma->enableTiltInterrupt();
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
  if (permanent.carillon)
  {
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
  }
  else
  {
    Serial.printf("no saldre del deep sleep automaticamente porque el carillon esta desactivado \n");
  }

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

  drawn = false;
  app = watch;

  ttgo->power->readIRQ();
  if (ttgo->power->isPEKShortPressIRQ())
  { // if short click turn off screen
    ttgo->power->clearIRQ();
    ttgo->bl->reverse();

    if (ttgo->bl->isOn())
    {
      setCpuFrequencyMhz(240);
      planedButtonCoolDown = getUsableTime() + 5;
    }
    else
    {
      planedDeepSleepTime = getUsableTime() + 15;
      setCpuFrequencyMhz(80);
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
  if (!permanent.carillon)
    return;
  ttgo->motor->onec(1000);
  delay(2000);
  // base 1
  //while (h > 0)
  // {
  //   //Serial.println(h);

  //   h--;
  //   ttgo->motor->onec(100);
  //   delay(400);
  // }
  // base 2

  while (h > 0)
  {
    if (h % 2 == 1)
    {
      ttgo->motor->onec(700);
      delay(1000);
    }
    else
    {
      ttgo->motor->onec(100);
      delay(1000);
    }
    h /= 2;
  }
  interaction();
}

int getUsableTime()
{
  return millis() / 1000;
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

template <class T>
T maximum(T a, T b)
{
  return a > b ? a : b;
}

template <class T>
T minimum(T a, T b)
{
  return a < b ? a : b;
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

void drawButtons()
{
  for (int i = 0; i < buttonList.counter; i++)
  {
    const int borderSize = 2;
    const tButton b = buttonList.buttons[i];
    int r, g, blue;
    destructurateRGB(b.color, r, g, blue);
    int k = 40;
    int borderCol = createRGB(max(0, r - k), max(0, g - k), max(0, blue - k));
    int fillCol = b.color;

    if (b.pressed)
    {
      int kk = borderCol;
      borderCol = fillCol;
      fillCol = kk;
    }

    ttgo->tft->fillRect(b.box.x0, b.box.y0, b.box.x1 - b.box.x0, b.box.y1 - b.box.y0, borderCol);
    ttgo->tft->fillRect(b.box.x0 + borderSize, b.box.y0 + borderSize, b.box.x1 - b.box.x0 - borderSize * 2, b.box.y1 - b.box.y0 - borderSize * 2, fillCol);
    drawText(b.text, borderSize + 3 + b.box.x0, b.box.y0, 2, 2, b.textColor);
  }
}
double secondDrawingAngle = 0;
double minuteDrawingAngle = 0;
double hourDrawingAngle = 0;
double battDrawingAngle = 0;

double battAngle = 0;

int startClickX = -1;
int startClickY = -1;

int lastTouchX = -1;
int lastTouchY = -1;

int selected = -1;

bool fingerDown = false;

void onfingerDown(int x, int y)
{
  Serial.printf("finger down x: %d y:%d \n", x, y);
  startClickX = x;
  startClickY = y;
}

void onfingerDrag(int x, int y)
{

  bool InsideAButton = false;

  for (int i = 0; i < buttonList.counter; i++)
  {
    if (insideBox(x, y, buttonList.buttons[i].box))
    {
      if (!buttonList.buttons[i].pressed)
      {
        InsideAButton = true;
        buttonList.buttons[i].pressed = true;
      }
      //Serial.println("se conoce que estamos pulsando un botton o algo");
    }
    else
    {
      if (buttonList.buttons[i].pressed)
      {
        InsideAButton = true;
        buttonList.buttons[i].pressed = false;
      }
    }
  }

  if (InsideAButton)
  {
    drawButtons();
  }
  else
  {
    if (app == launcher)
    {
      int borderTouchMargin = 20;
      double touchY = double(y) * double(h + borderTouchMargin * 2) / h - borderTouchMargin;

      touchY = minimum(maximum(0., touchY), double(h - 1));
      //Serial.println(touchY);
      int newSelected = int(touchY / double(h) * (appCount - 1));
      if (selected != newSelected)
      {
        selected = newSelected;
        drawn = false;
        Serial.printf("current selection is %d \n", selected);
      }
    }
    if (app == controlPannel) // brightness scrool
    {
      if (y > 40 && y < 100)
      {
        double normalizedPress = double(x - brightnessBarMargin) / double(w - brightnessBarMargin * 2);

        normalizedPress = minimum(maximum(normalizedPress, 0.), 1.);

        int newValue = MinBrightness + (MaxBrightness - MinBrightness) * normalizedPress;
        //Serial.println(newValue);

        if (permanent.brightness != newValue)
        {
          permanent.brightness = newValue;
          drawn = false;
        }
      }
    }
  }
}

void onfingerUp(int x, int y)
{

  Serial.printf("finger up x: %d y:%d \n", x, y);
  //Serial.printf("vertical distance entre touches: %d \n ", y-startClickY);

  bool InsideAButton = false;

  for (int i = 0; i < buttonList.counter; i++)
  {
    if (insideBox(x, y, buttonList.buttons[i].box))
    {
      buttonList.buttons[i].function();
      InsideAButton = true;
    }
  }

  if (!InsideAButton)
  {
    if (app == watch)
    {
      if (y - startClickY > 80)
      {
        enterDeepSleep();
      }
    }
    if (app != launcher)
    {
      if (y - startClickY < -80)
      {
        app = launcher;
        Serial.println("ahora estamos en el launcher");
        drawn = false;
        selected = -1;
        return;
      }
    }
    if (app == flashLight)
    {
      app = launcher;
      drawn = false;
      selected = -1;
      ttgo->bl->adjust(permanent.brightness);

      return;
    }
    if (app == launcher && selected >= 0)
    {
      app = tApp(selected + 1);
      Serial.printf("Cambiando a app: %s \n", AppToString[app].c_str());
      selected = -1;
      drawn = false;
      return;
    }
  }
}

int startTimerTime = 0;
int stopedTimerTime = 0;
int currentlyDrawnSecondsSinceStart = 0;
int TimerRuning = false;
int LapTimes[5] = {0};
int lastLapTime = -1;

const int MaxStoredLaps = 5;
struct tLapList
{
  char laps[MaxStoredLaps][25];
  int counter = 0;
};

tLapList lapList;

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
    {
      Serial.printf("quzas click \n");

      if (planedButtonCoolDown < UsableTime)
      {
        Serial.printf("click  \n");

        interaction();
        ToggleOnOff();
      }

      break;
    }
    }

    ttgo->power->clearIRQ();
    interrupt = none;
  }
  if (ttgo->bl->isOn())
  {
    // touch manager
    {
      int16_t touchX, touchY;
      bool touching = ttgo->getTouch(touchX, touchY);
      if (touching)
      {
        lastTouchX = touchX;
        lastTouchY = touchY;

        interaction();
        //Serial.printf("x: %u, y: %u \n", touchX, touchY);

        if (!fingerDown)
        {
          onfingerDown(touchX, touchY);
          fingerDown = true;
        }

        onfingerDrag(touchX, touchY);
      }
      else
      {
        if (fingerDown)
        {
          fingerDown = false;
          onfingerUp(lastTouchX, lastTouchY);
        }
      }
    }
    // app managing and ploting
    if (!drawn)
    {
      ttgo->bl->adjust(permanent.brightness);
      ttgo->tft->fillScreen(0);
      clearButtons();
    }

    switch (app)
    {

    case calculator:
    {
      break;
    }
    case countdown:
    {
      break;
    }
    case timer:
    {
      const int posX = 20;
      const int posY = 20;
      //Serial.println("timer loop");
      if (!drawn)
      {
        ttgo->tft->fillRect(0, 0, w, h / 4, 0);
        for (int i = 0; i < lapList.counter; i++)
        {
          drawText(String(lapList.laps[i]), 40, 75 + i * 20, 1, 2, 0xFFFF);
        }

        tBox boxStartAndStop = {20, 190, w / 2 - 20, 225};
        tBox boxLapAndReset = {w / 2 + 20, 190, w - 20, 225};
        if (!TimerRuning)
        {
          createButton(
              boxStartAndStop, []
              {
                if (startTimerTime == 0)
                {
                  startTimerTime = getUsableTime();
                }
                else
                {
                  startTimerTime = getUsableTime() - (stopedTimerTime - startTimerTime);
                }
                TimerRuning = true;
                drawn = false;
                currentlyDrawnSecondsSinceStart = -1;
              },
              GREEN_ALLOW, "Start", 0x0000);
          createButton(
              boxLapAndReset, []
              {
                startTimerTime = 0;
                stopedTimerTime = 0;
                TimerRuning = false;
                drawn = false;
                ttgo->tft->fillRect(0, 0, w, h / 4, 0);
                drawText("00:00:00", posX, posY, 4, 2, 0xFFFF);
                currentlyDrawnSecondsSinceStart = -1;

                lastLapTime = -1;
                lapList.counter = 0;
                ttgo->tft->fillRect(0, 70, w, 110, 0); // erase laps
              },
              RED_CANCEL, "Reset", 0x0000);
        }
        else
        {
          createButton(
              boxStartAndStop, []
              {
                stopedTimerTime = getUsableTime();
                TimerRuning = false;
                drawn = false;
                currentlyDrawnSecondsSinceStart = -1;
              },
              RED_CANCEL, "Stop", 0x0000);
          createButton(
              boxLapAndReset, []
              {
                if (lapList.counter == MaxStoredLaps)
                {

                  for (int i = 1; i < MaxStoredLaps; i++)
                  {
                    int dst = i - 1;
                    int src = i;

                    for (int j = 0; j < 25; j++)
                    {
                      lapList.laps[dst][j] = lapList.laps[src][j];
                    }
                  }

                  lapList.counter--;
                }
                int secondsSinceStart = getUsableTime() - startTimerTime;
                int hoursSinceStart = secondsSinceStart / 3600;
                int minutesSinceStart = (secondsSinceStart % 3600) / 60;
                int seconds = secondsSinceStart % 60;

                int diferenceFromLast = 0;
                if (lastLapTime != -1)
                {
                  diferenceFromLast = getUsableTime() - lastLapTime;
                  sprintf(lapList.laps[lapList.counter], "%02d:%02d:%02d (+%d) ", hoursSinceStart, minutesSinceStart, seconds, diferenceFromLast);
                }
                else
                {
                  sprintf(lapList.laps[lapList.counter], "%02d:%02d:%02d ", hoursSinceStart, minutesSinceStart, seconds);
                }

                lastLapTime = getUsableTime();

                lapList.counter++;

                ttgo->tft->fillRect(0, 70, w, 110, 0); // erase laps

                for (int i = 0; i < lapList.counter; i++)
                {
                  drawText(String(lapList.laps[i]), 40, 75 + i * 20, 1, 2, 0xFFFF);
                }
              },
              createRGB(255, 255, 30), "Lap", 0x0000);
        }
        if (startTimerTime == 0)
        {
          ttgo->tft->fillRect(0, 0, w, h / 4, 0);
          drawText("00:00:00", posX, posY, 4, 2, 0xFFFF);
        }
      }
      int secondsSinceStart;
      if (TimerRuning)
        secondsSinceStart = UsableTime - startTimerTime;
      else
        secondsSinceStart = stopedTimerTime - startTimerTime;

      // Serial.println("secondsSinceStart");
      // Serial.println(secondsSinceStart);
      // Serial.println("TimerRuning");
      // Serial.println(TimerRuning);
      // Serial.println("currentlyDrawnSecondsSinceStart");
      // Serial.println(currentlyDrawnSecondsSinceStart);

      if (secondsSinceStart != currentlyDrawnSecondsSinceStart)
      {
        currentlyDrawnSecondsSinceStart = secondsSinceStart;
        int hoursSinceStart = secondsSinceStart / 3600;
        int minutesSinceStart = (secondsSinceStart % 3600) / 60;
        int seconds = secondsSinceStart % 60;

        char buff[10];
        sprintf(buff, "%02d:%02d:%02d", hoursSinceStart, minutesSinceStart, seconds);
        //printf("[%d] %d:%d:%d \n", secondsSinceStart, hoursSinceStart, minutesSinceStart, seconds);
        //Serial.printf("%s la hora /n",buff)
        ttgo->tft->fillRect(0, 0, w, h / 4, 0);
        drawText(String(buff), posX, posY, 4, 2, 0xFFFF);
      }

      break;
    }
    case teamScores:
    {
      break;
    }
    case controlPannel:
    {
      if (!drawn)
      {
        // carrillon
        tBox box = {10, 10, w - 10, 45};
        if (permanent.carillon)
          createButton(
              box, []
              {
                permanent.carillon = false;
                drawn = false;
              },
              GREEN_ALLOW, "Apagar Carillon", createRGB(0, 0, 0));
        else
          createButton(
              box, []
              {
                permanent.carillon = true;
                drawn = false;
              },
              RED_CANCEL, "Activar Carillon", createRGB(255, 255, 255));

        // brigness bar

        const int barHeight = 70;

        ttgo->tft->drawLine(brightnessBarMargin, barHeight, w - brightnessBarMargin, barHeight, 0xFFFF);

        double normalizedBrightness = (double(permanent.brightness - MinBrightness) / double(MaxBrightness - MinBrightness));
        //Serial.println(normalizedBrightness);
        ttgo->tft->fillCircle(brightnessBarMargin + double(w - (brightnessBarMargin * 2)) * normalizedBrightness, barHeight, 10, 0xFFFF);
        ttgo->bl->adjust(permanent.brightness);

        // exit button
        tBox boxExit = {10, 100, w - 10, 135};
        createButton(
            boxExit, []
            {
              app = launcher;
              drawn = false;
            },
            createRGB(255, 255, 255), "Back", 0x0000);
      }

      break;
    }
    case unitConversor:
    {
      break;
    }
    case flashLight:
    {
      if (!drawn)
      {
        ttgo->bl->adjust(255);
        ttgo->tft->fillScreen(0xFFFF);
      }
      break;
    }
    case launcher:
    {
      if (!drawn)
      {
        int separation = 40;
        int totalSize = (appCount)*separation;
        int overflow = maximum(totalSize - h, 0);
        int offset = maximum(0, overflow / (appCount - 1) * selected);

        for (int i = 1; i < appCount; i++)
        {
          drawText(AppToString[i], 0, (i - 1) * separation - offset, 2, 2, selected + 1 == i ? createRGB(255, 255, 255) : createRGB(100, 100, 100));
        }
      }

      break;
    }
    case watch:
    {

      // sleep after some time without activity;
      {
        if (planedScreenSleepTime < UsableTime)
        {
          ttgo->bl->off();
          setCpuFrequencyMhz(80);
        }

        if (planedDeepSleepTime < UsableTime)
        { // 30s despues de apagar la pantalla entrar en sueño profundo
          enterDeepSleep();
        }
      }

      // draw corona
      if (!drawn)
      {
        //Serial.println("[START] drawing corona");
        int ringMmid = (ringMend + ringMstart) / 2 * h / 2;

        ttgo->tft->fillCircle(w / 2, h / 2, h * ringMend / 2, createRGB(50, 50, 50));
        ttgo->tft->fillCircle(w / 2, h / 2, h * ringMstart / 2, 0x0000);

        for (int i = 0; i < 60; i++)
        {
          double angle = 2 * PI / 60 * i;
          ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 2, 0x0000);
          //Serial.println(double(h) * sin(angle));
        }

        for (int i = 0; i < 12; i++)
        {
          double angle = 2 * PI / 12 * i;
          if (i % 3 == 0)
            ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 4, createRGB(200, 200, 200));
          else
            ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 4, createRGB(150, 150, 150));

          //Serial.println(double(h) * sin(angle));
        }

        //Serial.println("[DONE] drawing corona");
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

      break;
    }
    }
  }

  // carillón
  if (permanent.carillon)
  {
    if (minute == 59)
    {
      interaction();
    }

    if (seconds == 0 && minute == 0)
    {
      int dongs = hour;
      // if (dongs > 13)
      // {
      //   dongs -= 12;
      // }
      // if (dongs == 0)
      // {
      //   dongs = 12;
      // }
      carillon(dongs);
    }
  }

  // pintarBottones
  if (!drawn)
  {
    drawButtons();
  }

  drawn = true;
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

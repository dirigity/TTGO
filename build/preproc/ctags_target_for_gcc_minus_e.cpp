# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino"
# 2 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2

// apps

# 8 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 9 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 10 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 11 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 12 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 13 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 14 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2

void setup()
{
  Serial.begin(115200);
  //Serial.setTimeout(50);

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

  FULL_SCREEN_BOX = {0, 0, w, h};

  RED_CANCEL = createRGB(255, 70, 70);
  GREEN_ALLOW = createRGB(70, 255, 70);

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
  planedButtonCoolDown = getUsableTime() + 5;
  drawn = false;

  Serial.println("Setup done!!");
}

// to rember after release (used at onfingerUp call)
int lastTouchX = -1;
int lastTouchY = -1;

bool fingerDown = false;

void onfingerDown(int x, int y)
{
  //Serial.printf("finger down x: %d y:%d \n", x, y);
  startClickX = x;
  startClickY = y;

  for (int i = 0; i < buttonList.counter; i++)
  {
    if (insideBox(x, y, buttonList.buttons[i].box))
    {
      if (buttonList.buttons[i].listenerType == onDown)
        buttonList.buttons[i].function(x, y);

      buttonList.buttons[i].pressed = false;
      drawButtons();
    }
  }
}

void onfingerDrag(int x, int y)
{
  bool InsideAButton = false;

  for (int i = 0; i < buttonList.counter; i++)
  {
    if (insideBox(x, y, buttonList.buttons[i].box))
    {
      if (buttonList.buttons[i].listenerType == onDrag)
      {
        buttonList.buttons[i].function(x, y);
      }

      if (!buttonList.buttons[i].pressed && buttonList.buttons[i].draw)
      {
        InsideAButton = true;
        buttonList.buttons[i].pressed = true;
        drawButtons();
      }
      //Serial.println("se conoce que estamos pulsando un botton o algo");
    }
    else
    {
      if (buttonList.buttons[i].pressed && buttonList.buttons[i].draw)
      {
        InsideAButton = true;
        buttonList.buttons[i].pressed = false;
        drawButtons();
      }
    }
  }
}

void onfingerUp(int x, int y)
{
  //Logger logger("onFingerUp");

  //logger("finger up x: %d y:%d \n", x, y);
  //logger("vertical distance entre touches: %d \n ", y - startClickY);

  bool InsideAButton = false;

  for (int i = 0; i < buttonList.counter; i++)
  {
    if (insideBox(x, y, buttonList.buttons[i].box))
    {
      if (buttonList.buttons[i].listenerType == onUp)
        buttonList.buttons[i].function(x, y);

      buttonList.buttons[i].pressed = false;
      drawButtons();
      InsideAButton = true;
    }
  }
}

void ManageTouch()
{
  int16_t touchX, touchY;
  bool touching = ttgo->getTouch(touchX, touchY);
  if (touching)
  {
    lastTouchX = touchX;
    lastTouchY = touchY;

    interaction();

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
    ManageTouch();

    // app managing and ploting
    if (!drawn)
    {
      ttgo->bl->adjust(permanent.brightness);
      ttgo->tft->fillScreen(0);
      clearButtons();

      createInterationArea(
          FULL_SCREEN_BOX, onUp, [](int x, int y)
          {
            if (app != launcher)
            {
              if (y - startClickY < -80)
              {
                goToLauncher();
              }
            }
          });
    }

    switch (app)
    {

    case dataMonitor:
      dataMonitorTick(year, month, day, hour, minute, seconds);
      break;

    case turnOff:
      enterDeepSleep();

    case calculator:
      CalculatorTick();
      break;

    case countdown:
      break;

    case timer:
      timerTick(UsableTime);
      break;

    case teamScores:
      TeamScoresTick();
      break;

    case controlPannel:
      controlPannelTick();
      break;

    case unitConversor:
      break;

    case flashLight:

      if (!drawn)
      {
        ttgo->bl->adjust(255);
        ttgo->tft->fillScreen(0xFFFF);
        createInterationArea(
            FULL_SCREEN_BOX, onUp, [](int x, int y)
            {
              ttgo->bl->adjust(permanent.brightness);
              goToLauncher();
            });
      }
      break;

    case launcher:
      launcherTick();
      break;

    case watch:
      watchTick(year, month, day, hour, minute, seconds, UsableTime);
      break;
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
      carillon(dongs);
    }
  }

  // pintarBottones
  if (!drawn)
  {
    //Serial.println("via pintar los botones");
    drawButtons();
    drawn = true;
  }

  if (invalidate)
  {
    invalidate = false;
    drawn = false;
  }
}

// colors
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
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino" 2
# 5 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appControlPannel.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appControlPannel.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appControlPannel.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appDataMonitor.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appDataMonitor.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appDataMonitor.ino" 2




int lastDrawnTime = -1;

void dataMonitorTick(int year, int month, int day, int hour, int minute, int seconds)
{


    if (!drawn)
    {
        const int buffLenght = 100;
        char info[buffLenght];

        snprintf(info, buffLenght,
                 "year: %d  \n"
                 "month: %s %02d \n"
                 "day: %02d \n"
                 "time: %02d:%02d:%02d \n"
                 "power: %.2fV\n"
                 "batt: %02d%%\n"
                 "chargeing: %s\n",
                 year, monthsToWord[month], month, day, hour, minute, seconds, ttgo->power->getBattVoltage() / 1000., getBatteryCorrectedPorcentage(), ttgo->power->isChargeing() ? "True":"False");
        drawText(String(info), 0, 0, 2, 2, 0xFFFF /* 255, 255, 255 */);

        lastDrawnTime = seconds;
    }

    if (lastDrawnTime != seconds)
    {
        invalidate = true;
    }
}
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appLauncher.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appLauncher.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appLauncher.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appOcr.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appOcr.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appOcr.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appTeamScores.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appTeamScores.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appTeamScores.ino" 2




int ScoreA = 0;
int ScoreB = 0;

void reset()
{
    ScoreA = 0;
    ScoreB = 0;
    redrawScores();
}

const int margin = 25;
const int size = 50;
const int resetButtonW = 40;

void redrawScores()
{
    char buff[40];
    sprintf(buff, "%d - %d", ScoreA, ScoreB);

    int backOffset = (ScoreA > 9) * 20;

    ttgo->tft->fillRect(0, h / 2 - 30, w, h / 2 + 30, 0x0000 /*   0,   0,   0 */);
    drawText(String(buff), w / 2 - 50 - backOffset, h / 2 - 23, 2, 3, 0xFFFF /* 255, 255, 255 */);
}

void TeamScoresTick()
{

    tBox APlus = {margin, margin, margin + size, margin + size};
    tBox AMinus = {margin, h - margin - size, margin + size, h - margin};
    tBox BPlus = {w - margin - size, margin, w - margin, margin + size};
    tBox BMinus = {w - margin - size, h - margin - size, w - margin, h - margin};

    if (!drawn)
    {
        redrawScores();

        createButton(
            APlus, onUp, [](int x, int y)
            {
                ScoreA++;
                redrawScores();
            },
            GREEN_ALLOW, "+", 0x0000 /*   0,   0,   0 */);
        createButton(
            AMinus, onUp, [](int x, int y)
            {
                ScoreA--;
                ScoreA = max(0, ScoreA);
                redrawScores();
            },
            RED_CANCEL, "-", 0xFFFF /* 255, 255, 255 */);
        createButton(
            BPlus, onUp, [](int x, int y)
            {
                ScoreB++;
                redrawScores();
            },
            GREEN_ALLOW, "+", 0x0000 /*   0,   0,   0 */);
        createButton(
            BMinus, onUp, [](int x, int y)
            {
                ScoreB--;
                ScoreB = max(0, ScoreB);
                redrawScores();
            },
            RED_CANCEL, "-", 0xFFFF /* 255, 255, 255 */);
        tBox resetBox = {w / 2 - resetButtonW, margin, w / 2 + resetButtonW, margin + size};

        createButton(
            resetBox, onUp, [](int x, int y)
            {
                reset();
            },
            RED_CANCEL, "Reset", 0xFFFF /* 255, 255, 255 */);

    }
}
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appTimer.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appTimer.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appTimer.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appWatch.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appWatch.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appWatch.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\buttonSistem.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\buttonSistem.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\buttonSistem.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\logger.ino"
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\ttgoGlovalDeclarations.ino"
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\utils.ino"

# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\basicWatch.ino"
# 2 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\basicWatch.ino" 2
# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\basicWatch.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\basicWatch.ino" 2

// apps

# 8 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\basicWatch.ino" 2
# 9 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\basicWatch.ino" 2
# 10 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\basicWatch.ino" 2

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

  Serial.printf("Setup done!! \n");
}

int startClickX = -1;
int startClickY = -1;

int lastTouchX = -1;
int lastTouchY = -1;

int selected = -1;

bool fingerDown = false;

void onfingerDown(int x, int y)
{
  //Serial.printf("finger down x: %d y:%d \n", x, y);
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

  //Serial.printf("finger up x: %d y:%d \n", x, y);
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
      timerTick(UsableTime);
      break;
    }
    case teamScores:
    {
      break;
    }
    case controlPannel:
    {
      controlPannelTick();
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
      watchTick(year, month, day, hour, minute, seconds, UsableTime);
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
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\appControlPannel.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\appControlPannel.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\appControlPannel.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\appTimer.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\appTimer.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\appTimer.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\appWatch.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\appWatch.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\buttonSistem.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\buttonSistem.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\ttgoGlovalDeclarations.ino"
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\basicWatch\\utils.ino"

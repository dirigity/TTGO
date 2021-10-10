#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"

// apps

#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appWatch.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appTimer.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appControlPannel.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appLauncher.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\logger.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appCalculator.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appBaseConversion.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appCalendario.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appWifiPannel.ino"

void setup()
{

  Serial.begin(115200);

  ttgo = TTGOClass::getWatch();
  ttgo->begin();

  // if (permanent.lightSleepMode){
  //   int16_t kk;
  //   if(ttgo->getTouch(kk, kk)){
  //     permanent.lightSleepMode = false;
  //     turnOn();
  //   }else{
  //     softSleep();
  //   }
  // }

  //Serial.setTimeout(50);

  // Get Watch object and set up the display

  ttgo->motor_begin();
  // ttgo->lvgl_begin();
  ttgo->openBL();

  w = ttgo->tft->width();
  h = ttgo->tft->height();

  FULL_SCREEN_BOX = {0, 0, w, h};

  RED_CANCEL = createRGB(255, 70, 70);
  GREEN_ALLOW = createRGB(70, 255, 70);

  // turn on/off

  // pinMode(AXP202_INT, INPUT_PULLUP); // button
  // attachInterrupt(
  //     AXP202_INT, []
  //     { interrupt = button; },
  //     FALLING);

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
  //esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);

  //!Clear IRQ unprocessed  first

  //ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
  //ttgo->power->clearIRQ();

  interaction();
  planedButtonCoolDown = millis() + 5;
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
  int year, month, day, hour, minute, seconds;
  getTime(year, month, day, hour, minute, seconds);

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

      //Serial.println("loop1");
      //delay(100);

      createInterationArea(
          FULL_SCREEN_BOX, onUp, [](int x, int y)
          {
            if (app != launcher)
            {
              if (y - startClickY < -80)
              {
                goToLauncher();
                drawButtons();
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
      softSleep();
      break;

    case calculator:
      CalculatorTick();
      break;

    case timer:
      timerTick();
      break;

    case teamScores:
      TeamScoresTick();
      break;

    case controlPannel:
      controlPannelTick();
      break;

    case baseConversor:
      baseConversorTick();
      break;

    case morse:
      MorseTick();
      break;
    case calendar:
      CalendarioTick();
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
      watchTick(year, month, day, hour, minute, seconds);
      break;

    case wifiPannel:
      wifiPannelTick();
      break;
    }
  }
  else
  { // pantalla apagada

    //delay(100);

    //Serial.println("me despierto?");

    int16_t kk;
    bool touching = ttgo->getTouch(kk, kk);
    if (touching)
    {
      //Serial.println("nos despiertamos");

      turnOn();
    }
  }

  // carillón
  if (permanent.carillon)
  {
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

  firstLoop = false;
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

# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino"
# 2 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2

// apps
# 7 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 8 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 9 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 10 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 11 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 12 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 13 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 14 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 15 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 16 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2
# 17 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino" 2

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
    case joke:
      jokeTick();
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
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appBaseConversion.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appBaseConversion.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appBaseConversion.ino" 2
# 5 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appBaseConversion.ino" 2

# 7 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appBaseConversion.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino" 2
# 5 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalendario.ino"
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
        const int buffLenght = 150;
        char info[buffLenght];

        snprintf(info, buffLenght,
                 "year: %d  \n"
                 "month: %s %02d \n"
                 "day: %02d (%s)\n"
                 "time: %02d:%02d:%02d \n"
                 "power: %.2fV\n"
                 "batt: %02d%%\n"
                 "chargeing: %s\n",
                 year, monthsToWord[month], month, day, WeekDaysToWord[ttgo->rtc->getDayOfWeek(day, month, year)], hour, minute, seconds, ttgo->power->getBattVoltage() / 1000., getBatteryCorrectedPorcentage(), ttgo->power->isChargeing() ? "True" : "False");
        drawText(info, 0, 0, 2, 2, 0xFFFF /* 255, 255, 255 */);

        lastDrawnTime = seconds;
    }

    if (lastDrawnTime != seconds)
    {
        invalidate = true;
    }
}
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appJoke.ino"
//https://icanhazdadjoke.com/slack


# 5 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appJoke.ino" 2
# 6 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appJoke.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appLauncher.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appLauncher.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appLauncher.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appMorse.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appMorse.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appMorse.ino" 2
# 5 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appMorse.ino" 2
//#include <string.h>




char text[100] = "";

struct tMorseConversion
{
    const char c;
    const char *translation;
};

const int timeUd = 300;

const int short_ = 1;
const int long_ = 3;
const int silenceBetweenBits = 1;
const int silenceBetweenLetters = 3;
const int silenceBetweenWords = 7;
tMorseConversion MorseTranslation[36] = {
    {'a', ".-"},
    {'b', "-..."},
    {'c', "-.-."},
    {'d', "-.."},
    {'e', "."},
    {'f', ".._."},
    {'g', "--."},
    {'h', "...."},
    {'i', ".."},
    {'j', ".---"},
    {'k', "-.-"},
    {'l', ".-.."},
    {'m', "--"},
    {'n', "-."},
    {'o', "---"},
    {'p', ".--."},
    {'q', "--.-"},
    {'r', ".-."},
    {'s', "..."},
    {'t', "-"},
    {'u', "..-"},
    {'v', "...-"},
    {'w', ".--"},
    {'x', "-..-"},
    {'y', "-.--"},
    {'z', "--.."},
    {'0', "-----"},
    {'1', ".----"},
    {'2', "..---"},
    {'3', "...--"},
    {'4', "....-"},
    {'5', "....."},
    {'6', "-...."},
    {'7', "--..."},
    {'8', "---.."},
    {'9', "----."},

};

void write(char c, char *str, int loops)
{
    if (loops == 0)
    {
        return;
    }

    int strLen = strlen(str);

    char *start = strLen + str;

    for (int i = 0; i < loops; i++)
    {
        start[i] = c;
    }
    start[loops] = '\0';
    //Serial.printf("morse construction: \"%s\", current char %c %d times\n", str, c, loops);
}

void MorsePlay(const char *text)
{
    ttgo->bl->adjust(255);
    if (*text == '\0')
    {
        return;
    }

    bool abort = false;
    bool end = false;

    char translation[200] = " ";

    //Serial.printf("translating: %s", text);

    while (*text != '\0')
    {
        if (*text == ' ' || *text == '_')
        {
            // Serial.println("espacio entre palabras");
            write(' ', translation, silenceBetweenWords - silenceBetweenLetters + silenceBetweenBits);
        }
        else
        {
            //Serial.printf("buscando letra %c \n", *text);

            for (int i = 0; i < 36; i++)
            {
                if (toLowerChar(*text) == MorseTranslation[i].c)
                {
                    //Serial.printf("encontrada letra %c con codigo %s \n", MorseTranslation[i].c, MorseTranslation[1].translation);

                    const char *seq = MorseTranslation[i].translation;

                    while (*seq != '\0')
                    {
                        if (*seq == '.')
                        {
                            //Serial.println("short");

                            write('-', translation, short_);
                        }
                        else
                        {
                            //Serial.println("largo");

                            write('-', translation, long_);
                        }

                        //Serial.println("espacio entre letras");
                        write(' ', translation, silenceBetweenBits);

                        seq++;
                    }
                    i = 100; // break;
                }
            }
        }
        //Serial.println("fin de letra");

        write(' ', translation, silenceBetweenLetters - silenceBetweenBits);

        text++;
    }
    Serial.println("fin de traducion");

    int startTime = millis();

    Serial.println(translation);

    while (!abort)
    {

        int i = (millis() - startTime) / timeUd;
        bool currentVal = translation[i] == '-';

        ttgo->bl->adjust(currentVal ? 255 : 0);
        ttgo->tft->fillScreen(currentVal ? 0xFFFF /* 255, 255, 255 */ : 0x0000 /*   0,   0,   0 */);

        short int x, y;
        abort = ttgo->getTouch(x, y) || translation[i] == '\0';
    }

    ttgo->bl->adjust(permanent.brightness);
    invalidate = true;
}

bool blink = false;
bool deco = false;

const int MaxDecodeOutSize = 15;
char DecoDisplay[MaxDecodeOutSize] = "";
char DecoInput[MaxDecodeOutSize] = "";

void MorseTick()
{
    if (!drawn)
    {
        if (deco)
        {
            const tGrid layout = createGrid(FULL_SCREEN_BOX, 10, 10, 3, 4);
            const tBox DisplayBox = boxMerge(Cell(layout, 0, 0), Cell(layout, 1, 0));
            const tBox EraseDisplayBox = Cell(layout, 2, 0);
            const tBox InputBox = boxMerge(Cell(layout, 0, 1), Cell(layout, 1, 1));
            const tBox BackSpaceInputBox = Cell(layout, 2, 1);
            const tBox LongBox = Cell(layout, 0, 2);
            const tBox ShortBox = Cell(layout, 1, 2);
            const tBox SpaceBox = Cell(layout, 0, 3);
            const tBox DoneBox = boxMerge(Cell(layout, 1, 3), Cell(layout, 2, 3));

            const tBox Validate = Cell(layout, 2, 2);

            createButton(
                DisplayBox, onUp, [](int x, int y) {},
                0xFFFF /* 255, 255, 255 */, DecoDisplay, 0x0000 /*   0,   0,   0 */);
            createButton(
                InputBox, onUp, [](int x, int y) {},
                0xFFFF /* 255, 255, 255 */, DecoInput, 0x0000 /*   0,   0,   0 */);

            createButton(
                EraseDisplayBox, onUp, [](int x, int y)
                {
                    *DecoDisplay = '\0';
                    drawn = false;
                },
                RED_CANCEL, "CE", 0x0000 /*   0,   0,   0 */);
            createButton(
                BackSpaceInputBox, onUp, [](int x, int y)
                {
                    DecoInput[max(0, int(strlen(DecoInput) - 1))] = '\0';
                    drawn = false;
                },
                RED_CANCEL, "Back", 0x0000 /*   0,   0,   0 */);
            createButton(
                LongBox, onUp, [](int x, int y)
                {
                    int len = strlen(DecoInput);
                    if (len < MaxDecodeOutSize)
                    {
                        DecoInput[len] = '-';
                        DecoInput[len + 1] = '\0';
                        drawn = false;
                    }
                },
                createRGB(40, 40, 240), "L", 0x0000 /*   0,   0,   0 */);
            createButton(
                ShortBox, onUp, [](int x, int y)
                {
                    int len = strlen(DecoInput);
                    if (len < MaxDecodeOutSize)
                    {
                        DecoInput[len] = '.';
                        DecoInput[len + 1] = '\0';
                        drawn = false;
                    }
                },
                createRGB(40, 40, 240), "S", 0x0000 /*   0,   0,   0 */);
            createButton(
                SpaceBox, onUp, [](int x, int y)
                {
                    int len = strlen(DecoDisplay);
                    if (len < MaxDecodeOutSize)
                    {
                        DecoDisplay[len] = ' ';
                        DecoDisplay[len + 1] = '\0';
                        drawn = false;
                    }
                },
                createRGB(40, 40, 240), "_", 0x0000 /*   0,   0,   0 */);
            createButton(
                DoneBox, onUp, [](int x, int y)
                {
                    drawn = false;
                    blink = false;
                    deco = false;
                },
                createRGB(40, 40, 240), "Done", 0x0000 /*   0,   0,   0 */);

            createButton(
                Validate, onUp, [](int x, int y)
                {
                    for (int i = 0; i < 36; i++)
                    {
                        //Serial.printf("comparing %s and %s \n", MorseTranslation[i].translation, DecoInput);
                        if (0 == strcmp(MorseTranslation[i].translation, DecoInput))
                        {
                            //Serial.println("They are equal");
                            int len = strlen(DecoDisplay);
                            if (len < MaxDecodeOutSize)
                            {
                                DecoDisplay[len] = MorseTranslation[i].c;
                                DecoDisplay[len + 1] = '\0';
                                drawn = false;
                            }
                            i = 100;
                        }
                        //else
                        //Serial.println("They are different");
                    }
                    *DecoInput = '\0';
                },
                0x07E0 /*   0, 255,   0 */, "read", 0x0000 /*   0,   0,   0 */);
        }
        else if (blink)
        {
            createInterationArea(
                FULL_SCREEN_BOX, onUp, [](int x, int y)
                {
                    blink = false;
                    invalidate = true;
                    ttgo->bl->adjust(permanent.brightness);
                });
        }
        else
        {
            ttgo->tft->fillScreen(0);

            const tGrid layout = createGrid(FULL_SCREEN_BOX, 10, 10, 2, 4);
            const tBox TextInputBox = boxMerge(Cell(layout, 1, 0), Cell(layout, 0, 0));
            const tBox SOSBox = Cell(layout, 0, 2);
            const tBox PlayBox = Cell(layout, 1, 2);
            const tBox DecoBox = Cell(layout, 0, 3);
            const tBox BlinkBox = Cell(layout, 1, 3);
            const tBox BackSpaceBox = Cell(layout, 0, 1);
            const tBox CEBox = Cell(layout, 1, 1);

            createButton(
                CEBox, onUp, [](int x, int y)
                {
                    text[0] = '\0';
                    invalidate = true;
                },
                RED_CANCEL, "CE", 0x0000 /*   0,   0,   0 */);

            createButton(
                BackSpaceBox, onUp, [](int x, int y)
                {
                    text[strlen(text) - 1] = '\0';
                    invalidate = true;
                },
                0xFDA0 /* 255, 180,   0 */, "back", 0x0000 /*   0,   0,   0 */);

            createButton(
                TextInputBox, onUp, [](int x, int y)
                {
                    OCRstart(text, 99);
                    drawn = false;
                },
                0xFFFF /* 255, 255, 255 */, text, 0x0000 /*   0,   0,   0 */);
            createButton(
                SOSBox, onUp, [](int x, int y)
                { MorsePlay("SOS SOS SOS SOS"); },
                createRGB(40, 40, 240), "SOS", 0x0000 /*   0,   0,   0 */);
            createButton(
                PlayBox, onUp, [](int x, int y)
                { MorsePlay(text); },
                createRGB(40, 40, 240), "Play", 0x0000 /*   0,   0,   0 */);
            createButton(
                BlinkBox, onUp, [](int x, int y)
                {
                    blink = true;
                    drawn = false;
                },
                createRGB(40, 40, 240), "blink", 0x0000 /*   0,   0,   0 */);
            createButton(
                DecoBox, onUp, [](int x, int y)
                {
                    deco = true;
                    drawn = false;
                },
                createRGB(40, 40, 240), "Decode", 0x0000 /*   0,   0,   0 */);
        }
    }

    if (blink)
    {
        const int period = 500; // in ms
        ttgo->bl->adjust(millis() % period > period / 2 ? 0 : 255);
        ttgo->tft->fillScreen(millis() % period > period / 2 ? 0x0000 /*   0,   0,   0 */ : 0xFFFF /* 255, 255, 255 */);
    }
}
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
    drawText(buff, w / 2 - 50 - backOffset, h / 2 - 23, 2, 3, 0xFFFF /* 255, 255, 255 */);
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
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appWifiPannel.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appWifiPannel.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appWifiPannel.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\buttonSistem.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\buttonSistem.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\buttonSistem.ino" 2
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\logger.ino"
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\ttgoGlovalDeclarations.ino"
# 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\utils.ino"

# 3 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\utils.ino" 2
# 4 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\utils.ino" 2

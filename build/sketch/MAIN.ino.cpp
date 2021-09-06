#include <Arduino.h>
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\MAIN.ino"
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
      Serial.printf("quzas click \n");

      if (planedButtonCoolDown < UsableTime)
      {
        Serial.printf("click  \n");

        interaction();
        ToggleOnOff();
      }

      break;
    }

    ttgo->power->clearIRQ();
    interrupt = none;
  }
  if (ttgo->bl->isOn())
  {
    // touch manager
    ManageTouch();
    //if(firstLoop)Serial.println("firstLoop at 216 of MAIN");

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
  //if(firstLoop) Serial.println("End of first loop");
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

#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appBaseConversion.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"

#include <stdlib.h> /* strtol */

#ifndef appBaseConversion
#define appBaseConversion

char Dec[20] = "Dec";
char Hex[20] = "Hex";
char Bin[20] = "Bin";
char *removed0 = Bin;
char Oct[20] = "Oct";

int value = 0;

void refresh(int base)
{
    char buff[20] = "";

    OCRstart(buff, 99);
    value = strtol(buff, nullptr, base);

    sprintf(Dec, "%d", value);
    sprintf(Hex, "0x%X", value);
    sprintf(Oct, "0%o", value);

    Bin[0] = '\0';

    for (int z = (32768 / 8); z > 0; z >>= 1)
    {
        strcat(Bin, ((value & z) == z) ? "1" : "0");
    }

    removed0 = Bin;
    while (*removed0 == '0')
    {
        removed0++;
    }
    if(*removed0 == '\0'){
        removed0--;
    }
    invalidate = true;
}

void baseConversorTick()
{

    if (!drawn)
    {

        tGrid layout = createGrid(FULL_SCREEN_BOX, 10, 10, 1, 4);
        tBox DecBox = Cell(layout, 0, 0);
        tBox HexBox = Cell(layout, 0, 1);
        tBox BinBox = Cell(layout, 0, 3);
        tBox OctBox = Cell(layout, 0, 2);

        createButton(
            DecBox, onUp, [](int x, int y)
            { refresh(10); },
            TFT_WHITE, Dec, TFT_BLACK);
        createButton(
            HexBox, onUp, [](int x, int y)
            { refresh(16); },
            TFT_WHITE, Hex, TFT_BLACK);
        createButton(
            BinBox, onUp, [](int x, int y)
            { refresh(2); },
            TFT_WHITE, removed0, TFT_BLACK);
        createButton(
            OctBox, onUp, [](int x, int y)
            { refresh(8); },
            TFT_WHITE, Oct, TFT_BLACK);
    }
}

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalculator.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"

#ifndef appCalculator
#define appCalculator

static const char *errors[] = {
    "no error",
    "parentheses don't match",
    "invalid character",
    "division by zero"};

// (c) Peter Kankowski, 2007. http://smallcode.weblogs.us mailto:kankowski@narod.ru
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ================================
//   Simple expression evaluator
// ================================

double Gloval_X_Value = 0;

// Error codes
enum EXPR_EVAL_ERR
{
    EEE_NO_ERROR = 0,
    EEE_PARENTHESIS = 1,
    EEE_WRONG_CHAR = 2,
    EEE_DIVIDE_BY_ZERO = 3
};

typedef char EVAL_CHAR;

class ExprEval
{
private:
    EXPR_EVAL_ERR _err;
    EVAL_CHAR *_err_pos;
    int _paren_count;

    // Parse a number or an expression in parenthesis
    double ParseAtom(EVAL_CHAR *&expr)
    {
        // Skip spaces
        while (*expr == ' ')
            expr++;

        // Handle the sign before parenthesis (or before number)
        bool negative = false;
        if (*expr == '-')
        {
            negative = true;
            expr++;
        }
        if (*expr == '+')
        {
            expr++;
        }

        // Check if there is parenthesis
        if (*expr == '(')
        {
            expr++;
            _paren_count++;
            double res = ParseSummands(expr);
            if (*expr != ')')
            {
                // Unmatched opening parenthesis
                _err = EEE_PARENTHESIS;
                _err_pos = expr;
                return 0;
            }
            expr++;
            _paren_count--;
            return negative ? -res : res;
        }

        // It should be a number or an x; convert it to double

        if (*expr == 'x')
        {
            expr++;
            return negative ? -Gloval_X_Value : Gloval_X_Value;
        }

        char *end_ptr;
        double res = strtod(expr, &end_ptr);
        if (end_ptr == expr)
        {
            // Report error
            _err = EEE_WRONG_CHAR;
            _err_pos = expr;
            return 0;
        }
        // Advance the pointer and return the result
        expr = end_ptr;
        return negative ? -res : res;
    }

    // Parse multiplication and division
    double ParseFactors(EVAL_CHAR *&expr)
    {
        double num1 = ParseAtom(expr);
        for (;;)
        {
            // Skip spaces
            while (*expr == ' ')
                expr++;
            // Save the operation and position
            EVAL_CHAR op = *expr;
            EVAL_CHAR *pos = expr;
            if (op != '/' && op != '*')
                return num1;
            expr++;
            double num2 = ParseAtom(expr);
            // Perform the saved operation
            if (op == '/')
            {
                // Handle division by zero
                if (num2 == 0)
                {
                    _err = EEE_DIVIDE_BY_ZERO;
                    _err_pos = pos;
                    return 0;
                }
                num1 /= num2;
            }
            else
                num1 *= num2;
        }
    }

    // Parse addition and subtraction
    double ParseSummands(EVAL_CHAR *&expr)
    {
        double num1 = ParseFactors(expr);
        for (;;)
        {
            // Skip spaces
            while (*expr == ' ')
                expr++;
            EVAL_CHAR op = *expr;
            if (op != '-' && op != '+')
                return num1;
            expr++;
            double num2 = ParseFactors(expr);
            if (op == '-')
                num1 -= num2;
            else
                num1 += num2;
        }
    }

public:
    double Eval(EVAL_CHAR *expr)
    {
        _paren_count = 0;
        _err = EEE_NO_ERROR;
        double res = ParseSummands(expr);
        // Now, expr should point to '\0', and _paren_count should be zero
        if (_paren_count != 0 || *expr == ')')
        {
            _err = EEE_PARENTHESIS;
            _err_pos = expr;
            return 0;
        }
        if (*expr != '\0')
        {
            _err = EEE_WRONG_CHAR;
            _err_pos = expr;
            return 0;
        }
        return res;
    };
    EXPR_EVAL_ERR GetErr()
    {
        return _err;
    }
    EVAL_CHAR *GetErrPos()
    {
        return _err_pos;
    }
};

// expresion eval end

const int MaxInputStringSize = 300;

char buff[MaxInputStringSize] = "";

const int plotXres = 100;

double map(double StartRangeSrc, double EndRangeSrc, double StartRangeDst, double EndRangeDst, double val)
{
    return StartRangeDst + ((EndRangeDst - StartRangeDst) / (EndRangeSrc - StartRangeSrc)) * (val - StartRangeSrc);
}

double MinX, MinY, MaxX, MaxY;

void plot(double MinX_, double MinY_, double MaxX_, double MaxY_)
{

    MinX = MinX_;
    MinY = MinY_;
    MaxX = MaxX_;
    MaxY = MaxY_;
    ttgo->tft->fillScreen(TFT_WHITE);
    int zoomX = MaxX - MinX;
    int zoomY = MaxY - MinY;
    clearButtons();

    createInterationArea(
        FULL_SCREEN_BOX, onUp, [](int x, int y)
        {
            const int displacement = 1;

            bool exit = true;

            if (x > w * 2 / 3) // derecha
            {
                plot(MinX + displacement, MinY, MaxX + displacement, MaxY);
                exit = false;
            }
            if (x < w / 3) // izq
            {
                plot(MinX - displacement, MinY, MaxX - displacement, MaxY);
                exit = false;
            }
            if (y > h * 2 / 3) // abajo
            {
                plot(MinX, MinY + displacement, MaxX, MaxY + displacement);
                exit = false;
            }
            if (y < h / 3) // arriba
            {
                plot(MinX, MinY - displacement, MaxX, MaxY - displacement);
                exit = false;
            }

            if (exit)
            {
                invalidate = true;
            }
        });

    // draw lines
    // vertical

    const int firstLineX = floor(MinX);
    const int lastLineX = ceil(MaxX);
    for (int i = firstLineX; i < lastLineX; i++)
    {
        int lineX = map(MinX, MaxX, 0, w, i);
        int col = i == 0 ? 0 : createRGB(220, 220, 220);
        ttgo->tft->drawFastVLine(lineX, 0, h, col);
    }

    // horizontal

    const int firstLineY = floor(MinY);
    const int lastLineY = ceil(MaxY);
    for (int i = firstLineY; i < lastLineY; i++)
    {
        int lineY = map(MinY, MaxY, 0, w, i);
        int col = i == 0 ? 0 : createRGB(220, 220, 220);
        ttgo->tft->drawFastHLine(0, lineY, w, col);
    }

    // numba

    // draw curve

    int lastDrawnX = -100;
    int lastDrawnY = -100;

    for (int i = 0; i < plotXres + 1; i++)
    {
        double currentX = double(i) / plotXres * zoomX + MinX;
        Gloval_X_Value = currentX;
        //char substitution[MaxInputStringSize];
        //const int NumberLenght = 20;
        //char Xvalue[NumberLenght];
        //snprintf(Xvalue, NumberLenght - 1, "(%.4f)", currentX);
        //int valueSize = strlen(Xvalue);
        //strcpy(substitution, buff);
        // substitute 'x' by valueSize
        //Serial.printf("the string of the number is:\"%s\" and has size %d \n", Xvalue, valueSize);
        // for (int i = 0; i < strlen(substitution); i++)
        // {
        //     if (substitution[i] == 'x')
        //     {
        //         Serial.printf("found an x at pos: %d \n", i);
        //         //shift right from the end of the array to the position of the 'x'
        //         for (int j = strlen(substitution) + 1; j > i; j--)
        //         {
        //             substitution[j + valueSize] = substitution[j];
        //         }
        //         Serial.printf("after shifting: \"%s\"\n", substitution);
        //         for (int j = 0; j < valueSize; j++)
        //         {
        //             substitution[j + i] = Xvalue[j];
        //         }
        //         Serial.printf("after substitution: \"%s\"\n", substitution);
        //     }
        // }

        ExprEval eval;
        double res = eval.Eval(buff);

        if (eval.GetErr() != EEE_NO_ERROR)
        {
            Serial.printf("  Error: %s at %s\n", errors[eval.GetErr()], eval.GetErrPos());
        }
        else
        {
            int currentDrawingX = map(MinX, MaxX, 0, w, currentX);
            int currentDrawingY = map(MinY, MaxY, 0, h, -res);

            ttgo->tft->drawLine(lastDrawnX, lastDrawnY, currentDrawingX, currentDrawingY, createRGB(40, 0, 0));
            ttgo->tft->drawLine(lastDrawnX, lastDrawnY + 1, currentDrawingX, currentDrawingY + 1, createRGB(40, 0, 0));
            ttgo->tft->drawLine(lastDrawnX, lastDrawnY - 1, currentDrawingX, currentDrawingY - 1, createRGB(40, 0, 0));
            lastDrawnX = currentDrawingX;
            lastDrawnY = currentDrawingY;
        }
    }
}

const int buttonMidleSeparation = 140;

void CalculatorTick()
{

    const tGrid MainGrid = createGrid(FULL_SCREEN_BOX, 10, 10, 2, 4);

    const tBox textDisplay = boxMerge(Cell(MainGrid, 0, 0), Cell(MainGrid, 1, 0));
    const tBox equalsBox = Cell(MainGrid, 0, 2);    //{10, buttonMidleSeparation + 10, w / 2 - 5, h - 10};
    const tBox CEBox = Cell(MainGrid, 1, 1);        //{w / 2 + 5, 70, w - 10, buttonMidleSeparation};
    const tBox BackSpaceBox = Cell(MainGrid, 0, 1); //{10, 70, w / 2 - 5, buttonMidleSeparation};
    const tBox graphingBox = Cell(MainGrid, 1, 2);  //{w / 2 + 5, buttonMidleSeparation + 10, w - 10, h - 10};
    const tBox exitBox = boxMerge(Cell(MainGrid, 0, 3), Cell(MainGrid, 1, 3));

    if (!drawn)
    {
        ttgo->tft->fillRect(10, 10, 40, 40, TFT_WHITE);
        createButton(
            exitBox, onUp, [](int x, int y)
            { goToLauncher(); },
            RED_CANCEL, "Exit", TFT_BLACK);
        createButton(
            textDisplay, onUp, [](int x, int y)
            {
                OCRstart(buff, MaxInputStringSize);
                drawn = false;
            },
            TFT_WHITE, buff, TFT_BLACK);

        createButton(
            equalsBox, onUp, [](int x, int y)
            {
                Serial.printf("evaluating");
                ExprEval eval;
                double res = eval.Eval(buff);
                if (eval.GetErr() != EEE_NO_ERROR)
                {
                    Serial.printf("  Error: %s at %s\n", errors[eval.GetErr()], eval.GetErrPos());
                }
                else
                {
                    sprintf(buff, "%g", res);
                }
                drawn = false;
            },
            TFT_CYAN, "=", TFT_BLACK);

        createButton(
            CEBox, onUp, [](int x, int y)
            {
                buff[0] = '\0';
                invalidate = true;
            },
            RED_CANCEL, "CE", TFT_BLACK);

        createButton(
            BackSpaceBox, onUp, [](int x, int y)
            {
                buff[strlen(buff) - 1] = '\0';
                invalidate = true;
            },
            TFT_ORANGE, "back", TFT_BLACK);

        createButton(
            graphingBox, onUp, [](int x, int y)
            { plot(-6, -6, 6, 6); },
            TFT_GREEN, "plot", TFT_BLACK);

        //Serial.println("created button maybe probably");
    }

    //Serial.printf("calculator tick %d \n", drawn);
}

// double ParseAtom(char *&expr)
// {
//     // Read the number from string
//     char *end_ptr;
//     double res = strtod(expr, &end_ptr);
//     // Advance the pointer and return the result
//     expr = end_ptr;
//     return res;
// }

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appCalendario.ino"

#ifndef appCalendario
#define appCalendario

#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"

int day;
int month;
int year;

char title[10];

void CalendarioTick()
{
    if (firstLoop)
    {
        Serial.println("Starting Calendario and syncing date");
        {
            int k, kk, kkk;
            getTime(year, month, day, k, kk, kkk);
        }
    }
    if (!drawn)
    {
        const tGrid mainLayout = createGrid(FULL_SCREEN_BOX, 10, 10, 1, 5);
        const tBox header = Cell(mainLayout, 0, 0);
        const tBox body = boxMerge(Cell(mainLayout, 0, 1), Cell(mainLayout, 0, 4));
        const tGrid bodyLayout = createGrid(body, 0, 0, 7, 7);

        const tGrid headerLayout = createGrid(header, 0, 2, 5, 1);
        const tBox LastMonthBox = Cell(headerLayout, 0, 0);
        const tBox NextMonthBox = Cell(headerLayout, 4, 0);
        const tBox DateDisplayBox = boxMerge(Cell(headerLayout, 1, 0), Cell(headerLayout, 3, 0));

        createButton(
            LastMonthBox, onUp, [](int x, int y)
            {
                month--;
                if (month == 0)
                {
                    month = 12;
                    year--;
                }
                drawn = false;
            },
            TFT_WHITE, "<", TFT_BLACK);

        createButton(
            NextMonthBox, onUp, [](int x, int y)
            {
                month++;
                if (month == 13)
                {
                    month = 1;
                    year++;
                }
                drawn = false;
            },
            TFT_WHITE, ">", TFT_BLACK);

        sprintf(title, "%s %d", monthsToWord[month], year);
        createButton(
            DateDisplayBox, onUp, [](int x, int y)
            {
                int k;
                // open the year selection screen
            },
            TFT_WHITE, title, TFT_BLACK);

        for (int i = 0; i < 7; i++)
        {
            tBox box = Cell(bodyLayout, i, 0);
            createTextBox(box, rgb(20, 20, 20), DayToLable[i], TFT_BLACK, 2, 1); // top lables
        }

        int lastDayFromLastMonth = DaysInMonth[month - 2];
        int weekDay = ttgo->rtc->getDayOfWeek(lastDayFromLastMonth, month - 1, year);

        while (weekDay != 6)
        {
            createTextBox(Cell(bodyLayout, weekDay, 1), rgb(15, 15, 15), DayToText[lastDayFromLastMonth], TFT_BLACK, 2, 1);
            lastDayFromLastMonth--;
            weekDay = ttgo->rtc->getDayOfWeek(lastDayFromLastMonth, month - 1, year);
        }

        int week = 1;
        for (int i = 1; i < DaysInMonth[month - 1] + 1; i++)
        {
            int weekDay = ttgo->rtc->getDayOfWeek(i, month, year);

            createTextBox(Cell(bodyLayout, weekDay, week), weekDay > 4 ? rgb(25, 25, 25) : TFT_WHITE, DayToText[i], TFT_BLACK, 2, 1);
            if (weekDay == 6)
                week++;
        }
        // int i = 1;
        // while (week < 7)
        // {
        //     int weekDay = ttgo->rtc->getDayOfWeek(i, month+1, year);

        //     createTextBox(Cell(bodyLayout, weekDay, week), rgb(15, 15, 15), DayToText[i], TFT_BLACK, 2, 1);
        //     if (weekDay == 6)
        //         week++;
        //     i++;
        // }
    }
}

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appControlPannel.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appControlPannel
#define appControlPannel

void controlPannelTick()
{
    if (!drawn)
    {
        // carrillon
        tBox box = {10, 10, w - 10, 45};
        if (permanent.carillon)
            createButton(
                box, onUp, [](int x, int y)
                {
                    permanent.carillon = false;
                    drawn = false;
                },
                GREEN_ALLOW, "Apagar Carillon", createRGB(0, 0, 0));
        else
            createButton(
                box, onUp, [](int x, int y)
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
            boxExit, onUp, [](int x, int y)
            {
                app = launcher;
                drawn = false;
            },
            TFT_WHITE, "Back", TFT_BLACK);
        // scroll

        createInterationArea(
            {10, 50, w - 10, 95}, onDrag, [](int x, int y)
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
            });
    }
}
#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appDataMonitor.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appDataMonitor
#define appDataMonitor

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
                 "day: %02d (%s)\n"
                 "time: %02d:%02d:%02d \n"
                 "power: %.2fV\n"
                 "batt: %02d%%\n"
                 "chargeing: %s\n",
                 year, monthsToWord[month], month, day, WeekDaysToWord[ttgo->rtc->getDayOfWeek(day, month, year)], hour, minute, seconds, ttgo->power->getBattVoltage() / 1000., getBatteryCorrectedPorcentage(), ttgo->power->isChargeing() ? "True" : "False");
        drawText(info, 0, 0, 2, 2, TFT_WHITE);

        lastDrawnTime = seconds;
    }

    if (lastDrawnTime != seconds)
    {
        invalidate = true;
    }
}

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appLauncher.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appLauncher
#define appLauncher

// launcher app
int selected = -1;

const int appCount = 14;
const int skipedApps = 2;

const char *AppToString[appCount] = {
    "launcher",
    "OCR",

    "dataMonitor",
    "watch",
    "flashLight",
    "calculator", // demos and scientific
    "countdown",
    "timer",
    "teamScores",
    "controlPannel", // brigtness(rtc_mem), carillon(rtc_mem), battery stats
    "baseConversor",
    "morse",
    "calendar",
    "turnOff"};

void goToLauncher()
{
    app = launcher;
    drawn = false;
    selected = -1;
}

void launcherTick()
{
    if (!drawn)
    {
        createInterationArea(
            FULL_SCREEN_BOX, onDrag, [](int x, int y)
            {
                int borderTouchMargin = 20;
                double touchY = double(y) * double(h + borderTouchMargin * 2) / h - borderTouchMargin;

                touchY = minimum(maximum(0., touchY), double(h - 1));
                int newSelected = int(touchY / double(h) * (appCount - skipedApps));
                if (selected != newSelected)
                {
                    selected = newSelected;
                    drawn = false;
                }
            });

        createInterationArea(
            FULL_SCREEN_BOX, onUp, [](int x, int y)
            {
                app = tApp(selected + skipedApps);
                selected = -1;
                drawn = false;
                firstLoop = true;
                return;
            });

        int separation = 40;
        int totalSize = (appCount - skipedApps + 1) * separation;
        int overflow = maximum(totalSize - h, 0);
        int offset = maximum(0, overflow / (appCount - skipedApps) * selected - (skipedApps - 1));

        for (int i = skipedApps; i < appCount; i++)
        {
            drawText(AppToString[i], 0, (i - skipedApps) * separation - offset, 2, 2, selected + skipedApps == i ? createRGB(255, 255, 255) : createRGB(100, 100, 100));
        }
    }
}

#endif

#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appMorse.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"
//#include <string.h>

#ifndef appMorse
#define appMorse

char text[100] = "";

struct tMorseConversion
{
    char c;
    char *translation;
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

void MorsePlay(char *text)
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

                    char *seq = MorseTranslation[i].translation;

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
        ttgo->tft->fillScreen(currentVal ? TFT_WHITE : TFT_BLACK);

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
                TFT_WHITE, DecoDisplay, TFT_BLACK);
            createButton(
                InputBox, onUp, [](int x, int y) {},
                TFT_WHITE, DecoInput, TFT_BLACK);

            createButton(
                EraseDisplayBox, onUp, [](int x, int y)
                {
                    *DecoDisplay = '\0';
                    drawn = false;
                },
                RED_CANCEL, "CE", TFT_BLACK);
            createButton(
                BackSpaceInputBox, onUp, [](int x, int y)
                {
                    DecoInput[max(0, int(strlen(DecoInput) - 1))] = '\0';
                    drawn = false;
                },
                RED_CANCEL, "Back", TFT_BLACK);
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
                createRGB(40, 40, 240), "L", TFT_BLACK);
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
                createRGB(40, 40, 240), "S", TFT_BLACK);
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
                createRGB(40, 40, 240), "_", TFT_BLACK);
            createButton(
                DoneBox, onUp, [](int x, int y)
                {
                    drawn = false;
                    blink = false;
                    deco = false;
                },
                createRGB(40, 40, 240), "Done", TFT_BLACK);

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
                TFT_GREEN, "read", TFT_BLACK);
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
                RED_CANCEL, "CE", TFT_BLACK);

            createButton(
                BackSpaceBox, onUp, [](int x, int y)
                {
                    text[strlen(text) - 1] = '\0';
                    invalidate = true;
                },
                TFT_ORANGE, "back", TFT_BLACK);

            createButton(
                TextInputBox, onUp, [](int x, int y)
                {
                    OCRstart(text, 99);
                    drawn = false;
                },
                TFT_WHITE, text, TFT_BLACK);
            createButton(
                SOSBox, onUp, [](int x, int y)
                { MorsePlay("SOS SOS SOS SOS"); },
                createRGB(40, 40, 240), "SOS", TFT_BLACK);
            createButton(
                PlayBox, onUp, [](int x, int y)
                { MorsePlay(text); },
                createRGB(40, 40, 240), "Play", TFT_BLACK);
            createButton(
                BlinkBox, onUp, [](int x, int y)
                {
                    blink = true;
                    drawn = false;
                },
                createRGB(40, 40, 240), "blink", TFT_BLACK);
            createButton(
                DecoBox, onUp, [](int x, int y)
                {
                    deco = true;
                    drawn = false;
                },
                createRGB(40, 40, 240), "Decode", TFT_BLACK);
        }
    }

    if (blink)
    {
        const int period = 500; // in ms
        ttgo->bl->adjust(millis() % period > period / 2 ? 0 : 255);
        ttgo->tft->fillScreen(millis() % period > period / 2 ? TFT_BLACK : TFT_WHITE);
    }
}

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appOcr.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appOCR
#define appOCR

tApp returnApp;

struct tuple
{
    char equivalence;
    long long int seed;
};

struct tPos
{
    int x;
    int y;
};

tPos nodes[8] = {
    {0, 0},
    {2, 0},
    {1, 1},
    {0, 2},
    {2, 2},
    {1, 3},
    {0, 4},
    {2, 4},
};

const int MaxLinks = 15;
struct tLinkList
{
    int list[MaxLinks];
    int counter = 0;
};

tLinkList linkList;

int indexCombinations[28][2] = {{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {4, 5}, {4, 6}, {4, 7}, {5, 6}, {5, 7}, {6, 7}};

const int dictLen = 74;

tuple dict[dictLen] = {
    {' ', 134217728},
    {'1', 16777728},
    {'2', 135528961},
    {'3', 151257601},
    {'4', 17039876},
    {'4', 17047680},
    {'4', 17048192},
    {'5', 151257093},
    {'6', 152305669},
    {'7', 16777729},
    {'8', 152306181},
    {'9', 17039877},
    {'9', 151257605},
    {'0', 152044037},
    {'+', 294912},
    {'-', 262144},
    {'(', 67141760},
    {')', 33587202},
    {'/', 2048},
    {'/', 33587328},
    {'/', 65664},
    {'/', 33555456},
    {'^', 24576},
    {'*', 4743168},
    {'=', 134479872},
    {'.', 33554432},
    {'a', 18088453},
    {'b', 202907781},
    {'b', 152305668},
    {'c', 135528448},
    {'c', 134217761},
    {'d', 152306176},
    {'e', 134480417},
    {'f', 1310725},
    {'g', 84148741},
    {'g', 155189281},
    {'g', 152043525},
    {'h', 18088452},
    {'h', 18087940},
    {'i', 16777216},
    {'i', 1048576},
    {'j', 152044032},
    {'j', 83886592},
    {'k', 68690052},
    {'l', 135266308},
    {'m', 17826438},
    {'m', 22544384},
    {'n', 84967942},
    {'n', 85458944},
    {'o', 152305664},
    {'p', 1311237},
    {'q', 84148741},
    {'r', 68944389},
    {'r', 1310720},
    {'s', 184811653},
    {'s', 201875456},
    {'t', 32769},
    {'t', 1048581},
    {'u', 152043520},
    {'u', 152044036},
    {'v', 4719108},
    {'v', 38797312},
    {'v', 38797828},
    {'v', 84410368},
    {'v', 84410884},
    {'w', 118489604},
    {'w', 118489088},
    {'x', 105381888},
    {'y', 16794114},
    {'y', 37765634},
    {'y', 32898},
    {'y', 1056900},
    {'y', 67641476},
    {'z', 172228608},
};

const int OCRsize = 50;
const int OCRLeftOfset = 40; // espacio pa poner bottones
const int OCRmargin = 10;
const int OCRbuttSize = 50;

char CallOCR()
{

    bool PrePressedDone = false;
    bool PrePressedCancel = false;
    const tBox DoneBox = {w - OCRmargin - OCRbuttSize, OCRmargin, w - OCRmargin, h / 2 - OCRmargin / 2};
    const tBox CancellBox = {w - OCRmargin - OCRbuttSize, h / 2 + OCRmargin / 2, w - OCRmargin, h - OCRmargin};

    drawn = false;
    linkList.counter = 0;
    clearButtons();

    bool captureFocus = true;
    int lastTouchedNode = -1;

    while (captureFocus)
    {
        if (!drawn)
        {
            ttgo->tft->fillScreen(0);

            for (int i = 0; i < 8; i++)
            {
                tPos node = nodes[i];
                int x = w / 2 - OCRsize + node.x * OCRsize - OCRLeftOfset;
                int y = h / 2 - OCRsize * 2 + node.y * OCRsize;
                if (i == lastTouchedNode)
                    ttgo->tft->fillCircle(x, y, 10, TFT_WHITE);
                else
                    ttgo->tft->fillCircle(x, y, 10, createRGB(200, 200, 200));
            }
            for (int i = 0; i < linkList.counter; i++)
            {
                int *nodeCombination = indexCombinations[linkList.list[i]];

                tPos node1 = nodes[nodeCombination[0]];
                int x1 = w / 2 - OCRsize + node1.x * OCRsize - OCRLeftOfset;
                int y1 = h / 2 - OCRsize * 2 + node1.y * OCRsize;

                tPos node2 = nodes[nodeCombination[1]];
                int x2 = w / 2 - OCRsize + node2.x * OCRsize - OCRLeftOfset;
                int y2 = h / 2 - OCRsize * 2 + node2.y * OCRsize;

                ttgo->tft->drawLine(x1, y1, x2, y2, TFT_WHITE);
            }

            drawBox(DoneBox, TFT_GREEN);
            drawBox(CancellBox, TFT_RED);

            drawn = true;
        }

        int16_t touchX, touchY;
        bool touching = ttgo->getTouch(touchX, touchY);
        if (touching)
        {
            int currentTouchingNode = -1;

            for (int i = 0; i < 8; i++)
            {
                tPos checkingNode = nodes[i];

                int x = w / 2 - OCRsize + checkingNode.x * OCRsize - OCRLeftOfset;
                int y = h / 2 - OCRsize * 2 + checkingNode.y * OCRsize;

                if (d(touchX, touchY, x, y) < 20)
                {
                    currentTouchingNode = i;
                    i = 100; // break
                }
            }

            //Serial.printf("currentTouchingNode: %d, lastTouchedNode: %d \n", currentTouchingNode, lastTouchedNode);

            if (currentTouchingNode != -1 && lastTouchedNode != -1 && currentTouchingNode != lastTouchedNode)
            {
                Serial.printf("creating link from %d to %d \n", lastTouchedNode, currentTouchingNode);
                int linkID = -1;

                for (int i = 0; i < 28; i++)
                {
                    int min_ = min(currentTouchingNode, lastTouchedNode);
                    int max_ = max(currentTouchingNode, lastTouchedNode);
                    if (min_ == indexCombinations[i][0] && max_ == indexCombinations[i][1])
                    {
                        linkID = i;
                        i = 100; // break
                    }
                }

                if (linkList.counter < MaxLinks)
                {
                    bool found = false;
                    for (int i = 0; i < linkList.counter; i++)
                    {
                        if (linkID == linkList.list[i])
                        {
                            found = true;
                        }
                    }
                    if (!found)
                    {
                        linkList.list[linkList.counter] = linkID;
                        linkList.counter++;
                        drawn = false;
                        ttgo->motor->onec(10);
                    }
                }
            }

            if (currentTouchingNode != -1)
                lastTouchedNode = currentTouchingNode;
        }
        else
        {
            lastTouchedNode = -1;
        }

        {

            if (insideBox(touchX, touchY, CancellBox))
            {
                PrePressedCancel = true;
                //Serial.println("clickDown");
            }
            else
            {
                //Serial.printf("PrePressedCancel: %d \n", PrePressedCancel);
                if (PrePressedCancel)
                {
                    Serial.println("pressed cancel");
                    if (linkList.counter == 0)
                    {
                        return '_';
                        Serial.println("cancel char input");
                    }
                    Serial.println("reset char input");
                    linkList.counter = 0;
                    drawn = false;
                }
                PrePressedCancel = false;
            }
            if (insideBox(touchX, touchY, DoneBox))
            {
                //Serial.println("clickDown");
                PrePressedDone = true;
            }
            else
            {
                //Serial.printf("PrePressedDone: %d \n", PrePressedDone);
                if (PrePressedDone)
                {
                    Serial.println("presed submit");
                    captureFocus = false;
                    int linkListCode = 0;
                    for (int i = 0; i < linkList.counter; i++)
                    {
                        linkListCode += pow(2, linkList.list[i]);
                    }
                    Serial.printf("linkListCode: %d \n", linkListCode);

                    for (int i = 0; i < dictLen; i++)
                    {
                        if (dict[i].seed == linkListCode)
                        {
                            Serial.printf("recognized: %c \n", dict[i].equivalence);
                            return dict[i].equivalence;
                        }
                    }
                    Serial.println("not recognized");
                }
                PrePressedDone = false;
            }
        }
    }

    invalidate = true;
    return '?';
}

void OCRstart(char *buff, int MaxInputStringSize)
{
    bool stop = false;
    while (!stop)
    {
        char next = CallOCR();
        if (next == '_')
        {
            stop = true;
        }
        else if (next == '?')
        {
            ttgo->motor->onec(10);
        }
        else
        {
            int CeroIdx = strlen(buff);
            if (CeroIdx != MaxInputStringSize - 1)
            {
                buff[CeroIdx] = next;
                buff[CeroIdx + 1] = '\0';
            }
            else
            {
                buff[MaxInputStringSize - 1] = '\0';
            }

            Serial.println(buff);
        }
    }
}
#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appTeamScores.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appTeamScores
#define appTeamScores

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

    ttgo->tft->fillRect(0, h / 2 - 30, w, h / 2 + 30, TFT_BLACK);
    drawText(buff, w / 2 - 50 - backOffset, h / 2 - 23, 2, 3, TFT_WHITE);
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
            GREEN_ALLOW, "+", TFT_BLACK);
        createButton(
            AMinus, onUp, [](int x, int y)
            {
                ScoreA--;
                ScoreA = max(0, ScoreA);
                redrawScores();
            },
            RED_CANCEL, "-", TFT_WHITE);
        createButton(
            BPlus, onUp, [](int x, int y)
            {
                ScoreB++;
                redrawScores();
            },
            GREEN_ALLOW, "+", TFT_BLACK);
        createButton(
            BMinus, onUp, [](int x, int y)
            {
                ScoreB--;
                ScoreB = max(0, ScoreB);
                redrawScores();
            },
            RED_CANCEL, "-", TFT_WHITE);
        tBox resetBox = {w / 2 - resetButtonW, margin, w / 2 + resetButtonW, margin + size};

        createButton(
            resetBox, onUp, [](int x, int y)
            { 
                reset(); 
            },
            RED_CANCEL, "Reset", TFT_WHITE);

    }
}

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appTimer.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appTimer
#define appTimer

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

void timerTick(int UsableTime)
{
    //Logger logger("timerTick");
    const int posX = 20;
    const int posY = 20;
    //logger("timer loop");
    if (!drawn)
    {
        ttgo->tft->fillRect(0, 0, w, h / 4, 0);
        for (int i = 0; i < lapList.counter; i++)
        {
            drawText(lapList.laps[i], 40, 75 + i * 20, 1, 2, 0xFFFF);
        }

        tBox boxStartAndStop = {20, 190, w / 2 - 20, 225};
        tBox boxLapAndReset = {w / 2 + 20, 190, w - 20, 225};
        if (!TimerRuning)
        {
            createButton(
                boxStartAndStop, onUp, [](int x, int y)
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
                boxLapAndReset, onUp, [](int x, int y)
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
                boxStartAndStop, onUp, [](int x, int y)
                {
                    stopedTimerTime = getUsableTime();
                    TimerRuning = false;
                    drawn = false;
                    currentlyDrawnSecondsSinceStart = -1;
                },
                RED_CANCEL, "Stop", 0x0000);
            createButton(
                boxLapAndReset, onUp, [](int x, int y)
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
                        drawText(lapList.laps[i], 40, 75 + i * 20, 1, 2, 0xFFFF);
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
        drawText(buff, posX, posY, 4, 2, 0xFFFF);
    }
}

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\appWatch.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appWatch
#define appWatch

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

void manageDisc(double clockAngle, double timeAngle, double midsM, double MThickness, int r, int g, int b)
{
    double Intensity = angle(clockAngle - timeAngle) / (2 * PI);
    //double Intensity_ = max(0., Intensity - 0.01);
    double CurrentCapRoundness = CapRoundness(Intensity, midsM, MThickness);
    double Mstart_ = midsM - MThickness / 2 * CurrentCapRoundness;
    double Mend_ = midsM + MThickness / 2 * CurrentCapRoundness;

    if (CurrentCapRoundness != 1)
    {
        drawPolarSegment(clockAngle, h / 2 * (midsM - MThickness / 2), h / 2 * Mstart_, TFT_BLACK);
        drawPolarSegment(clockAngle, h / 2 * Mend_, h / 2 * (midsM + MThickness / 2), TFT_BLACK);
    }

    drawPolarSegment(clockAngle, h / 2 * Mstart_, h / 2 * Mend_, createRGB(Intensity * r, Intensity * g, Intensity * b));
}

double secondDrawingAngle = 0;
double minuteDrawingAngle = 0;
double hourDrawingAngle = 0;
double battDrawingAngle = 0;

double battAngle = 0;

void watchTick(int year, int month, int day, int hour, int minute, int seconds, int UsableTime)
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
        createInterationArea(
            FULL_SCREEN_BOX, onUp, [](int x, int y)
            {
                if (y - startClickY > h/2)
                {
                    enterDeepSleep();
                }
                if( x - startClickX > w/2){
                    app = dataMonitor;
                    invalidate = true;
                }
            });

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
        battAngle = (getBatteryCorrectedPorcentage() / 100. * 2. * PI);
    }
    else
    {
        battAngle = battAngle * 0.95 + (getBatteryCorrectedPorcentage() / 100. * 2. * PI) * 0.05;
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
}

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\logger.ino"

#ifndef buttonSistem
#define buttonSistem

struct tBox
{
    int x0;
    int y0;
    int x1;
    int y1;
};

tBox FULL_SCREEN_BOX;

typedef enum
{
    onDown,
    onUp,
    onDrag
} tListenerType;

typedef void (*tListener)(int x, int y);

struct tButton
{
    tBox box;
    tListenerType listenerType;
    tListener function;
    int color;
    const char *text;
    int textColor;
    bool pressed;
    bool draw;
    bool clicable;
    int textSize;
    int textFont;
};

struct tGrid
{
    tBox box;
    int marginInside;
    int marginOutside;
    int w;
    int h;
};

const int MAX_ONSCREEN_BUTTONS = 50;
struct tButtonList
{
    tButton buttons[MAX_ONSCREEN_BUTTONS];
    int counter = 0;
};

tButtonList buttonList;

tGrid createGrid(tBox box, int marginInside, int marginOutside, int w, int h)
{
    return {box, marginInside, marginOutside, w, h};
}

tBox boxMerge(tBox A, tBox B)
{
    return {min(A.x0, B.x0), min(A.y0, B.y0), max(A.x1, B.x1), max(A.y1, B.y1)};
}

tBox Cell(tGrid grid, int x, int y)
{
    tBox ret;

    int baseX = grid.box.x0;
    int baseY = grid.box.y0;

    int TrueW = (grid.box.x1 - grid.box.x0) / grid.w;
    int TrueH = (grid.box.y1 - grid.box.y0) / grid.h;

    ret.x0 = baseX + TrueW * x;
    ret.x1 = baseX + TrueW * (x + 1);

    ret.y0 = baseY + TrueH * y;
    ret.y1 = baseY + TrueH * (y + 1);

    if (x == 0)
    {
        ret.x0 += grid.marginOutside;
        ret.x1 -= grid.marginInside / 2;
    }
    else if (x == grid.w - 1)
    {
        ret.x0 += grid.marginInside / 2;
        ret.x1 -= grid.marginOutside;
    }
    else
    {
        ret.x0 += grid.marginInside / 2;
        ret.x1 -= grid.marginInside / 2;
    }

    if (y == 0)
    {
        ret.y0 += grid.marginOutside;
        ret.y1 -= grid.marginInside / 2;
    }
    else if (y == grid.h - 1)
    {
        ret.y0 += grid.marginInside / 2;
        ret.y1 -= grid.marginOutside;
    }
    else
    {
        ret.y0 += grid.marginInside / 2;
        ret.y1 -= grid.marginInside / 2;
    }

    return ret;
}

void createButton(struct tBox box, tListenerType listenerType, tListener function, int color, const char *text, int textColor)
{
    struct tButton wip = {box, listenerType, function, color, text, textColor, false, true, true, 2, 2};
    //Serial.println("buttonList.counter");
    //Serial.println(buttonList.counter);
    buttonList.buttons[buttonList.counter] = wip;
    buttonList.counter++;
}

void createInterationArea(struct tBox box, tListenerType listenerType, tListener function)
{
    struct tButton wip = {box, listenerType, function, 0, "", 0, false, false, true, 2, 2};
    buttonList.buttons[buttonList.counter] = wip;
    buttonList.counter++;
}

void createTextBox(struct tBox box, int color, const char *text, int textColor, int textSize, int textFont)
{
    struct tButton wip = {box, onUp, [](int x, int y) {}, color, text, textColor, false, true, false, textSize, textFont};
    buttonList.buttons[buttonList.counter] = wip;
    buttonList.counter++;
}

void clearButtons()
{
    buttonList.counter = 0;
}

bool insideBox(int x, int y, tBox box)
{
    //Logger logger("insideBox");
    bool ret = x > box.x0 && x < box.x1 && y > box.y0 && y < box.y1;
    //logger("ret: %d", (int)ret);
    return ret;
}

void drawBox(tBox box, int col)
{
    ttgo->tft->fillRect(box.x0, box.y0, box.x1 - box.x0, box.y1 - box.y0, col);
}

void drawButtons()
{
    for (int i = 0; i < buttonList.counter; i++)
    {
        if (buttonList.buttons[i].draw)
        {
            //Serial.printf("drawing %d \n", i);

            const int borderSize = 2;
            const tButton b = buttonList.buttons[i];
            int r, g, blue;
            destructurateRGB(b.color, r, g, blue);
            int k = 40;
            int borderCol = createRGB(max(0, r - k), max(0, g - k), max(0, blue - k));
            int fillCol = b.color;

            if (b.pressed && b.clicable)
            {
                int kk = borderCol;
                borderCol = fillCol;
                fillCol = kk;
            }

            ttgo->tft->fillRect(b.box.x0, b.box.y0, b.box.x1 - b.box.x0, b.box.y1 - b.box.y0, borderCol);
            ttgo->tft->fillRect(b.box.x0 + borderSize, b.box.y0 + borderSize, b.box.x1 - b.box.x0 - borderSize * 2, b.box.y1 - b.box.y0 - borderSize * 2, fillCol);
            drawText(b.text, borderSize + 3 + b.box.x0, b.box.y0, b.textSize, b.textFont, b.textColor);
        }
    }
}

#endif
#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\logger.ino"

#define LOGGER_INCLUDED

#ifndef LOGGER_INCLUDED

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

class Logger{

private:
    char _function[100];
    int _indent;

    static int indent;

    static void log(int indent, const char *fmt, va_list args){
        char buffer[1000];
        char indentation[1000];
        vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
        indentation[0] = '\0';
        for (int i = 0; i < indent; i++)
        {
            strncat(indentation, "  ", sizeof(indentation) - 1);
        }

        Serial.printf("%s%s\n", indentation, buffer);
    }

    static bool accept(const char *function ){
        const char *disabled[] = {
            "insideBox",  "timerTick"
        };

        for( int i = 0 ; i < sizeof(disabled)/sizeof(*disabled) ; i++ ){
            if( !strcmp(function,disabled[i]) ){
                return false;
            }
        }
        return true;
    }

public:

    Logger(){
        Logger("");
    }

    Logger(const char* function){
        strncpy(_function, function, sizeof(_function) - 1);
        if( !Logger::accept(_function) ){
            return;
        }
        _indent = Logger::indent;

        operator()("--> %s", _function);
        Logger::indent += 1;
        _indent = Logger::indent;

    }

    void operator()(const char* fmt, ... ){
        if (!Logger::accept(_function))
        {
            return;
        }
        va_list args;
        va_start(args, fmt);
        log(_indent,fmt,args);
        va_end(args);
    }

    ~Logger(){
        if (!Logger::accept(_function))
        {
            return;
        }
        Logger::indent -= 1;
        _indent = Logger::indent;
        operator()("<-- %s", _function);
    }

 
};

int Logger::indent = 0;

#endif

#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\ttgoGlovalDeclarations.ino"
#ifndef ttgoGlovalDeclarations
#define ttgoGlovalDeclarations

// Must be defined before include LilyGoWatch librarie
// Uncomment the line corresponding your T-Watch
// #define LILYGO_WATCH_2019_WITH_TOUCH     // To use T-Watch2019 with touchscreen, please uncomment this line
// #define LILYGO_WATCH_2019_NO_TOUCH    // To use T-Watch2019 Not touchscreen , please uncomment this line
#define LILYGO_WATCH_2020_V1 //To use T-Watch2020, please uncomment this line


#define LILYGO_WATCH_HAS_MOTOR
// #define LILYGO_WATCH_LVGL

#include <LilyGoWatch.h>
#include <Arduino.h>

TTGOClass *ttgo;

struct tPermanent
{
    int brightness = 255;
    bool carillon = true;
};

RTC_DATA_ATTR tPermanent permanent;

bool drawn = false;
bool firstLoop = false;
bool invalidate = false;

// touch data n stuff

int startClickX = -1;
int startClickY = -1;

typedef enum
{
    launcher,
    OCR,

    dataMonitor,
    watch,
    flashLight,
    calculator,
    countdown,
    timer,
    teamScores,
    controlPannel, // brigtness(rtc_mem), carillon(rtc_mem), battery stats
    baseConversor,
    morse,
    calendar,
    turnOff,

} tApp;
tApp app = watch;

#endif


#line 1 "c:\\Users\\Jaime\\Desktop\\TTGO\\MAIN\\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"

#ifndef Utils
#define Utils

// cosas del asleep() para no molestar mientras duermes
const int SleepingHours = 10;
const int timeToSleep = 23;
const int timeToWakeUp = 9;

const char *monthsToWord[] = {
    "none",
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Ago",
    "Sep",
    "Oct",
    "Nob",
    "Dec"};

const char *WeekDaysToWord[] = {
    "none",
    "Lun",
    "Mar",
    "Mie",
    "Jue",
    "Vie",
    "Sab",
    "Dom"};
const char *DayToLable[7] = {"L", "M", "X", "J", "V", "S", "D"};
const int DaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char *DayToText[32] = {
    "cero",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
    "17",
    "18",
    "19",
    "20",
    "21",
    "22",
    "23",
    "24",
    "25",
    "26",
    "27",
    "28",
    "29",
    "30",
    "31"};
int planedDeepSleepTime = 0;
int planedScreenSleepTime = 0;
int planedButtonCoolDown = 0;
int w, h;

const int MaxBrightness = 255;
const int MinBrightness = 10;
const int brightnessBarMargin = 10;

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

int createRGB(int r, int g, int b)
{
    return ttgo->tft->color565(r, g, b);
    //return ((r & 31) << 11) + ((g & 31) << 6) + (b & 31);
}

int rgb(int r, int g, int b)
{
    return createRGB(r << 3, g << 3, b << 3);
}

int RED_CANCEL;
int GREEN_ALLOW;

typedef enum
{
    none,
    button
} tInterrupt;
tInterrupt interrupt = none;

void destructurateRGB(int col, int &r, int &g, int &b)
{
    b = col & 0x1F;
    col >> 5;
    g = col & 0x3F;
    col >> 6;
    r = col & 0x1F;

    b = b << 3;
    g = g >> 1;
    g = g << 3;
    r = r << 3;
}

char toLowerChar(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A' + 'a';
    }
    return c;
}

bool asleep(int h)
{
    return !(h > timeToWakeUp && h < timeToSleep);
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

int getUsableTime()
{
    return millis() / 1000;
    // int year, month, day, hour, minute, seconds;
    // getTime(year, month, day, hour, minute, seconds);
    // return seconds + 60 * (minute + 60 * (hour + 24 * (day + 30 * (month + 12 * (year - 2020)))));
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

void drawText(const char *t, int x, int y, int size, int font, int col)
{
    ttgo->tft->setTextColor(col);

    ttgo->tft->setTextFont(size);
    ttgo->tft->setTextSize(font);

    ttgo->tft->setCursor(x, y);
    ttgo->tft->println(t);
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

void interaction()
{
    int time = getUsableTime();
    planedScreenSleepTime = time + 30;
    planedDeepSleepTime = time + 45;
}

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

template <class T>
T d(T xa, T ya, T xb, T yb)
{
    int X = xa - xb;
    int Y = ya - yb;
    return sqrt(X * X + Y * Y);
}

int d(int xa, int ya, int xb, int yb)
{
    int X = xa - xb;
    int Y = ya - yb;
    return sqrt(X * X + Y * Y);
}

int getBatteryCorrectedPorcentage()
{
    return ttgo->power->isChargeing() ? ttgo->power->getBattPercentage() : int(ttgo->power->getBattPercentage() * (100. / 128.));
}

#endif

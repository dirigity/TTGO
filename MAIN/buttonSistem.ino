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
    String text;
    int textColor;
    bool pressed;
    bool draw;
};

const int MAX_ONSCREEN_BUTTONS = 10;
struct tButtonList
{
    tButton buttons[MAX_ONSCREEN_BUTTONS];
    int counter = 0;
};

tButtonList buttonList;

void createButton(struct tBox box, tListenerType listenerType, tListener function, int color, String text, int textColor)
{
    struct tButton wip = {box, listenerType, function, color, text, textColor, false, true};
    //Serial.println("buttonList.counter");
    //Serial.println(buttonList.counter);
    buttonList.buttons[buttonList.counter] = wip;
    buttonList.counter++;
}

void createInterationArea(struct tBox box, tListenerType listenerType, tListener function)
{
    struct tButton wip = {box, listenerType, function, 0, "", 0, false, false};
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

void drawBox(tBox box, int col){
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
}

#endif
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
    char* text;
    int textColor;
    bool pressed;
    bool draw;
};

struct tGrid
{
    tBox box;
    int marginInside;
    int marginOutside;
    int w;
    int h;
};

const int MAX_ONSCREEN_BUTTONS = 10;
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

void createButton(struct tBox box, tListenerType listenerType, tListener function, int color, char* text, int textColor)
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
#include "C:\Users\Jaime\Desktop\TTGO\basicWatch\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\basicWatch\utils.ino"

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

struct tButton
{
    tBox box;
    void (*function)();
    int color;
    String text;
    int textColor;
    bool pressed;
    bool draw;
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
    struct tButton wip = {box, function, color, text, textColor, false, true};
    buttonList.buttons[buttonList.counter] = wip;
    buttonList.counter++;
}

void createButton(struct tBox box, void (*function)(), int color, String text, int textColor, bool draw)
{
    struct tButton wip = {box, function, color, text, textColor, false, draw};
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

void drawButtons()
{
    for (int i = 0; i < buttonList.counter; i++)
    {
        if (buttonList.buttons[i].draw)
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
}

#endif
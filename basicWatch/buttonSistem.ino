#include "C:\Users\Jaime\Desktop\TTGO\basicWatch\ttgoGlovalDeclarations.ino"

#ifndef buttonSistem
#define buttonSistem

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

#endif
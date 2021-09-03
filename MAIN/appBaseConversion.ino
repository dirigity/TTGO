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
#include "C:\Users\Jaime\Desktop\TTGO\basicWatch\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\basicWatch\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\basicWatch\buttonSistem.ino"

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
                box, []
                {
                    permanent.carillon = false;
                    drawn = false;
                },
                GREEN_ALLOW, "Apagar Carillon", createRGB(0, 0, 0));
        else
            createButton(
                box, []
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
            boxExit, []
            {
                app = launcher;
                drawn = false;
            },
            createRGB(255, 255, 255), "Back", 0x0000);
    }
}
#endif
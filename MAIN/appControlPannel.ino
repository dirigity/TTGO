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
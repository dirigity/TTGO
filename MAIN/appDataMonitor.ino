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
                 "day: %02d \n"
                 "time: %02d:%02d:%02d \n"
                 "power: %.2fV\n"
                 "batt: %02d%%\n"
                 "chargeing: %s\n",
                 year, monthsToWord[month], month, day, hour, minute, seconds, ttgo->power->getBattVoltage() / 1000., getBatteryCorrectedPorcentage(), ttgo->power->isChargeing() ? "True":"False");
        drawText(String(info), 0, 0, 2, 2, TFT_WHITE);

        lastDrawnTime = seconds;
    }

    if (lastDrawnTime != seconds)
    {
        invalidate = true;
    }
}

#endif
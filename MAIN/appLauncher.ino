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
    "timer",
    "teamScores",
    "controlPannel", // brigtness(rtc_mem), carillon(rtc_mem), battery stats
    "baseConversor",
    "morse",
    "calendar",
    "wifiPannel",
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
                //Serial.printf("Programa selecionado:%d,%s \n", selected + skipedApps, AppToString[selected + skipedApps]);
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

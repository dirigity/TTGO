#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appTimer
#define appTimer

int startTimerTime = 0;
int stopedTimerTime = 0;
int TimerRuning = false;
int lastLapTime = -1;

const int MaxStoredLaps = 5;
struct tLapList
{
    char laps[MaxStoredLaps][25];
    int counter = 0;
};

tLapList lapList;

int currentlyDrawnMillisSinceStart = 0;
int millisSinceStart = -1;

void getStringTime(char *buff)
{
    int hoursSinceStart = millisSinceStart / 1000 / 3600;
    int minutesSinceStart = (millisSinceStart / 1000 % 3600) / 60;
    int seconds = millisSinceStart / 1000 % 60;
    int millisToDisplay = millisSinceStart % 1000;

    if (hoursSinceStart == 0){
        sprintf(buff, "%02d:%02d.%03d", minutesSinceStart, seconds, millisToDisplay);
    }else{
        sprintf(buff, "%02d:%02d:%03d", hoursSinceStart,  minutesSinceStart, seconds);
    }


    //Serial.println(buff);
}

void mainTextDraw(const char *buff)
{
    const int posX = 20;
    const int posY = 15;

    ttgo->tft->fillRect(0, 0, w, h / 4, 0);
    drawText(buff, posX, posY, 2,3, 0xFFFF);
}

void drawLaps()
{
    ttgo->tft->fillRect(0, 70, w, 110, 0); // erase laps

    for (int i = 0; i < lapList.counter; i++)
    {
        drawText(lapList.laps[i], 30, 75 + i * 20, 1, 2, 0xFFFF);
    }
}

void timerTick()
{

    if (TimerRuning)
        millisSinceStart = millis() - startTimerTime;
    else
        millisSinceStart = stopedTimerTime - startTimerTime;

    if (millisSinceStart != currentlyDrawnMillisSinceStart || !drawn)
    {
        currentlyDrawnMillisSinceStart = millisSinceStart;
        char buff[20];
        getStringTime(buff);
        mainTextDraw(buff);
    }

    if (!drawn)
    {
        drawLaps();

        tBox boxStartAndStop = {20, 190, w / 2 - 20, 225};
        tBox boxLapAndReset = {w / 2 + 20, 190, w - 20, 225};
        if (!TimerRuning)
        {
            createButton(
                boxStartAndStop, onUp, [](int x, int y)
                {
                    if (startTimerTime == 0)
                    {
                        startTimerTime = millis();
                    }
                    else
                    {
                        startTimerTime = millis() - (stopedTimerTime - startTimerTime);
                    }
                    TimerRuning = true;
                    drawn = false;
                    currentlyDrawnMillisSinceStart = -1;
                },
                GREEN_ALLOW, "Start", 0x0000);
            createButton(
                boxLapAndReset, onUp, [](int x, int y)
                {
                    startTimerTime = 0;
                    stopedTimerTime = 0;
                    TimerRuning = false;
                    drawn = false;
                    millisSinceStart = 0;
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
                    stopedTimerTime = millis();
                    TimerRuning = false;
                    drawn = false;
                    currentlyDrawnMillisSinceStart = -1;
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

                    getStringTime(lapList.laps[lapList.counter]);

                    if (lastLapTime != -1)
                    {
                        int diferenceFromLast = (millis() - lastLapTime) / 1000;
                        sprintf(lapList.laps[lapList.counter], "%s (+%d)", lapList.laps[lapList.counter], diferenceFromLast);
                    }

                    lastLapTime = millis();

                    lapList.counter++;
                    drawLaps();
                },
                createRGB(255, 255, 30), "Lap", 0x0000);
        }
        if (startTimerTime == 0)
        {
            millisSinceStart = 0;
        }
    }

    // Serial.println("millisSinceStart");
    // Serial.println(millisSinceStart);
    // Serial.println("TimerRuning");
    // Serial.println(TimerRuning);
    // Serial.println("currentlyDrawnmillisSinceStart");
    // Serial.println(currentlyDrawnmillisSinceStart);
}

#endif
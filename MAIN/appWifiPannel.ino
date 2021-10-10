#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appWifiPannel
#define appWifiPannel

// launcher app

/*
amo a ver, que hace esto

conecto to wifo
sinc time
sinc trello
sinc güeder
https://api.nasa.gov/planetary/apod?api_key=DEMO_KEY&date=2019-03-17
https://api.nasa.gov/

curl -H "Accept: text/plain" https://icanhazdadjoke.com/ <- lo mas importante



*/
#include <WiFi.h>

const char *ssid = "Unknown";
const char *password = "1234567890asdf";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const int CONNECTION_TIME_OUT = 40000;

void wifiPannelTick()
{
    if (!drawn)
    {

        tGrid layout = createGrid(FULL_SCREEN_BOX, 10, 10, 1, 4);

        tBox toggleButton = Cell(layout, 0, 0);
        tBox timeButton = Cell(layout, 0, 1);
        tBox trelloButton = Cell(layout, 0, 2);
        tBox weatherButton = Cell(layout, 0, 3);

        int enabledIfWifiColor = (WiFi.status() == WL_CONNECTED) ? TFT_DARKCYAN : TFT_LIGHTGREY;

        createButton(
            toggleButton, onUp, [](int x, int y)
            {
                if (WiFi.status() != WL_CONNECTED)
                {
                    WiFi.begin(ssid, password);
                    int time = 0;
                    int col = random();
                    ttgo->tft->fillScreen(TFT_BLACK);

                    while (WiFi.status() != WL_CONNECTED && time < CONNECTION_TIME_OUT)
                    {
                        delay(50);
                        time += 50;
                        //Serial.print(".");
                        //tft->print(".");

                        //ttgo->tft->fillScreen(TFT_BLACK);
                        int r = 10;
                        int Dx = cos(millis() / 1000.) * (20 + 90 * cos(millis() / 600.));
                        int Dy = sin(millis() / 1000.) * (20 + 90 * cos(millis() / 600.));
                        if (time % 700 == 0)
                        {
                            col += random();
                        }
                        ttgo->tft->fillCircle(TFT_HEIGHT / 2 + Dx, TFT_WIDTH / 2 + Dy, r, col);
                    }

                    if (time >= CONNECTION_TIME_OUT)
                    {
                        WiFi.disconnect();
                    }
                }
                else
                {
                    WiFi.disconnect();
                }

                invalidate = true;
            },
            WiFi.status() != WL_CONNECTED ? TFT_RED : TFT_GREEN, "toggle wifi", TFT_BLACK);

        createButton(
            timeButton, onUp, [](int x, int y)
            {
                if (WiFi.status() == WL_CONNECTED)
                {
                }
            },
            enabledIfWifiColor, "sync time", TFT_BLACK);

        createButton(
            trelloButton, onUp, [](int x, int y)
            {
                if (WiFi.status() == WL_CONNECTED)
                {
                }
            },
            enabledIfWifiColor, "sync trello", TFT_BLACK);
        createButton(
            weatherButton, onUp, [](int x, int y)
            {
                if (WiFi.status() == WL_CONNECTED)
                {
                }
            },
            enabledIfWifiColor, "sync weather", TFT_BLACK);
    }
}

#endif

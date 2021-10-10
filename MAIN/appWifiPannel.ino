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
const int CONNECTION_TIME_OUT = 4000;

void wifiPannelTick()
{
    if (!drawn)
    {

        tGrid layout = createGrid(FULL_SCREEN_BOX, 10, 10, 1, 4);

        tBox toggleButton = Cell(layout, 0, 0);
        tBox timeButton = Cell(layout, 0, 1);
        tBox trelloButton = Cell(layout, 0, 2);
        tBox weatherButton = Cell(layout, 0, 3);

        int enabledIfWifiColor = (WiFi.status() == WL_CONNECTED) ? TFT_LIGHTGREY : TFT_DARKCYAN;

        createButton(
            toggleButton, onUp, [](int x, int y)
            {
                if (WiFi.status() != WL_CONNECTED)
                {
                    WiFi.begin(ssid, password);
                    WiFi.setSleep(false);
                    int time = 0;
                    int col = random();
                    ttgo->tft->fillScreen(TFT_BLACK);

                    Serial.println(WiFi.status());
                    int aviableNetworksN = WiFi.scanNetworks();
                    Serial.printf("Wifises(%d): \n", aviableNetworksN);
                    for (int i = 0; i < aviableNetworksN; i++)
                    {
                        Serial.println(WiFi.SSID(i));
                    }

                    while (WiFi.status() != WL_CONNECTED && time < CONNECTION_TIME_OUT)
                    {
                        delay(50);
                        time += 50;
                        //tft->print(".");

                        //ttgo->tft->fillScreen(TFT_BLACK);
                        int r = 5;
                        int Dx = cos(millis() / 1000.) * (20 + 90 * cos(millis() / 600.));
                        int Dy = sin(millis() / 1000.) * (20 + 90 * cos(millis() / 600.));
                        if (time % 700 == 0)
                        {
                            Serial.print(".");
                            col += random() / 1000000;
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
                    WiFi.disconnect(true);
                }

                invalidate = true;
            },
            WiFi.status() != WL_CONNECTED ? TFT_RED : TFT_GREEN, "toggle wifi", TFT_BLACK);

        createButton(
            timeButton, onUp, [](int x, int y)
            {
                if (WiFi.status() == WL_CONNECTED)
                {
                    bool gotTime = false;
                    while (!gotTime)
                    {
                        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
                        struct tm timeinfo;
                        gotTime = getLocalTime(&timeinfo);
                        if (!gotTime)
                        {
                            delay(3000);
                        }
                    }
                    ttgo->rtc->syncToRtc();
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

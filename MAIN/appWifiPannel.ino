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
#include "jsmn-master\jsmn.h"

const int CONNECTION_TIME_OUT = 20000;

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
                    const char *ssid = "Unknown";
                    const char *password = "1234567890asdf---";

                    WiFi.begin(ssid, password);
                    WiFi.setSleep(false);
                    int time = 0;
                    int col = random();
                    ttgo->tft->fillScreen(TFT_BLACK);

                    while (WiFi.status() != WL_CONNECTED && time < CONNECTION_TIME_OUT)
                    {
                        delay(50);
                        time += 50;
                        
                        int r = 5;
                        int Dx = cos(millis() / 1000.) * (20 + 90 * cos(millis() / 600.));
                        int Dy = sin(millis() / 1000.) * (20 + 90 * cos(millis() / 600.));
                        if (time % 700 == 0)
                        {
                            Serial.println(".");
                            col += random() / 10000000;
                        }
                        ttgo->tft->fillCircle(TFT_HEIGHT / 2 + Dx, TFT_WIDTH / 2 + Dy, r, col);
                    }

                    if (time >= CONNECTION_TIME_OUT)
                    {
                        WiFi.disconnect();
                        Serial.println("time out");
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
                    const char *ntpServer = "pool.ntp.org";
                    const long gmtOffset_sec = 3600;
                    const int daylightOffset_sec = 3600;
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
                Serial.println("trello fetching");

                if (WiFi.status() == WL_CONNECTED)
                {
                    const char *apiUrl = "https://trello.com/b/OA8Jy4rW.json";
                    const char *certificate =
                        "-----BEGIN CERTIFICATE-----\n"
                        "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
                        "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
                        "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
                        "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
                        "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
                        "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
                        "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"
                        "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"
                        "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"
                        "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"
                        "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"
                        "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"
                        "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"
                        "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"
                        "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"
                        "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"
                        "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"
                        "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"
                        "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
                        "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
                        "-----END CERTIFICATE-----";
                    char *recived = fetch(apiUrl, certificate);

                    { // parse scope

                        jsmn_parser p;
                        jsmn_init(&p);

                        const int TOK_N = 100;
                        jsmntok_t t[TOK_N]; /* We expect no more than TOK_N JSON tokens */

                        int r = jsmn_parse(&p, recived, strlen(recived), t, TOK_N);

                        //serial.printf("encontre %d tokens", r);
                    }
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

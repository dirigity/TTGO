#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appTeamScores
#define appTeamScores

int ScoreA = 0;
int ScoreB = 0;

void reset()
{
    ScoreA = 0;
    ScoreB = 0;
    redrawScores();
}

const int margin = 25;
const int size = 50;
const int resetButtonW = 40;

void redrawScores()
{
    char buff[40];
    sprintf(buff, "%d - %d", ScoreA, ScoreB);

    int backOffset = (ScoreA > 9) * 20;

    ttgo->tft->fillRect(0, h / 2 - 30, w, h / 2 + 30, TFT_BLACK);
    drawText(String(buff), w / 2 - 50 - backOffset, h / 2 - 23, 2, 3, TFT_WHITE);
}

void TeamScoresTick()
{

    tBox APlus = {margin, margin, margin + size, margin + size};
    tBox AMinus = {margin, h - margin - size, margin + size, h - margin};
    tBox BPlus = {w - margin - size, margin, w - margin, margin + size};
    tBox BMinus = {w - margin - size, h - margin - size, w - margin, h - margin};

    if (!drawn)
    {
        redrawScores();

        createButton(
            APlus, onUp, [](int x, int y)
            {
                ScoreA++;
                redrawScores();
            },
            GREEN_ALLOW, "+", TFT_BLACK);
        createButton(
            AMinus, onUp, [](int x, int y)
            {
                ScoreA--;
                ScoreA = max(0, ScoreA);
                redrawScores();
            },
            RED_CANCEL, "-", TFT_WHITE);
        createButton(
            BPlus, onUp, [](int x, int y)
            {
                ScoreB++;
                redrawScores();
            },
            GREEN_ALLOW, "+", TFT_BLACK);
        createButton(
            BMinus, onUp, [](int x, int y)
            {
                ScoreB--;
                ScoreB = max(0, ScoreB);
                redrawScores();
            },
            RED_CANCEL, "-", TFT_WHITE);
        tBox resetBox = {w / 2 - resetButtonW, margin, w / 2 + resetButtonW, margin + size};

        createButton(
            resetBox, onUp, [](int x, int y)
            { 
                reset(); 
            },
            RED_CANCEL, "Reset", TFT_WHITE);

    }
}

#endif
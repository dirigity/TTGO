#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appWatch
#define appWatch

const double secondsMstart = .15;
const double secondsMend = .3;

const double minuteMstart = .35;
const double minuteMend = .5;

const double hourMstart = .55;
const double hourMend = .75;

const double battMstart = .8;
const double battMend = .9;

const double ringMstart = .95;
const double ringMend = .99;

// calculations

const double midSecondsM = (secondsMstart + secondsMend) / 2;
const double secondsMThickness = secondsMend - secondsMstart;

const double midMinuteM = (minuteMstart + minuteMend) / 2;
const double minuteMThickness = minuteMend - minuteMstart;

const double midHourM = (hourMstart + hourMend) / 2;
const double hourMThickness = hourMend - hourMstart;

const double midBattM = (battMstart + battMend) / 2;
const double battMThickness = battMend - battMstart;

void drawPolarSegment(double angle, double startM, double endM, int col) //, int darkerCol)
{
    int x0 = w / 2 + sin(angle) * startM;
    int y0 = h / 2 - cos(angle) * startM;
    int x1 = w / 2 + sin(angle) * endM;
    int y1 = h / 2 - cos(angle) * endM;

    ttgo->tft->drawLine(x0, y0, x1, y1, col);

    ttgo->tft->drawLine(x0 + 1, y0, x1 + 1, y1, col);
    ttgo->tft->drawLine(x0, y0 + 1, x1, y1 + 1, col);
}

double capRoundFunctionCeroToOne(double i)
{
    return sqrt(1 - (i * i));
}

double CapRoundness(double in, double midRad, double Thickness)
{

    double perimeter = 2 * PI * midRad;
    double perimeterRadiousRelation = 1 - (Thickness / perimeter);

    if (in < perimeterRadiousRelation)
    {
        return 1;
    }
    else
    {
        //return .5;
        double i = (in - perimeterRadiousRelation) / (Thickness / perimeter);

        return capRoundFunctionCeroToOne(i);
    }
}

void manageDisc(double clockAngle, double timeAngle, double midsM, double MThickness, int r, int g, int b)
{
    double Intensity = angle(clockAngle - timeAngle) / (2 * PI);
    //double Intensity_ = max(0., Intensity - 0.01);
    double CurrentCapRoundness = CapRoundness(Intensity, midsM, MThickness);
    double Mstart_ = midsM - MThickness / 2 * CurrentCapRoundness;
    double Mend_ = midsM + MThickness / 2 * CurrentCapRoundness;

    if (CurrentCapRoundness != 1)
    {
        drawPolarSegment(clockAngle, h / 2 * (midsM - MThickness / 2), h / 2 * Mstart_, TFT_BLACK);
        drawPolarSegment(clockAngle, h / 2 * Mend_, h / 2 * (midsM + MThickness / 2), TFT_BLACK);
    }

    drawPolarSegment(clockAngle, h / 2 * Mstart_, h / 2 * Mend_, createRGB(Intensity * r, Intensity * g, Intensity * b));
}

double secondDrawingAngle = 0;
double minuteDrawingAngle = 0;
double hourDrawingAngle = 0;
double battDrawingAngle = 0;

double battAngle = 0;

void watchTick(int year, int month, int day, int hour, int minute, int seconds, int UsableTime)
{
    // sleep after some time without activity;
    {
        if (planedScreenSleepTime < UsableTime)
        {
            ttgo->bl->off();
            setCpuFrequencyMhz(80);
        }

        if (planedDeepSleepTime < UsableTime)
        { // 30s despues de apagar la pantalla entrar en sueño profundo
            enterDeepSleep();
        }
    }

    // draw corona
    if (!drawn)
    {
        createInterationArea(
            FULL_SCREEN_BOX, onUp, [](int x, int y)
            {
                if (y - startClickY > 80)
                {
                    enterDeepSleep();
                }
            });

        int ringMmid = (ringMend + ringMstart) / 2 * h / 2;

        ttgo->tft->fillCircle(w / 2, h / 2, h * ringMend / 2, createRGB(50, 50, 50));
        ttgo->tft->fillCircle(w / 2, h / 2, h * ringMstart / 2, 0x0000);

        for (int i = 0; i < 60; i++)
        {
            double angle = 2 * PI / 60 * i;
            ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 2, 0x0000);
            //Serial.println(double(h) * sin(angle));
        }

        for (int i = 0; i < 12; i++)
        {
            double angle = 2 * PI / 12 * i;
            if (i % 3 == 0)
                ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 4, createRGB(200, 200, 200));
            else
                ttgo->tft->fillCircle(w / 2 + ringMmid * sin(angle), h / 2 + ringMmid * cos(angle), 4, createRGB(150, 150, 150));

            //Serial.println(double(h) * sin(angle));
        }

        //Serial.println("[DONE] drawing corona");
    }

    double secondAngle = double(seconds) / 60. * 2. * PI;
    double minuteAngle = double(minute) / 60. * 2. * PI + secondAngle / 60.;
    double hourAngle = double(hour) / 12. * 2. * PI + minuteAngle / 12.;
    if (battAngle == 0)
    {
        battAngle = (getBatteryCorrectedPorcentage() / 100. * 2. * PI);
    }
    else
    {
        battAngle = battAngle * 0.95 + (getBatteryCorrectedPorcentage() / 100. * 2. * PI) * 0.05;
    }

    // draw gradient circles
    {
        for (int i = 0; i < 200; i++)
        {
            secondDrawingAngle += 0.03123123;
            secondDrawingAngle = angle(secondDrawingAngle);
            manageDisc(secondDrawingAngle, secondAngle, midSecondsM, secondsMThickness, 0, 0, 255);
        }

        for (int i = 0; i < (drawn ? 40 : 300); i++)
        {
            minuteDrawingAngle += 0.02123123;
            minuteDrawingAngle = angle(minuteDrawingAngle);
            manageDisc(minuteDrawingAngle, minuteAngle, midMinuteM, minuteMThickness, 0, 0, 255);
        }

        for (int i = 0; i < (drawn ? 40 : 700); i++)
        {
            hourDrawingAngle += 0.01123123;
            hourDrawingAngle = angle(hourDrawingAngle);
            manageDisc(hourDrawingAngle, hourAngle, midHourM, hourMThickness, 0, 0, 255);
        }

        for (int i = 0; i < (drawn ? 40 : 700); i++)
        {
            battDrawingAngle += 0.01123123;
            battDrawingAngle = angle(battDrawingAngle);
            if (battAngle < 1)
                manageDisc(battDrawingAngle, battAngle, midBattM, battMThickness, 255, 0, 0);
            else
                manageDisc(battDrawingAngle, battAngle, midBattM, battMThickness, 0, 0, 255);
        }
    }
}

#endif
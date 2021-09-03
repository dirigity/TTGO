#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"

#ifndef Utils
#define Utils

// cosas del asleep() para no molestar mientras duermes
const int SleepingHours = 10;
const int timeToSleep = 23;
const int timeToWakeUp = 9;

const char *monthsToWord[] = {
    "none",
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Ago",
    "Sep",
    "Oct",
    "Nob",
    "Dec"};

const char *WeekDaysToWord[] = {
    "none",
    "Lun",
    "Mar",
    "Mie",
    "Jue",
    "Vie",
    "Sab",
    "Dom"};

int planedDeepSleepTime = 0;
int planedScreenSleepTime = 0;
int planedButtonCoolDown = 0;
int w, h;

const int MaxBrightness = 255;
const int MinBrightness = 10;
const int brightnessBarMargin = 10;

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

int createRGB(int r, int g, int b)
{
    return ttgo->tft->color565(r, g, b);
    //return ((r & 31) << 11) + ((g & 31) << 6) + (b & 31);
}

int RED_CANCEL;
int GREEN_ALLOW;

typedef enum
{
    none,
    button
} tInterrupt;
tInterrupt interrupt = none;

void destructurateRGB(int col, int &r, int &g, int &b)
{
    b = col & 0x1F;
    col >> 5;
    g = col & 0x3F;
    col >> 6;
    r = col & 0x1F;

    b = b << 3;
    g = g << 2;
    r = r << 3;
}

char toLowerChar(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A' + 'a';
    }
    return c;
}

bool asleep(int h)
{
    return !(h > timeToWakeUp && h < timeToSleep);
}

void getTime(int &year, int &month, int &day, int &h, int &m, int &s)
{
    RTC_Date ret = ttgo->rtc->getDateTime();

    year = ret.year;
    month = ret.month;
    day = ret.day;
    h = ret.hour;
    m = ret.minute;
    s = ret.second;
}

int getUsableTime()
{
    return millis() / 1000;
    // int year, month, day, hour, minute, seconds;
    // getTime(year, month, day, hour, minute, seconds);
    // return seconds + 60 * (minute + 60 * (hour + 24 * (day + 30 * (month + 12 * (year - 2020)))));
}

void enterDeepSleep()
{ // manage next automatic wakeup

    Serial.println("starting deep sleep");

    int year, month, day, hour, minute, seconds;
    getTime(year, month, day, hour, minute, seconds);
    if (permanent.carillon)
    {
        int waitTime = 0;

        if (!asleep(hour + 1))
        { // wake up next hour o'Clock
            waitTime = ((60 - minute) * 60 - 60);
        }
        else
        { // wake up tomorrow
            waitTime = ((60 - minute) * 60 + SleepingHours * 3600 - 60);
        }

        Serial.printf("Saldre del deep sleep %u \n", waitTime);

        esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * waitTime);
    }
    else
    {
        Serial.printf("no saldre del deep sleep automaticamente porque el carillon esta desactivado \n");
    }

    esp_deep_sleep_start();
}

void drawText(const char *t, int x, int y, int size, int font, int col)
{
    ttgo->tft->setTextColor(col);

    ttgo->tft->setTextFont(size);
    ttgo->tft->setTextSize(font);

    ttgo->tft->setCursor(x, y);
    ttgo->tft->println(t);
}

void ToggleOnOff()
{

    drawn = false;
    app = watch;

    ttgo->power->readIRQ();
    if (ttgo->power->isPEKShortPressIRQ())
    { // if short click turn off screen
        ttgo->power->clearIRQ();
        ttgo->bl->reverse();

        if (ttgo->bl->isOn())
        {
            setCpuFrequencyMhz(240);
            planedButtonCoolDown = getUsableTime() + 5;
        }
        else
        {
            planedDeepSleepTime = getUsableTime() + 15;
            setCpuFrequencyMhz(80);
        }
    }
    else
    { // if long click enter deepSleep
        ttgo->power->clearIRQ();
        enterDeepSleep();
    }
}

void interaction()
{
    int time = getUsableTime();
    planedScreenSleepTime = time + 30;
    planedDeepSleepTime = time + 45;
}

bool operator>(RTC_Date a, RTC_Date b)
{
    if (a.year > b.year)
    {
        return true;
    }
    else if (a.year == b.year)
    {
        if (a.month > b.month)
        {
            return true;
        }
        else if (a.month == b.month)
        {
            if (a.day > b.day)
            {
                return true;
            }
            else if (a.day == b.day)
            {
                if (a.hour > b.hour)
                {
                    return true;
                }
                else if (a.hour == b.hour)
                {
                    if (a.minute > b.minute)
                    {
                        return true;
                    }
                    else
                    {
                        if (a.minute == b.minute)
                        {
                            if (a.second > b.second)
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

void carillon(int h)
{
    if (!permanent.carillon)
        return;
    ttgo->motor->onec(1000);
    delay(2000);
    // base 1
    //while (h > 0)
    // {
    //   //Serial.println(h);

    //   h--;
    //   ttgo->motor->onec(100);
    //   delay(400);
    // }
    // base 2

    while (h > 0)
    {
        if (h % 2 == 1)
        {
            ttgo->motor->onec(700);
            delay(1000);
        }
        else
        {
            ttgo->motor->onec(100);
            delay(1000);
        }
        h /= 2;
    }
    interaction();
}

double angle(double a)
{
    while (a > 2 * PI)
    {
        a -= 2 * PI;
    }
    while (a < 0)
    {
        a += 2 * PI;
    }
    return a;
}

template <class T>
T maximum(T a, T b)
{
    return a > b ? a : b;
}

template <class T>
T minimum(T a, T b)
{
    return a < b ? a : b;
}

template <class T>
T d(T xa, T ya, T xb, T yb)
{
    int X = xa - xb;
    int Y = ya - yb;
    return sqrt(X * X + Y * Y);
}

int d(int xa, int ya, int xb, int yb)
{
    int X = xa - xb;
    int Y = ya - yb;
    return sqrt(X * X + Y * Y);
}

int getBatteryCorrectedPorcentage()
{
    return ttgo->power->isChargeing() ? ttgo->power->getBattPercentage() : int(ttgo->power->getBattPercentage() * (100. / 128.));
}

#endif
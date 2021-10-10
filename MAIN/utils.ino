#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"

#ifndef Utils
#define Utils

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
    "Dom",
    "Lun",
    "Mar",
    "Mie",
    "Jue",
    "Vie",
    "Sab",
};
const char *DayToLable[7] = {"D", "L", "M", "X", "J", "V", "S"};
const int DaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char *DayToText[32] = {
    "cero",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
    "17",
    "18",
    "19",
    "20",
    "21",
    "22",
    "23",
    "24",
    "25",
    "26",
    "27",
    "28",
    "29",
    "30",
    "31"};
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

int rgb(int r, int g, int b)
{
    return createRGB(r << 3, g << 3, b << 3);
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
    g = g >> 1;
    g = g << 3;
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

void enterDeepSleep()
{ // manage next automatic wakeup, it will brick the phone!!!!

    Serial.println("starting deep sleep");

    int year, month, day, hour, minute, seconds;
    getTime(year, month, day, hour, minute, seconds);
    if (permanent.carillon)
    {
        int waitTime = 0;
        // wake up next hour oclock
        waitTime = ((60 - minute) * 60 - 60);

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

void interaction()
{
    planedScreenSleepTime = millis() + 30000;
}

// void softSleepWasteFull()
// {
//     ttgo->bl->off();
//     setCpuFrequencyMhz(80);
//     delay(200);
// }

void softSleep()
{
    //permanent.lightSleepMode = true;
    //esp_sleep_enable_timer_wakeup(0.25 * uS_TO_S_FACTOR);
    ttgo->bl->off();
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_38, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_light_sleep_start();
}

void turnOn()
{
    if (!ttgo->bl->isOn())
    {
        ttgo->bl->on();
        app = watch;
        invalidate = true;
        setCpuFrequencyMhz(240);
        interaction();
    }
}

void ButtonIRQAnalize()
{

    drawn = false;
    app = watch;

    ttgo->power->readIRQ();
    if (ttgo->power->isPEKShortPressIRQ())
    { // if short click turn off screen
        ttgo->power->clearIRQ();
        softSleep();
    }
    else
    { // if long click enter deepSleep
        ttgo->power->clearIRQ();
        enterDeepSleep();
    }
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

    bool abort = false;
    while (h > 0 && !abort)
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
        int16_t kk;
        abort = ttgo->getTouch(kk, kk);
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

double map(double StartRangeSrc, double EndRangeSrc, double StartRangeDst, double EndRangeDst, double val)
{
    return StartRangeDst + ((EndRangeDst - StartRangeDst) / (EndRangeSrc - StartRangeSrc)) * (val - StartRangeSrc);
}

const double MAX_VOLTAGE = 4.1877;
const double MIN_VOLTAGE = 3.;

int getBatteryCorrectedPorcentage()
{
    double currentV = ttgo->power->getBattVoltage() / 1000;
    int ret = int(map(MIN_VOLTAGE, MAX_VOLTAGE, 0, 100, currentV));
    //Serial.printf("voltage:%f, ret:%d \n",currentV,ret);
    return ret;

    //return ttgo->power->isChargeing() ? ttgo->power->getBattPercentage() : int(ttgo->power->getBattPercentage() * (100. / 128.));
}

#endif
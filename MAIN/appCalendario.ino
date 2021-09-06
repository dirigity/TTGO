
#ifndef appCalendario
#define appCalendario

#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"

int day;
int month;
int year;

char title[10];

void CalendarioTick()
{
    if (firstLoop)
    {
        Serial.println("Starting Calendario and syncing date");
        {
            int k, kk, kkk;
            getTime(year, month, day, k, kk, kkk);
        }
    }
    if (!drawn)
    {
        const tGrid mainLayout = createGrid(FULL_SCREEN_BOX, 10, 10, 1, 5);
        const tBox header = Cell(mainLayout, 0, 0);
        const tBox body = boxMerge(Cell(mainLayout, 0, 1), Cell(mainLayout, 0, 4));
        const tGrid bodyLayout = createGrid(body, 0, 0, 7, 7);

        const tGrid headerLayout = createGrid(header, 0, 2, 5, 1);
        const tBox LastMonthBox = Cell(headerLayout, 0, 0);
        const tBox NextMonthBox = Cell(headerLayout, 4, 0);
        const tBox DateDisplayBox = boxMerge(Cell(headerLayout, 1, 0), Cell(headerLayout, 3, 0));

        createButton(
            LastMonthBox, onUp, [](int x, int y)
            {
                month--;
                if (month == 0)
                {
                    month = 12;
                    year--;
                }
                drawn = false;
            },
            TFT_WHITE, "<", TFT_BLACK);

        createButton(
            NextMonthBox, onUp, [](int x, int y)
            {
                month++;
                if (month == 13)
                {
                    month = 1;
                    year++;
                }
                drawn = false;
            },
            TFT_WHITE, ">", TFT_BLACK);

        sprintf(title, "%s %d", monthsToWord[month], year);
        createButton(
            DateDisplayBox, onUp, [](int x, int y)
            {
                int k;
                // open the year selection screen
            },
            TFT_WHITE, title, TFT_BLACK);

        for (int i = 0; i < 7; i++)
        {
            tBox box = Cell(bodyLayout, i, 0);
            createTextBox(box, rgb(20, 20, 20), DayToLable[i], TFT_BLACK, 2, 1); // top lables
        }

        int lastDayFromLastMonth = DaysInMonth[month - 2];
        int weekDay = ttgo->rtc->getDayOfWeek(lastDayFromLastMonth, month - 1, year);

        while (weekDay != 6)
        {
            createTextBox(Cell(bodyLayout, weekDay, 1), rgb(15, 15, 15), DayToText[lastDayFromLastMonth], TFT_BLACK, 2, 1);
            lastDayFromLastMonth--;
            weekDay = ttgo->rtc->getDayOfWeek(lastDayFromLastMonth, month - 1, year);
        }

        int week = 1;
        for (int i = 1; i < DaysInMonth[month - 1] + 1; i++)
        {
            int weekDay = ttgo->rtc->getDayOfWeek(i, month, year);

            createTextBox(Cell(bodyLayout, weekDay, week), weekDay > 4 ? rgb(25, 25, 25) : TFT_WHITE, DayToText[i], TFT_BLACK, 2, 1);
            if (weekDay == 6)
                week++;
        }
        // int i = 1;
        // while (week < 7)
        // {
        //     int weekDay = ttgo->rtc->getDayOfWeek(i, month+1, year);

        //     createTextBox(Cell(bodyLayout, weekDay, week), rgb(15, 15, 15), DayToText[i], TFT_BLACK, 2, 1);
        //     if (weekDay == 6)
        //         week++;
        //     i++;
        // }
    }
}

#endif
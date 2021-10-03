
#ifndef appCalendario
#define appCalendario

#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"

int day;
int month;
int year;

int currentMonth;
int currentYear;

char title[10];

void CalendarioTick()
{
    if (firstLoop)
    {
        Serial.println("Starting Calendario and syncing date");
        {
            int k, kk, kkk;
            getTime(year, month, day, k, kk, kkk);
            getTime(currentYear, currentMonth, day, k, kk, kkk);
        }
    }
    if (!drawn)
    {
        const tGrid mainLayout = createGrid(FULL_SCREEN_BOX, 10, 10, 1, 5);
        const tBox header = Cell(mainLayout, 0, 0);
        const tBox body = boxMerge(Cell(mainLayout, 0, 1), Cell(mainLayout, 0, 4));
        const tGrid bodyLayout = createGrid(body, 0, 0, 7, 7);
        int week = 1;

        {
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
                TFT_WHITE, " <", TFT_BLACK);

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
                TFT_WHITE, " >", TFT_BLACK);

            sprintf(title, "%s %d", monthsToWord[month], year);
            createButton(
                DateDisplayBox, onUp, [](int x, int y)
                {
                    int k;
                    // open the year selection screen
                },
                TFT_WHITE, title, TFT_BLACK);
        }
        {
            for (int i = 0; i < 7; i++)
            {
                tBox box = Cell(bodyLayout, i, 0);
                createTextBox(box, rgb(20, 20, 20), DayToLable[i], TFT_BLACK, 2, 1); // top lables
            }
        }
        {
            int lastMonthIndex = (month - 2 >= 0) ? month - 2 : 11;
            int lastMonth = month - 1 == 0 ? 12 : month - 1;
            int lastDayFromLastMonth = DaysInMonth[lastMonthIndex];
            int weekDay = ttgo->rtc->getDayOfWeek(lastDayFromLastMonth, lastMonth, year - (lastMonthIndex == 11));

            while (weekDay != 6)
            {
                createTextBox(Cell(bodyLayout, weekDay, 1), rgb(15, 15, 15), DayToText[lastDayFromLastMonth], TFT_BLACK, 2, 1);
                lastDayFromLastMonth--;
                weekDay = ttgo->rtc->getDayOfWeek(lastDayFromLastMonth, lastMonth, year - (lastMonthIndex == 11));
            }
        }
        {
            for (int i = 1; i < DaysInMonth[month - 1] + 1; i++)
            {
                int weekDay = ttgo->rtc->getDayOfWeek(i, month, year);

                int color = weekDay > 4 ? rgb(25, 25, 25) : TFT_WHITE;

                if (i == day && month == currentMonth && year == currentYear)
                {
                    color = rgb(25,25,32);
                }
                createTextBox(Cell(bodyLayout, weekDay, week), color, DayToText[i], TFT_BLACK, 2, 1);

                if (weekDay == 6)
                    week++;
            }
        }

        {

            Serial.printf("el valor de week es: %d \n", week);

            for (int i = 1; week < 7; i++)
            {
                //Serial.println("entre en el loop");

                //Serial.println(i);

                int weekDay;

                if (month == 12)
                {
                    weekDay = ttgo->rtc->getDayOfWeek(i, 1, year + 1);
                }
                else
                {
                    weekDay = ttgo->rtc->getDayOfWeek(i, month + 1, year);
                }

                //Serial.printf("el dia de la semana es: %d \n", weekDay);

                createTextBox(Cell(bodyLayout, weekDay, week), rgb(15, 15, 15), DayToText[i], TFT_BLACK, 2, 1);
                if (weekDay == 6)
                {
                    //Serial.println("paso a la siguiente semana");
                    week++;
                }
                //Serial.println("termina el bucle 1");
            }

            Serial.println("done drawing");
        }
    }
}

#endif
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"

#ifndef appCalculator
#define appCalculator

static const char *errors[] = {
    "no error",
    "parentheses don't match",
    "invalid character",
    "division by zero"};

// (c) Peter Kankowski, 2007. http://smallcode.weblogs.us mailto:kankowski@narod.ru
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ================================
//   Simple expression evaluator
// ================================

double Gloval_X_Value = 0;

// Error codes
enum EXPR_EVAL_ERR
{
    EEE_NO_ERROR = 0,
    EEE_PARENTHESIS = 1,
    EEE_WRONG_CHAR = 2,
    EEE_DIVIDE_BY_ZERO = 3
};

typedef char EVAL_CHAR;

class ExprEval
{
private:
    EXPR_EVAL_ERR _err;
    EVAL_CHAR *_err_pos;
    int _paren_count;

    // Parse a number or an expression in parenthesis
    double ParseAtom(EVAL_CHAR *&expr)
    {
        // Skip spaces
        while (*expr == ' ')
            expr++;

        // Handle the sign before parenthesis (or before number)
        bool negative = false;
        if (*expr == '-')
        {
            negative = true;
            expr++;
        }
        if (*expr == '+')
        {
            expr++;
        }

        // Check if there is parenthesis
        if (*expr == '(')
        {
            expr++;
            _paren_count++;
            double res = ParseSummands(expr);
            if (*expr != ')')
            {
                // Unmatched opening parenthesis
                _err = EEE_PARENTHESIS;
                _err_pos = expr;
                return 0;
            }
            expr++;
            _paren_count--;
            return negative ? -res : res;
        }

        // It should be a number or an x; convert it to double

        if (*expr == 'x')
        {
            expr++;
            return negative ? -Gloval_X_Value : Gloval_X_Value;
        }

        char *end_ptr;
        double res = strtod(expr, &end_ptr);
        if (end_ptr == expr)
        {
            // Report error
            _err = EEE_WRONG_CHAR;
            _err_pos = expr;
            return 0;
        }
        // Advance the pointer and return the result
        expr = end_ptr;
        return negative ? -res : res;
    }

    // Parse multiplication and division
    double ParseFactors(EVAL_CHAR *&expr)
    {
        double num1 = ParseAtom(expr);
        for (;;)
        {
            // Skip spaces
            while (*expr == ' ')
                expr++;
            // Save the operation and position
            EVAL_CHAR op = *expr;
            EVAL_CHAR *pos = expr;
            if (op != '/' && op != '*')
                return num1;
            expr++;
            double num2 = ParseAtom(expr);
            // Perform the saved operation
            if (op == '/')
            {
                // Handle division by zero
                if (num2 == 0)
                {
                    _err = EEE_DIVIDE_BY_ZERO;
                    _err_pos = pos;
                    return 0;
                }
                num1 /= num2;
            }
            else
                num1 *= num2;
        }
    }

    // Parse addition and subtraction
    double ParseSummands(EVAL_CHAR *&expr)
    {
        double num1 = ParseFactors(expr);
        for (;;)
        {
            // Skip spaces
            while (*expr == ' ')
                expr++;
            EVAL_CHAR op = *expr;
            if (op != '-' && op != '+')
                return num1;
            expr++;
            double num2 = ParseFactors(expr);
            if (op == '-')
                num1 -= num2;
            else
                num1 += num2;
        }
    }

public:
    double Eval(EVAL_CHAR *expr)
    {
        _paren_count = 0;
        _err = EEE_NO_ERROR;
        double res = ParseSummands(expr);
        // Now, expr should point to '\0', and _paren_count should be zero
        if (_paren_count != 0 || *expr == ')')
        {
            _err = EEE_PARENTHESIS;
            _err_pos = expr;
            return 0;
        }
        if (*expr != '\0')
        {
            _err = EEE_WRONG_CHAR;
            _err_pos = expr;
            return 0;
        }
        return res;
    };
    EXPR_EVAL_ERR GetErr()
    {
        return _err;
    }
    EVAL_CHAR *GetErrPos()
    {
        return _err_pos;
    }
};

// expresion eval end

const int MaxInputStringSize = 300;

char buff[MaxInputStringSize] = "";

const int plotXres = 100;


double MinX, MinY, MaxX, MaxY;

void plot(double MinX_, double MinY_, double MaxX_, double MaxY_)
{

    MinX = MinX_;
    MinY = MinY_;
    MaxX = MaxX_;
    MaxY = MaxY_;
    ttgo->tft->fillScreen(TFT_WHITE);
    int zoomX = MaxX - MinX;
    int zoomY = MaxY - MinY;
    clearButtons();

    createInterationArea(
        FULL_SCREEN_BOX, onUp, [](int x, int y)
        {
            const int displacement = 1;

            bool exit = true;

            if (x > w * 2 / 3) // derecha
            {
                plot(MinX + displacement, MinY, MaxX + displacement, MaxY);
                exit = false;
            }
            if (x < w / 3) // izq
            {
                plot(MinX - displacement, MinY, MaxX - displacement, MaxY);
                exit = false;
            }
            if (y > h * 2 / 3) // abajo
            {
                plot(MinX, MinY + displacement, MaxX, MaxY + displacement);
                exit = false;
            }
            if (y < h / 3) // arriba
            {
                plot(MinX, MinY - displacement, MaxX, MaxY - displacement);
                exit = false;
            }

            if (exit)
            {
                invalidate = true;
            }
        });

    // draw lines
    // vertical

    const int firstLineX = floor(MinX);
    const int lastLineX = ceil(MaxX);
    for (int i = firstLineX; i < lastLineX; i++)
    {
        int lineX = map(MinX, MaxX, 0, w, i);
        int col = i == 0 ? 0 : createRGB(220, 220, 220);
        ttgo->tft->drawFastVLine(lineX, 0, h, col);
    }

    // horizontal

    const int firstLineY = floor(MinY);
    const int lastLineY = ceil(MaxY);
    for (int i = firstLineY; i < lastLineY; i++)
    {
        int lineY = map(MinY, MaxY, 0, w, i);
        int col = i == 0 ? 0 : createRGB(220, 220, 220);
        ttgo->tft->drawFastHLine(0, lineY, w, col);
    }

    // numba

    // draw curve

    int lastDrawnX = -100;
    int lastDrawnY = -100;

    for (int i = 0; i < plotXres + 1; i++)
    {
        double currentX = double(i) / plotXres * zoomX + MinX;
        Gloval_X_Value = currentX;
        //char substitution[MaxInputStringSize];
        //const int NumberLenght = 20;
        //char Xvalue[NumberLenght];
        //snprintf(Xvalue, NumberLenght - 1, "(%.4f)", currentX);
        //int valueSize = strlen(Xvalue);
        //strcpy(substitution, buff);
        // substitute 'x' by valueSize
        //Serial.printf("the string of the number is:\"%s\" and has size %d \n", Xvalue, valueSize);
        // for (int i = 0; i < strlen(substitution); i++)
        // {
        //     if (substitution[i] == 'x')
        //     {
        //         Serial.printf("found an x at pos: %d \n", i);
        //         //shift right from the end of the array to the position of the 'x'
        //         for (int j = strlen(substitution) + 1; j > i; j--)
        //         {
        //             substitution[j + valueSize] = substitution[j];
        //         }
        //         Serial.printf("after shifting: \"%s\"\n", substitution);
        //         for (int j = 0; j < valueSize; j++)
        //         {
        //             substitution[j + i] = Xvalue[j];
        //         }
        //         Serial.printf("after substitution: \"%s\"\n", substitution);
        //     }
        // }

        ExprEval eval;
        double res = eval.Eval(buff);

        if (eval.GetErr() != EEE_NO_ERROR)
        {
            Serial.printf("  Error: %s at %s\n", errors[eval.GetErr()], eval.GetErrPos());
        }
        else
        {
            int currentDrawingX = map(MinX, MaxX, 0, w, currentX);
            int currentDrawingY = map(MinY, MaxY, 0, h, -res);

            ttgo->tft->drawLine(lastDrawnX, lastDrawnY, currentDrawingX, currentDrawingY, createRGB(40, 0, 0));
            ttgo->tft->drawLine(lastDrawnX, lastDrawnY + 1, currentDrawingX, currentDrawingY + 1, createRGB(40, 0, 0));
            ttgo->tft->drawLine(lastDrawnX, lastDrawnY - 1, currentDrawingX, currentDrawingY - 1, createRGB(40, 0, 0));
            lastDrawnX = currentDrawingX;
            lastDrawnY = currentDrawingY;
        }
    }
}

const int buttonMidleSeparation = 140;

void CalculatorTick()
{

    const tGrid MainGrid = createGrid(FULL_SCREEN_BOX, 10, 10, 2, 4);

    const tBox textDisplay = boxMerge(Cell(MainGrid, 0, 0), Cell(MainGrid, 1, 0));
    const tBox equalsBox = Cell(MainGrid, 0, 2);    //{10, buttonMidleSeparation + 10, w / 2 - 5, h - 10};
    const tBox CEBox = Cell(MainGrid, 1, 1);        //{w / 2 + 5, 70, w - 10, buttonMidleSeparation};
    const tBox BackSpaceBox = Cell(MainGrid, 0, 1); //{10, 70, w / 2 - 5, buttonMidleSeparation};
    const tBox graphingBox = Cell(MainGrid, 1, 2);  //{w / 2 + 5, buttonMidleSeparation + 10, w - 10, h - 10};
    const tBox exitBox = boxMerge(Cell(MainGrid, 0, 3), Cell(MainGrid, 1, 3));

    if (!drawn)
    {
        ttgo->tft->fillRect(10, 10, 40, 40, TFT_WHITE);
        createButton(
            exitBox, onUp, [](int x, int y)
            { goToLauncher(); },
            RED_CANCEL, "Exit", TFT_BLACK);
        createButton(
            textDisplay, onUp, [](int x, int y)
            {
                OCRstart(buff, MaxInputStringSize);
                drawn = false;
            },
            TFT_WHITE, buff, TFT_BLACK);

        createButton(
            equalsBox, onUp, [](int x, int y)
            {
                Serial.printf("evaluating");
                ExprEval eval;
                double res = eval.Eval(buff);
                if (eval.GetErr() != EEE_NO_ERROR)
                {
                    Serial.printf("  Error: %s at %s\n", errors[eval.GetErr()], eval.GetErrPos());
                }
                else
                {
                    sprintf(buff, "%g", res);
                }
                drawn = false;
            },
            TFT_CYAN, "=", TFT_BLACK);

        createButton(
            CEBox, onUp, [](int x, int y)
            {
                buff[0] = '\0';
                invalidate = true;
            },
            RED_CANCEL, "CE", TFT_BLACK);

        createButton(
            BackSpaceBox, onUp, [](int x, int y)
            {
                buff[strlen(buff) - 1] = '\0';
                invalidate = true;
            },
            TFT_ORANGE, "back", TFT_BLACK);

        createButton(
            graphingBox, onUp, [](int x, int y)
            { plot(-6, -6, 6, 6); },
            TFT_GREEN, "plot", TFT_BLACK);

        //Serial.println("created button maybe probably");
    }

    //Serial.printf("calculator tick %d \n", drawn);
}

// double ParseAtom(char *&expr)
// {
//     // Read the number from string
//     char *end_ptr;
//     double res = strtod(expr, &end_ptr);
//     // Advance the pointer and return the result
//     expr = end_ptr;
//     return res;
// }

#endif
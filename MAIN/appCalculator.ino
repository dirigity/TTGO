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

        // It should be a number; convert it to double
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

const int plotXres = 50;

double map(double StartRangeSrc, double EndRangeSrc, double StartRangeDst, double EndRangeDst, double val)
{
    return StartRangeDst + ((EndRangeDst - StartRangeDst) / (EndRangeSrc - StartRangeSrc)) * (val - StartRangeSrc);
}

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

            if (x > w * 4 / 5)
            {
                plot(MinX + displacement, MinY, MaxX + displacement, MaxY);
                exit = false;
            }
            if (x < w / 5)
            {
                plot(MinX - displacement, MinY, MaxX - displacement, MaxY);
                exit = false;
            }
            if (y > h * 4 / 5)
            {
                plot(MinX, MinY - displacement, MaxX, MaxY - displacement);
                exit = false;
            }
            if (y < h / 5)
            {
                plot(MinX, MinY + displacement, MaxX, MaxY + displacement);
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
        int linePos = map(MinX, MaxX, 0, w, i);
        int col = i == 0 ? 0 : createRGB(220, 220, 220);
        ttgo->tft->drawFastHLine(0, linePos, w, col);
    }

    const int firstLineY = floor(MinY);
    const int lastLineY = ceil(MaxY);
    for (int i = firstLineY; i < lastLineY; i++)
    {
        int linePos = map(MinY, MaxY, 0, w, i);
        int col = i == 0 ? 0 : createRGB(220, 220, 220);
        ttgo->tft->drawFastVLine(linePos, 0, h, col);
    }

    // draw curve

    int lastDrawnX = -100;
    int lastDrawnY = -100;

    for (int i = 0; i < plotXres; i++)
    {
        char substitution[MaxInputStringSize];
        const int NumberLenght = 20;
        char Xvalue[NumberLenght];
        double currentX = double(i) / plotXres * zoomX + MinX;
        snprintf(Xvalue, NumberLenght - 1, "%.4f", currentX);
        int valueSize = strlen(Xvalue);
        strcpy(substitution, buff);

        // substitute 'x' by valueSize

        for (int i = 0; i < strlen(substitution); i++)
        {
            if (substitution[i] == 'x')
            {

                // shift right from the end of the array to the position of the 'x'

                for (int j = strlen(substitution); j > i; j--)
                {
                    substitution[j + valueSize] = substitution[j];
                }

                for (int j = 0; j < valueSize; j++)
                {
                    substitution[j + i] = Xvalue[i];
                }
            }
        }

        ExprEval eval;
        double res = eval.Eval(buff);
        double Yval = 0;

        if (eval.GetErr() != EEE_NO_ERROR)
        {
            Serial.printf("  Error: %s at %s\n", errors[eval.GetErr()], eval.GetErrPos());
        }
        else
        {
            int currentDrawingX = map(MinX, MaxX, 0, w, currentX);
            int currentDrawingY = map(MinY, MaxY, h, 0, res);

            ttgo->tft->drawLine(lastDrawnX, lastDrawnY, currentDrawingX, currentDrawingY, createRGB(40, 0, 0));

            lastDrawnX = currentDrawingX;
            lastDrawnY = currentDrawingY;
        }
    }
}

const int buttonMidleSeparation = 140;

void CalculatorTick()
{
    const tBox textDisplay = {10, 10, w - 10, 60};
    const tBox equalsBox = {10, buttonMidleSeparation + 10, w / 2 - 5, h - 10};
    const tBox CEBox = {w / 2 + 5, 70, w - 10, buttonMidleSeparation};
    const tBox BackSpaceBox = {10, 70, w / 2 - 5, buttonMidleSeparation};
    const tBox graphingBox = {w / 2 + 5, buttonMidleSeparation + 10, w - 10, h - 10};

    if (!drawn)
    {
        ttgo->tft->fillRect(10, 10, 40, 40, TFT_WHITE);
        createButton(
            textDisplay, onUp, [](int x, int y)
            {
                bool stop = false;
                while (!stop)
                {
                    char next = CallOCR();
                    if (next == '_')
                    {
                        stop = true;
                    }
                    else if (next == '?')
                    {
                        ttgo->motor->onec(10);
                    }
                    else
                    {
                        int CeroIdx = strlen(buff);
                        if (CeroIdx != MaxInputStringSize - 1)
                        {
                            buff[CeroIdx] = next;
                            buff[CeroIdx + 1] = '\0';
                        }
                        else
                        {
                            buff[MaxInputStringSize - 1] = '\0';
                        }

                        Serial.println(buff);
                    }
                }
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

double ParseAtom(char *&expr)
{
    // Read the number from string
    char *end_ptr;
    double res = strtod(expr, &end_ptr);
    // Advance the pointer and return the result
    expr = end_ptr;
    return res;
}

#endif
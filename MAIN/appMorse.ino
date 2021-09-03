#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\appOcr.ino"
//#include <string.h>

#ifndef appMorse
#define appMorse

char text[100] = "";

struct tMorseConversion
{
    char c;
    char *translation;
};

const int timeUd = 300;

const int short_ = 1;
const int long_ = 3;
const int silenceBetweenBits = 1;
const int silenceBetweenLetters = 3;
const int silenceBetweenWords = 7;
tMorseConversion MorseTranslation[36] = {
    {'a', ".-"},
    {'b', "-..."},
    {'c', "-.-."},
    {'d', "-.."},
    {'e', "."},
    {'f', ".._."},
    {'g', "--."},
    {'h', "...."},
    {'i', ".."},
    {'j', ".---"},
    {'k', "-.-"},
    {'l', ".-.."},
    {'m', "--"},
    {'n', "-."},
    {'o', "---"},
    {'p', ".--."},
    {'q', "--.-"},
    {'r', ".-."},
    {'s', "..."},
    {'t', "-"},
    {'u', "..-"},
    {'v', "...-"},
    {'w', ".--"},
    {'x', "-..-"},
    {'y', "-.--"},
    {'z', "--.."},
    {'0', "-----"},
    {'1', ".----"},
    {'2', "..---"},
    {'3', "...--"},
    {'4', "....-"},
    {'5', "....."},
    {'6', "-...."},
    {'7', "--..."},
    {'8', "---.."},
    {'9', "----."},

};

void write(char c, char *str, int loops)
{
    if (loops == 0)
    {
        return;
    }

    int strLen = strlen(str);

    str[strLen] = c;
    str[strLen + 1] = '\0';

    Serial.printf("morse construction: \"%s\", current char %c %d times\n", str, c, loops);

    write(c, str, loops - 1);
}

void MorsePlay(char *text)
{
    ttgo->bl->adjust(255);
    if (*text == '\0')
    {
        return;
    }

    bool abort = false;
    bool end = false;

    char translation[200] = " ";

    //Serial.printf("translating: %s", text);

    while (*text != '\0')
    {
        if (*text == ' ' || *text == '_')
        {
            // Serial.println("espacio entre palabras");
            write(' ', translation, silenceBetweenWords - silenceBetweenLetters + silenceBetweenBits);
        }
        else
        {
            //Serial.printf("buscando letra %c \n", *text);

            for (int i = 0; i < 36; i++)
            {
                if (toLowerChar(*text) == MorseTranslation[i].c)
                {
                    //Serial.printf("encontrada letra %c con codigo %s \n", MorseTranslation[i].c, MorseTranslation[1].translation);

                    char *seq = MorseTranslation[i].translation;

                    while (*seq != '\0')
                    {
                        if (*seq == '.')
                        {
                            //Serial.println("short");

                            write('-', translation, short_);
                        }
                        else
                        {
                            //Serial.println("largo");

                            write('-', translation, long_);
                        }

                        //Serial.println("espacio entre letras");
                        write(' ', translation, silenceBetweenBits);

                        seq++;
                    }
                    i = 100; // break;
                }
            }
        }
        //Serial.println("fin de letra");

        write(' ', translation, silenceBetweenLetters - silenceBetweenBits);

        text++;
    }
    Serial.println("fin de traducion");

    int startTime = millis();

    Serial.println(translation);

    while (!abort)
    {

        int i = (millis() - startTime) / timeUd;
        bool currentVal = translation[i] == '-';

        ttgo->bl->adjust(currentVal ? 255 : 0);
        ttgo->tft->fillScreen(currentVal ? TFT_WHITE : TFT_BLACK);

        short int x, y;
        abort = ttgo->getTouch(x, y) || translation[i] == '\0';
    }

    ttgo->bl->adjust(permanent.brightness);
    invalidate = true;
}

bool blink = false;
bool deco = false;

const int MaxDecodeOutSize = 15;
char DecoDisplay[MaxDecodeOutSize] = "";
char DecoInput[MaxDecodeOutSize] = "";

void MorseTick()
{
    if (!drawn)
    {
        if (deco)
        {
            const tGrid layout = createGrid(FULL_SCREEN_BOX, 10, 10, 3, 4);
            const tBox DisplayBox = boxMerge(Cell(layout, 0, 0), Cell(layout, 1, 0));
            const tBox EraseDisplayBox = Cell(layout, 2, 0);
            const tBox InputBox = boxMerge(Cell(layout, 0, 1), Cell(layout, 1, 1));
            const tBox BackSpaceInputBox = Cell(layout, 2, 1);
            const tBox LongBox = Cell(layout, 0, 2);
            const tBox ShortBox = Cell(layout, 1, 2);
            const tBox SpaceBox = Cell(layout, 0, 3);
            const tBox DoneBox = boxMerge(Cell(layout, 1, 3), Cell(layout, 2, 3));

            const tBox Validate = Cell(layout, 2, 2);

            createButton(
                DisplayBox, onUp, [](int x, int y) {},
                TFT_WHITE, DecoDisplay, TFT_BLACK);
            createButton(
                InputBox, onUp, [](int x, int y) {},
                TFT_WHITE, DecoInput, TFT_BLACK);

            createButton(
                EraseDisplayBox, onUp, [](int x, int y)
                {
                    *DecoDisplay = '\0';
                    drawn = false;
                },
                RED_CANCEL, "CE", TFT_BLACK);
            createButton(
                BackSpaceInputBox, onUp, [](int x, int y)
                {
                    DecoInput[max(0, int(strlen(DecoInput) - 1))] = '\0';
                    drawn = false;
                },
                RED_CANCEL, "Back", TFT_BLACK);
            createButton(
                LongBox, onUp, [](int x, int y)
                {
                    int len = strlen(DecoInput);
                    if (len < MaxDecodeOutSize)
                    {
                        DecoInput[len] = '_';
                        DecoInput[len + 1] = '\0';
                        drawn = false;
                    }
                },
                createRGB(40, 40, 240), "L", TFT_BLACK);
            createButton(
                ShortBox, onUp, [](int x, int y)
                {
                    int len = strlen(DecoInput);
                    if (len < MaxDecodeOutSize)
                    {
                        DecoInput[len] = '.';
                        DecoInput[len + 1] = '\0';
                        drawn = false;
                    }
                },
                createRGB(40, 40, 240), "S", TFT_BLACK);
            createButton(
                SpaceBox, onUp, [](int x, int y)
                {
                    int len = strlen(DecoDisplay);
                    if (len < MaxDecodeOutSize)
                    {
                        DecoDisplay[len] = ' ';
                        DecoDisplay[len + 1] = '\0';
                        drawn = false;
                    }
                },
                createRGB(40, 40, 240), "_", TFT_BLACK);
            createButton(
                DoneBox, onUp, [](int x, int y)
                {
                    drawn = false;
                    blink = false;
                },
                createRGB(40, 40, 240), "Done", TFT_BLACK);

            createButton(
                Validate, onUp, [](int x, int y)
                {
                    for (int i = 0; i < 36; i++)
                    {
                        if (0 == strcmp(MorseTranslation[i].translation, DecoInput))
                        {
                            int len = strlen(DecoDisplay);
                            if (len < MaxDecodeOutSize)
                            {
                                DecoDisplay[len] = MorseTranslation[i].c;
                                DecoDisplay[len + 1] = '\0';
                                drawn = false;
                            }
                        }
                        i = 100;
                    }
                    *DecoInput = '\0';
                },
                TFT_GREEN, "read", TFT_BLACK);
        }
        else if (blink)
        {
            createInterationArea(
                FULL_SCREEN_BOX, onUp, [](int x, int y)
                {
                    blink = false;
                    invalidate = true;
                    ttgo->bl->adjust(permanent.brightness);
                });
        }
        else
        {
            ttgo->tft->fillScreen(0);

            const tGrid layout = createGrid(FULL_SCREEN_BOX, 10, 10, 2, 4);
            const tBox TextInputBox = boxMerge(Cell(layout, 1, 0), Cell(layout, 0, 0));
            const tBox SOSBox = Cell(layout, 0, 2);
            const tBox PlayBox = Cell(layout, 1, 2);
            const tBox DecoBox = Cell(layout, 0, 3);
            const tBox BlinkBox = Cell(layout, 1, 3);
            const tBox BackSpaceBox = Cell(layout, 0, 1);
            const tBox CEBox = Cell(layout, 1, 1);

            createButton(
                CEBox, onUp, [](int x, int y)
                {
                    text[0] = '\0';
                    invalidate = true;
                },
                RED_CANCEL, "CE", TFT_BLACK);

            createButton(
                BackSpaceBox, onUp, [](int x, int y)
                {
                    text[strlen(text) - 1] = '\0';
                    invalidate = true;
                },
                TFT_ORANGE, "back", TFT_BLACK);

            createButton(
                TextInputBox, onUp, [](int x, int y)
                {
                    OCRstart(text, 99);
                    drawn = false;
                },
                TFT_WHITE, text, TFT_BLACK);
            createButton(
                SOSBox, onUp, [](int x, int y)
                { MorsePlay("SOS SOS SOS SOS"); },
                createRGB(40, 40, 240), "SOS", TFT_BLACK);
            createButton(
                PlayBox, onUp, [](int x, int y)
                { MorsePlay(text); },
                createRGB(40, 40, 240), "Play", TFT_BLACK);
            createButton(
                BlinkBox, onUp, [](int x, int y)
                {
                    blink = true;
                    drawn = false;
                },
                createRGB(40, 40, 240), "blink", TFT_BLACK);
            createButton(
                DecoBox, onUp, [](int x, int y)
                {
                    deco = true;
                    drawn = false;
                },
                createRGB(40, 40, 240), "Decode", TFT_BLACK);
        }
    }

    if (blink)
    {
        const int period = 500; // in ms
        ttgo->bl->adjust(millis() % period > period / 2 ? 0 : 255);
        ttgo->tft->fillScreen(millis() % period > period / 2 ? TFT_BLACK : TFT_WHITE);
    }
}

#endif
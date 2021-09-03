#include "C:\Users\Jaime\Desktop\TTGO\MAIN\ttgoGlovalDeclarations.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\utils.ino"
#include "C:\Users\Jaime\Desktop\TTGO\MAIN\buttonSistem.ino"

#ifndef appOCR
#define appOCR

tApp returnApp;

struct tuple
{
    char equivalence;
    long long int seed;
};

struct tPos
{
    int x;
    int y;
};

tPos nodes[8] = {
    {0, 0},
    {2, 0},
    {1, 1},
    {0, 2},
    {2, 2},
    {1, 3},
    {0, 4},
    {2, 4},
};

const int MaxLinks = 15;
struct tLinkList
{
    int list[MaxLinks];
    int counter = 0;
};

tLinkList linkList;

int indexCombinations[28][2] = {{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {4, 5}, {4, 6}, {4, 7}, {5, 6}, {5, 7}, {6, 7}};

const int dictLen = 74;

tuple dict[dictLen] = {
    {' ', 134217728},
    {'1', 16777728},
    {'2', 135528961},
    {'3', 151257601},
    {'4', 17039876},
    {'4', 17047680},
    {'4', 17048192},
    {'5', 151257093},
    {'6', 152305669},
    {'7', 16777729},
    {'8', 152306181},
    {'9', 17039877},
    {'9', 151257605},
    {'0', 152044037},
    {'+', 294912},
    {'-', 262144},
    {'(', 67141760},
    {')', 33587202},
    {'/', 2048},
    {'/', 33587328},
    {'/', 65664},
    {'/', 33555456},
    {'^', 24576},
    {'*', 4743168},
    {'=', 134479872},
    {'.', 33554432},
    {'a', 18088453},
    {'b', 202907781},
    {'b', 152305668},
    {'c', 135528448},
    {'c', 134217761},
    {'d', 152306176},
    {'e', 134480417},
    {'f', 1310725},
    {'g', 84148741},
    {'g', 155189281},
    {'g', 152043525},
    {'h', 18088452},
    {'h', 18087940},
    {'i', 16777216},
    {'i', 1048576},
    {'j', 152044032},
    {'j', 83886592},
    {'k', 68690052},
    {'l', 135266308},
    {'m', 17826438},
    {'m', 22544384},
    {'n', 84967942},
    {'n', 85458944},
    {'o', 152305664},
    {'p', 1311237},
    {'q', 84148741},
    {'r', 68944389},
    {'r', 1310720},
    {'s', 184811653},
    {'s', 201875456},
    {'t', 32769},
    {'t', 1048581},
    {'u', 152043520},
    {'u', 152044036},
    {'v', 4719108},
    {'v', 38797312},
    {'v', 38797828},
    {'v', 84410368},
    {'v', 84410884},
    {'w', 118489604},
    {'w', 118489088},
    {'x', 105381888},
    {'y', 16794114},
    {'y', 37765634},
    {'y', 32898},
    {'y', 1056900},
    {'y', 67641476},
    {'z', 172228608},
};

const int OCRsize = 50;
const int OCRLeftOfset = 40; // espacio pa poner bottones
const int OCRmargin = 10;
const int OCRbuttSize = 50;

char CallOCR()
{

    bool PrePressedDone = false;
    bool PrePressedCancel = false;
    const tBox DoneBox = {w - OCRmargin - OCRbuttSize, OCRmargin, w - OCRmargin, h / 2 - OCRmargin / 2};
    const tBox CancellBox = {w - OCRmargin - OCRbuttSize, h / 2 + OCRmargin / 2, w - OCRmargin, h - OCRmargin};

    drawn = false;
    linkList.counter = 0;
    clearButtons();

    bool captureFocus = true;
    int lastTouchedNode = -1;

    while (captureFocus)
    {
        if (!drawn)
        {
            ttgo->tft->fillScreen(0);

            for (int i = 0; i < 8; i++)
            {
                tPos node = nodes[i];
                int x = w / 2 - OCRsize + node.x * OCRsize - OCRLeftOfset;
                int y = h / 2 - OCRsize * 2 + node.y * OCRsize;
                if (i == lastTouchedNode)
                    ttgo->tft->fillCircle(x, y, 10, TFT_WHITE);
                else
                    ttgo->tft->fillCircle(x, y, 10, createRGB(200, 200, 200));
            }
            for (int i = 0; i < linkList.counter; i++)
            {
                int *nodeCombination = indexCombinations[linkList.list[i]];

                tPos node1 = nodes[nodeCombination[0]];
                int x1 = w / 2 - OCRsize + node1.x * OCRsize - OCRLeftOfset;
                int y1 = h / 2 - OCRsize * 2 + node1.y * OCRsize;

                tPos node2 = nodes[nodeCombination[1]];
                int x2 = w / 2 - OCRsize + node2.x * OCRsize - OCRLeftOfset;
                int y2 = h / 2 - OCRsize * 2 + node2.y * OCRsize;

                ttgo->tft->drawLine(x1, y1, x2, y2, TFT_WHITE);
            }

            drawBox(DoneBox, TFT_GREEN);
            drawBox(CancellBox, TFT_RED);

            drawn = true;
        }

        int16_t touchX, touchY;
        bool touching = ttgo->getTouch(touchX, touchY);
        if (touching)
        {
            int currentTouchingNode = -1;

            for (int i = 0; i < 8; i++)
            {
                tPos checkingNode = nodes[i];

                int x = w / 2 - OCRsize + checkingNode.x * OCRsize - OCRLeftOfset;
                int y = h / 2 - OCRsize * 2 + checkingNode.y * OCRsize;

                if (d(touchX, touchY, x, y) < 20)
                {
                    currentTouchingNode = i;
                    i = 100; // break
                }
            }

            //Serial.printf("currentTouchingNode: %d, lastTouchedNode: %d \n", currentTouchingNode, lastTouchedNode);

            if (currentTouchingNode != -1 && lastTouchedNode != -1 && currentTouchingNode != lastTouchedNode)
            {
                Serial.printf("creating link from %d to %d \n", lastTouchedNode, currentTouchingNode);
                int linkID = -1;

                for (int i = 0; i < 28; i++)
                {
                    int min_ = min(currentTouchingNode, lastTouchedNode);
                    int max_ = max(currentTouchingNode, lastTouchedNode);
                    if (min_ == indexCombinations[i][0] && max_ == indexCombinations[i][1])
                    {
                        linkID = i;
                        i = 100; // break
                    }
                }

                if (linkList.counter < MaxLinks)
                {
                    bool found = false;
                    for (int i = 0; i < linkList.counter; i++)
                    {
                        if (linkID == linkList.list[i])
                        {
                            found = true;
                        }
                    }
                    if (!found)
                    {
                        linkList.list[linkList.counter] = linkID;
                        linkList.counter++;
                        drawn = false;
                        ttgo->motor->onec(10);
                    }
                }
            }

            if (currentTouchingNode != -1)
                lastTouchedNode = currentTouchingNode;
        }
        else
        {
            lastTouchedNode = -1;
        }

        {

            if (insideBox(touchX, touchY, CancellBox))
            {
                PrePressedCancel = true;
                //Serial.println("clickDown");
            }
            else
            {
                //Serial.printf("PrePressedCancel: %d \n", PrePressedCancel);
                if (PrePressedCancel)
                {
                    Serial.println("pressed cancel");
                    if (linkList.counter == 0)
                    {
                        return '_';
                        Serial.println("cancel char input");
                    }
                    Serial.println("reset char input");
                    linkList.counter = 0;
                    drawn = false;
                }
                PrePressedCancel = false;
            }
            if (insideBox(touchX, touchY, DoneBox))
            {
                //Serial.println("clickDown");
                PrePressedDone = true;
            }
            else
            {
                //Serial.printf("PrePressedDone: %d \n", PrePressedDone);
                if (PrePressedDone)
                {
                    Serial.println("presed submit");
                    captureFocus = false;
                    int linkListCode = 0;
                    for (int i = 0; i < linkList.counter; i++)
                    {
                        linkListCode += pow(2, linkList.list[i]);
                    }
                    Serial.printf("linkListCode: %d \n", linkListCode);

                    for (int i = 0; i < dictLen; i++)
                    {
                        if (dict[i].seed == linkListCode)
                        {
                            Serial.printf("recognized: %c \n", dict[i].equivalence);
                            return dict[i].equivalence;
                        }
                    }
                    Serial.println("not recognized");
                }
                PrePressedDone = false;
            }
        }
    }

    invalidate = true;
    return '?';
}

void OCRstart(char *buff, int MaxInputStringSize)
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
}
#endif
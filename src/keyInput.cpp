#include <Arduino.h>
#include "keyInput.h"
#include "supportFun.h"
#include "global.h"

// Global variables
extern bool cecRxFlag;
extern bool cecTxFlag;
extern bool stopKeyHDMI;
extern long keyPrevCheckTime;

// Local Variables
static uint8_t debounce = 1;
static uint8_t butNum = 0;
static uint8_t butNumPrev = 0;

// Prototypes
void doKeyTask();

// Implementation

void doKeyTask()
{
    // if (!cecTxFlag && !stopKeyHDMI)
    // {
    // generateDebugPulse(10);
    // Serial.printf("Iam in doKeytask and cecTxFlag is %d and stopKeyHDMI flog is %d \n", cecTxFlag, stopKeyHDMI);
    butNum = 0;
    if (digitalRead(PIN_KEY_POWER) == HIGH)
    {
        butNum = butNum | 0x01;
    }
    if (digitalRead(PIN_KEY_VOLPLUS) == HIGH)
    {
        butNum = butNum | 0x02;
    }
    if (digitalRead(PIN_KEY_VOLMINUS) == HIGH)
    {
        butNum = butNum | 0x04;
    }
    if (butNum != 7)
    {
        if (butNumPrev == butNum)
        {
            debounce++;
            if (debounce > 5)
            {
                uint8_t butCode[7] = {0, 0, 0, KEYCODE_VOLUME_MINUS, 0, KEYCODE_VOLUME_PLUS, KEYCODE_STANDBY};
                Serial.printf("BUTTON %d Pressed\n", butCode[butNum]);
                processFunction(butCode[butNum]);
                debounce = 0;
                butNumPrev = 0;
            }
        }
        else
        {
            debounce = 0;
            butNumPrev = butNum;
        }
    }
    // }
}

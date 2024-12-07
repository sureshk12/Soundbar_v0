// Key Board, IR Checking

#include <Arduino.h>
#include <Wire.h>

#include "btUsbFunction.h"
#include "cec.h"
#include "cecReceive.h"
#include "cecTransmit.h"
#include "global.h"
#include "irDecode.h"
#include "keyInput.h"
#include "sb_I2c.h"
#include "supportFun.h"


// Global Variables
bool isHotplug = false;
uint8_t sb_power = PWR_STDBY;
LastDataStruct lastValueStruct;
uint8_t lastStatusArray[NO_OF_LASTSTATUS] = {0, 0, 0, 0, 0};
bool cecRxFlag = false;
bool debugContinue = true;

// Main Program Variables
uint8_t cecMainState = 0;
uint8_t num = 0;
long keyPrevCheckTime = millis();
long btPrevCheckTime = millis();

// Prototype
void reportStatus();

void setup()
{
    //***** System boot, sets up all pins and Serial UART *****
    systemBoot();
    doSbPowerStdByAction(); // put SB into Stand By
}

void loop()
{
    // Debug to indicate Start loop:
    // generateDebugPulse(10);

    // Check for key board every 20mS
    int deltaIntervel = millis() - keyPrevCheckTime;
    if (deltaIntervel > 20)
    {
        doKeyTask();
        keyPrevCheckTime = millis();
    }

    // Check for Remote activity
    doIrDecode();

    // Check for HotPlug Status
    doHdmiHotPlugTask();

    if (sb_power == PWR_ON)
    {
        if (isHotplug)
        {
            cecRxFlag = true;
            // doArcTask();
            doCecTxTask();
            doCecReadTask();
            doAutoReply();
        }
        // Check BT Volume status in BT mode
        if ((lastStatusArray[0] == SOURCE_BT || lastStatusArray[0] == SOURCE_USB) && ((millis() - btPrevCheckTime) > 2000))
        {
            doCheckBtDeviceStatus();
            btPrevCheckTime = millis();
        }
    }
    reportStatus();
}

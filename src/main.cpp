// Key Board, IR Checking

#include <Arduino.h>
#include <Wire.h>

#include "supportFun.h"
#include "keyInput.h"
#include "irDecode.h"
#include "cec.h"
#include "cecReceive.h"
#include "cecTransmit.h"
#include "lastData.h"
#include "sb_I2c.h"
#include "btUsbFunction.h"
#include "global.h"
#include "audioDsp.h"

// Global Variables SB State
bool isTvCecOn = false;
bool isTvCecOnPrev = true;
bool isHotplug = false;
bool isHotplugPrev = true;
bool isTvOn = false;
bool sb_Mute = MUTE_ON;
uint8_t sb_Volume = 0x64;
uint8_t sb_power = PWR_STDBY;
uint8_t tv_Audio_Status_Volume_Mute = 0;
bool tv_Power_Status = false;
bool tvArc = false;
extern bool btConnected;
bool btEnabled = false;

bool resetDisplayToSource;
long prevResetDisplayToSource = millis();

// Global System variables
LastDataStruct lastValueStruct;
uint8_t lastStatusArray[NO_OF_LASTSTATUS] = {0, 0, 0, 0, 0};
bool cecRxFlag = false;
bool cecTxFlag = false;
bool cecTxFlagPrev = true;
bool startedToSendData = false;
bool cecDataSemafore = false;
bool cecTxDisableErrChk = true;
uint8_t cecTxErrorFlag = 0;
uint8_t cecTxErrorFlagPrev = 1;
bool cecTxReady = false;
uint8_t cecTxErrorCount = 0;
bool doNotAck = false;
//bool stopKeyHDMI = false;

bool pendingBtUsbKeyProcess = false;
uint8_t currIoData = 0xFF;

long keyPrevCheckTime = millis();
long btPrevCheckTime = millis();

// Global Debug
bool debugContinue = true;

// Main Program Variables
uint8_t cecMainState = 0;

uint8_t num = 0;

// Prototype
void reportStatus();

void setup()
{
    // /*
    //***** System boot, sets up all pins and Serial UART *****
    delay(100);
    systemBoot();
    printf("System Boot OK\n");

    // */

    // /*
    // ***** Start I2C *****
    if (!Wire.begin())
    {
        Serial.printf("ERROR: Could not Initialize I2C\n");
        stopHere();
    }
    else
    {
        Serial.printf("I2C OK\n");
        delay(10);
    }
    // */

    // /*
    // ***** Initilize OLED ****
    if (!intilizeOled())
    {
        Serial.printf("ERROR: Could not Initialize OLED\n");
        stopHere();
    }
    Serial.printf("OK:OLED\n");
    delay(10);
    // */

    byte error = 1;
    byte errorCount = 1;

    // /*
    // ***** Start PCM9211 *****
    error = 1;
    errorCount = 1;
    while (error != 0)
    {
        Wire.beginTransmission(PCM9211_I2C_ADDR);
        error = Wire.endTransmission();
        if (error == 0)
        {
            Serial.println("MUX Found :-)");
            if (!intilizePCM9211())
            {
                Serial.printf("ERROR: Could not Initialize PCM9211\n");
                stopHere();
            }
            Serial.printf("OK:PCM9211\n");
        }
        else
        {
            Serial.printf("%d : MUX NOT FOUND :-(\n", errorCount);
            delay(1000);
            errorCount++;
            if (errorCount > 5)
            {
                stopHere();
            }
        }
    }
    // */

    // /*
    //***** Initilaize BT USB*****
    // initilizeBtUsb_BK828881();
    // initilizeBtUsb_PCF8574();
    // */

    // /*
    // ***** Get last status Source, Volume, Mode ****
    getLastStatus();
    changeDspVolume(lastStatusArray[1]);
    doSourceselection(lastStatusArray[0]);
    // printLastStatus(); //[DEBUG]
    // */

    // ***** Reset Hotplug status to known status *****

    hotPlugReset();         // Hotplug Reset
    doSbPowerStdByAction(); // put SB into Stand By
    sb_power = PWR_STDBY;   // First time power up, put system to STDBY
    cecMainState = 0;       // Main ON set cecState to reset mode
    oledDisplayLogo();      // Display boot up logo
    getBtDevStatus();
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

    // Check tthis for noe commenting
    //  doDisplayResetToSource();

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

    /*
      //not sure what this below is doing
      if (cecMainState == 0)
      {
        Serial.println("Mains ON Suresh");
        doSbPowerStdByAction(); // put SB into Stand By
        cecMainState = 10;
      }
      */
}

void reportStatus()
{

    // static bool isHdmiHotplugStatusPrev = true;
    // static bool tv_Power_Status_Prev = true;
    // Serious errors will STOP

    // Non serious but limiting Functionality will be reported
    // Serial.printf("isCecBusOk %s\n", isCecBusOk ? "TRUE" : "FALSE");

    /*
    if (cecTxErrorFlagPrev != cecTxErrorFlag)
    {
        if (cecTxErrorFlag > 0)
        {
            Serial.printf("CEC Tx Error %d\n", cecTxErrorFlag);
        }
        else
        {
            Serial.printf("CEC Tx OK %d\n", cecTxErrorFlag);
        }
        cecTxErrorFlagPrev = cecTxErrorFlag;
    }

    if (isTvCecOnPrev != isTvCecOn)
    {
        if (!isTvCecOn)
        {
            Serial.println("TV NOT YET Communicating (with SB)");
        }
        else
        {
            Serial.println("TV  CEC ON");
        }
        isTvCecOnPrev = isTvCecOn;
    }

    if (isHotplugPrev != isHotplug)
    {
        if (!isHotplug)
        {
            Serial.println("HOT Plug is LOW");
        }
        else
        {
            Serial.println("HOT Plug is HIGH");
        }

        isHotplugPrev = isHotplug;
    }

    */
    // if(cecTxFlagPrev != cecTxFlag)
    // {
    //     if(!cecTxFlag)
    //     {
    //         Serial.printf("cecTxFlag is FALSE\n");
    //     } else 
    //     {
    //         Serial.printf("cecTxFlag is TRUE\n");
    //     }
    //     cecTxFlagPrev = cecTxFlag;
    // }

    // if (tv_Power_Status_Prev != tv_Power_Status)
    // {
    //     if (!tv_Power_Status)
    //     {
    //         Serial.println("TV in STAND BY");
    //     }
    //     else
    //     {
    //         Serial.println("TV in ON!!!");
    //     }

    //     tv_Power_Status_Prev = tv_Power_Status;
    // }

    //   tv_Power_Status
}
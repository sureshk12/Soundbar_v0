#include <Arduino.h>

#include "cec.h"
#include "supportFun.h"
#include "cecTransmit.h"
#include "global.h"

// global variables
extern bool isHotplug;
extern bool isTvCecOn;
extern bool isTvOn;
extern bool tvArc;
extern uint8_t lastStatusArray[NO_OF_LASTSTATUS];
extern uint8_t sb_power;
uint8_t arcState = 0;
uint8_t arcStopState = 0;
extern uint8_t cecTxErrorCount;
extern bool cecTxDisableErrChk;
extern bool cecTxReady;
extern bool cecRxFlag;
extern bool cecTxFlag;
extern bool stopKeyHDMI;
// Local variables
static uint8_t hdmiHotplugStatusCount = 0;
static long hdmiHotplugPrevTime = millis();
static uint8_t hdmiHotplugPrevPinState = LOW;
static uint8_t hdmiDebounce = 0;
static uint8_t actualHotPlugPrevStatus = LOW;

static long arcDelayPrev = millis();
static long arcDelayIntervel = 0;
static uint8_t sourceBeforeHdmi = 0;

// prototypes
void hotPlugReset();
void doHdmiHotPlugTask();
void doArcTask();
void stopArc();
void doStopArcTask();

void hotPlugReset()
{
    hdmiHotplugStatusCount = 0;
    hdmiHotplugPrevTime = millis();
    hdmiHotplugPrevPinState = LOW;
    isHotplug = false;
    arcState = 0;
}

void doHdmiHotPlugTask()
{
    if ((millis() - hdmiHotplugPrevTime) > 100) // wait 100mS
    {
        if (!cecTxFlag && !stopKeyHDMI) // If No active CEC Tx and
        {
            // generateDebugPulse(10);
            //  Serial.println("[CEC:57]");
            hdmiHotplugPrevTime = millis();

            uint8_t hdmiHotplugCurrPinState = digitalRead(HDMI_HOTPLUG_PIN);
            if (hdmiHotplugCurrPinState == hdmiHotplugPrevPinState)
            {
                // Serial.println("[CEC:63]");
                if (hdmiDebounce != 4)
                {
                    // Serial.println("[CEC:56]");
                    hdmiDebounce++;
                    if (hdmiDebounce > 3)
                    {
                        hdmiDebounce = 4;
                        if (actualHotPlugPrevStatus != hdmiHotplugCurrPinState)
                        {
                            actualHotPlugPrevStatus = hdmiHotplugCurrPinState;
                            if (hdmiHotplugCurrPinState == HIGH)
                            {
                                isHotplug = true; // Hotplug detected
                                // Serial.println("HOTPLUG HIGH");
                                sourceBeforeHdmi = lastStatusArray[0]; // Store prev source ststus in temporary location
                                lastStatusArray[0] = SOURCE_HDMI_CEC;  // Chamge source to CEC-ARC
                                if (sb_power != PWR_ON)                // If SB is OFF switch on SB
                                {
                                    doSbPowerOnAction(); // this also will switch to current source
                                }
                                else
                                {
                                    if (lastStatusArray[0] != SOURCE_HDMI_CEC)
                                    {
                                        doSourceselection(lastStatusArray[0]); // Change the source to current source
                                    }
                                }
                            }
                            else
                            {
                                // If HDMI disconnected (Hotplug low), switch to previous selected source
                                if (sb_power == PWR_ON)
                                {
                                    // Serial.println("HOTPLUG LOW");
                                    lastStatusArray[0] = sourceBeforeHdmi;
                                    doSourceselection(lastStatusArray[0]);
                                    isHotplug = false;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                hdmiDebounce = 0;
                hdmiHotplugPrevPinState = hdmiHotplugCurrPinState;
            }
            // if (hdmiHotplugCurrPinState == HIGH)
            // {
            //     if (!isHotplug)
            //     {
            //         isHotplug = true;

            // if ((millis() - hdmiHotplugPrevtime) > 100)
            // {
            //     generateDebugPulse();
            //     hdmiHotplugPrevtime = millis();
            //     hdmiHotplugStatusCount++;
            //     if (hdmiHotplugStatusCount > 4)
            //     {
            //         isHotplug = true;
            //         sourceBeforeHdmi = lastStatusArray[0];
            //         lastStatusArray[0] = SOURCE_HDMI_CEC;
            //         if (sb_power != PWR_ON)
            //         {
            //             doSbPowerOnAction();
            //         }
            //         else
            //         {
            //             if(lastStatusArray[0] != SOURCE_HDMI_CEC)
            //             {
            //                 doSourceselection(lastStatusArray[0]);
            //             }
            //         }
            //     }
            // }
            //     }
            // }
            // else
            // {
            //     if (isHotplug)
            //     {
            //         isHotplug = false;
            //         isTvCecOn = false;
            //         isTvOn = false;
            //         tvArc = false;
            //         // Serial.println("TV Disconnected");
            //         doSourceselection(sourceBeforeHdmi);
            //     }
            // }
            // generateDebugPulse(10);
        }
    }
}

void doArcTask()
{
    // uint8_t hdmiHotplugCurrPinState = digitalRead(HDMI_HOTPLUG_PIN);
    // if (hdmiHotplugCurrPinState == HIGH && !isHotplug && arcState == 0)
    //  Serial.printf("[CEC:89] %s\n", isHotplug? "HOTplu Ok" : "Hotplug not Ok");
    // generateDebugPulse();
    if (isHotplug && (arcState == 0))
    {
        // Serial.println("[CEC:91]");
        //  isHotplug = true;
        arcState = 11;
        arcDelayPrev = millis();
        arcDelayIntervel = 0;
    }

    if (arcState == 11)
    {
        if ((millis() - arcDelayPrev) > arcDelayIntervel)
        {
            // if (hdmiHotplugCurrPinState == HIGH)
            // {
            cecTxErrorCount = 0;
            cecTxDisableErrChk = false;
            broadcast_oneByte();
            arcDelayPrev = millis();
            arcDelayIntervel = 50;
            arcState = 12;
            // }
            // else
            // {
            //     isHotplug = false;
            //     arcState = 0;
            // }
        }
    }
    if (arcState == 12)
    {
        if ((millis() - arcDelayPrev) - arcDelayIntervel)
        {
            if (cecTxReady == true)
            {
                cecTxErrorCount = 0;
                cecTxDisableErrChk = true;
                get_TvPowerStatus();
                arcDelayPrev = millis();
                arcState = 13;

                // else
                // {
                //     if ((millis() - arcDelayPrev) > 400)
                //     {
                //         //isHotplug = false;
                //         arcState = 0;
                //     }
                // }
            }
        }
    }
    if (arcState == 13)
    {
        if (cecTxReady == true)
        {
            if (isTvOn)
            {
                cecTxErrorCount = 0;
                cecTxDisableErrChk = true;
                set_tvSystemAudioMode(0x01);
                arcDelayPrev = millis();
                arcDelayIntervel = 50;
                arcState = 14;
            }
            else
            {
                if ((millis() - arcDelayPrev) > 500)
                {
                    // isHotplug = false;
                    // isTvCecOn = false;
                    arcDelayIntervel = 0;
                    arcState = 11;
                }
            }
        }
    }
    if (arcState == 14)
    {
        if ((millis() - arcDelayPrev) > arcDelayIntervel)
        {
            if (!tvArc)
            {
                if (cecTxReady == true)
                {
                    cecTxErrorCount = 0;
                    cecTxDisableErrChk = true;
                    set_tvArcOn();
                    arcState = 13;
                    arcDelayPrev = millis();
                    arcDelayIntervel = 1000;
                }
            }
            else
            {
                arcState = 15;
            }
        }
    }
    if (arcState == 15)
    {
        if (cecTxReady == true)
        {
            if (tvArc)
            {
                cecTxErrorCount = 0;
                cecTxDisableErrChk = true;
                set_tvSystemAudioMode(0x01);
                arcDelayPrev = millis();
                arcDelayIntervel = 50;
                arcState = 16;
            }
            else
            {
                if ((millis() - arcDelayPrev) > arcDelayIntervel)
                {
                    arcState = 13;
                }
            }
        }
    }
    if (arcState == 16)
    {
        if (cecTxReady == true)
        {
            report_sbVolumeMuteStatus();
            arcDelayPrev = millis();
            arcDelayIntervel = 50;
            arcState = 17;
        }
    }
    if (arcState == 17)
    {
        Serial.printf("finally here");
        arcState = 18;
    }
    if (arcState == 18)
    {
    }
}

void stopArc()
{
    if (!isHotplug)
    {
        tvArc = false;
    }
    else
    {
        arcStopState = 0;
        arcDelayPrev = millis();
        arcDelayIntervel = 0;
    }
}

void doStopArcTask()
{
    if(arcStopState == 0) {
        set_tvSystemAudioMode(0x00);
        arcDelayPrev = millis();
        arcDelayIntervel = 50;
        arcStopState = 11;
    }
    if(arcStopState == 11) {
        set_tvArcTermination();
        arcDelayPrev = millis();
        arcStopState = 12;
    }
    if(arcStopState == 12) {
        Serial.println("Stopped ARC");
        arcStopState = 13;
    }
    if(arcStopState == 13) {
        //just return
    }
}
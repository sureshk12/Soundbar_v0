#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>

#include "global.h"
#include "supportFun.h"
// #include "modeBt.h"
#include "sb_I2c.h"
#include "cec.h"
#include "cecTransmit.h"
#include "lastData.h"
#include "btUsbFunction.h"

// Global Variables
// System constants
Preferences preferences;
extern uint8_t lastStatusArray[NO_OF_LASTSTATUS];
extern LastDataStruct lastValueStruct;
extern bool isTvCecOn;
extern bool isHotplug;
bool sb_Mute = MUTE_ON;
// extern uint8_t sb_Volume;
extern uint8_t sb_power;
extern bool doNotAck;
bool btConnected = false;
extern uint8_t arcState;

struct btDeviceStatusStruct btDeviceStatus;

void systemBoot();
void processFunction(uint8_t keyCode);
void doSbPowerOnAction();
void doSbPowerStdByAction();
void setMuteOn();
void setMuteOff();
void doSourceselection(uint8_t source);
void getLastStatus();
void setLastStatus();
void stopHere();
void generateDebugPulse(uint8_t intervel);
void updateBtVolume(uint8_t newVolume);

void systemBoot()
{
    delay(100);
    // Set up hardware
    pinMode(PIN_KEY_POWER, INPUT_PULLUP);    // KEY BOARD LSB Bit0 : POWER
    pinMode(PIN_KEY_VOLPLUS, INPUT_PULLUP);  // KEY BOARD MID Bit1 : Volume Plus
    pinMode(PIN_KEY_VOLMINUS, INPUT_PULLUP); // KEY BOARD MSB Bit2 : Volume Minus
    pinMode(PIN_CEC_TEST, OUTPUT);           // Debug Pin
    digitalWrite(PIN_CEC_TEST, HIGH);        // Keep debug pin in default state = HIGH
    pinMode(PIN_IR, INPUT_PULLUP);           // IR input DETECT
    pinMode(PIN_BT_USB_MODE, OUTPUT);        // BT_USB_MODE
    digitalWrite(PIN_BT_USB_MODE, HIGH);     // Default is HIGH
    pinMode(PIN_STDBY, OUTPUT);
    digitalWrite(PIN_STDBY, LOW);             // Put System in STDBY
    pinMode(PIN_CEC_READ, INPUT);             // CEC Read pin
    pinMode(PIN_CEC_WRITE, OUTPUT);           // CEC Write pin
    digitalWrite(PIN_CEC_WRITE, LOW);         // Set CECWrite Pin low or keep CEC Bus High
    pinMode(PIN_MUX_INT0, INPUT);             // Interrupt pin for MUX0
    pinMode(PIN_MUX_INT1, INPUT);             // Interrupt pin for MUX1
    pinMode(PIN_HDMI_HOTPLUG, INPUT);         // Hotplug Detect
    pinMode(PIN_BT_USB_NEXT, OUTPUT);         // NEXT
    digitalWrite(PIN_BT_USB_NEXT, HIGH);      // Default is HIGH
    pinMode(PIN_BT_USB_PREV, OUTPUT);         // PREV
    digitalWrite(PIN_BT_USB_PREV, HIGH);      // Default is HIGH
    pinMode(PIN_BT_USB_PLAYPAUSE, OUTPUT);    // PLAYPAUSE_PAIR
    digitalWrite(PIN_BT_USB_PLAYPAUSE, HIGH); // Deafult HIGH

    hotPlugReset(); // Hotplug Reset     // ***** Reset Hotplug status to known status *****
    Serial.begin(115200); // Enable Serial communication

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

    // ***** Initilize OLED ****
    if (!intilizeOled())
    {
        Serial.printf("ERROR: Could not Initialize OLED\n");
        stopHere();
    }
    Serial.printf("OK:OLED\n");
    delay(10);

    byte error = 1;
    byte errorCount = 1;

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

    //***** Initilaize BT USB*****
    // initilizeBtUsb_BK828881();
    // initilizeBtUsb_PCF8574();

        // ***** Get last status Source, Volume, Mode ****
    getLastStatus();
    changeDspVolume(lastStatusArray[1]);
    doSourceselection(lastStatusArray[0]);

    btDeviceStatus.btDeviceVolume = 0x06;
    btDeviceStatus.btPlayerStatus = 0x01;
    btDeviceStatus.bt_Usb_Mode = 0x01;

    getBtDevStatus();

    printf("System Boot OK\n");
    delay(10);
    // generateDebugPulse();

    // Get last status
}

void processFunction(uint8_t keyCode)
{
    /*
    //DEBUG START
    Serial.printf("BeforeChange : powerLastStatus = %d, sourceLastStatus = %d, volumeLastStatus = %d,  muteLastStatus = %d,  modeLastStatus = %d\n",
            lastValueStruct->powerLastStatus, lastValueStruct->sourceLastStatus, lastValueStruct->volumeLastStatus, lastValueStruct->muteLastStatus, lastValueStruct->modeLastStatus);
    //DEBUF END
    */
    /*
     * Previous, Next , Playpass action is valid only for
     * BT
     * USB
     * Volume Plus and Volume minus is valid for DSP and BT
     */
    if (keyCode == KEYCODE_PAIRING)
    {
        if (lastStatusArray[0] == SOURCE_BT)
        {
            Serial.println("Action PAIRING in BT mode");
            // stopHere();
            doKeyBtUsbAction(keyCode);
        }
    }

    if (keyCode == KEYCODE_PREVIOUS || keyCode == KEYCODE_NEXT || keyCode == KEYCODE_PLAY_PAUSE)
    {
        // Serial.println("Action PLAY/PAUSE or PREVIOUS or NEXT!");
        if (lastStatusArray[0] == SOURCE_BT || lastStatusArray[0] == SOURCE_USB)
        {
            Serial.println("Action PLAY/PAUSE or PREVIOUS or NEXT and in BT mode");
            // stopHere();
            doKeyBtUsbAction(keyCode);
        }
    }
    /*
     *Volume Plus and Volume minus is valid for DSP and BT
     */
    if (keyCode == KEYCODE_VOLUME_MINUS || keyCode == KEYCODE_VOLUME_PLUS)
    {
        // If DSP Volume is > BT Volume, then Increase BT Volume Else increase DSP Volume
        // Serial.println("Action Vol + or Vol -");
        // stopHere();

        /*
        This may not be required as we will be updaing the volume by I2C

        if (lastStatusArray[0] == SOURCE_BT || lastStatusArray[0] == SOURCE_USB)
        {
            Serial.println("Action Vol + or Vol - AND in BT OR USB");
            doKeyBtUsbAction(keyCode);
        }
        */
        // Serial.println("Action Vol + or Vol - AND in NON BT OR NON USB");
        if (keyCode == KEYCODE_VOLUME_MINUS)
        {
            // Serial.printf("Action Vol - before value is %d\n", lastStatusArray[1]);
            //  was 4
            if (lastStatusArray[1] > 0)
            {
                lastStatusArray[1] = lastStatusArray[1] - 1;
            }
            // Serial.printf("Action Vol - After value is %d\n", lastStatusArray[1]);
            changeDspVolume(lastStatusArray[1]);
            if (lastStatusArray[0] == SOURCE_BT || lastStatusArray[0] == SOURCE_USB)
            {
                writeVolumeDataToBTUSB(lastStatusArray[1]);
            }
        }
        if (keyCode == KEYCODE_VOLUME_PLUS)
        {
            // Serial.printf("Action Vol + before value is %d\n", lastStatusArray[1]);
            lastStatusArray[1] = lastStatusArray[1] + 1; // was 4
            if (lastStatusArray[1] > 32)
            {
                lastStatusArray[1] = 32;
            }
            // Serial.printf("Action Vol + After lastStatusArray[1] value is %d\n", lastStatusArray[1]);
            changeDspVolume(lastStatusArray[1]);
            if (lastStatusArray[0] == SOURCE_BT || lastStatusArray[0] == SOURCE_USB)
            {
                writeVolumeDataToBTUSB(lastStatusArray[1]);
            }
        }

        setLastData(); // Update last ststus in memeory
        // TO DO
        // Update Local DSP Volume level
        Serial.printf("Volume is set at %d\n", lastStatusArray[1]);
    }

    if (keyCode == KEYCODE_STANDBY)
    {
        Serial.printf("STAND BY KEY PRESSED, previous sb_power is %d\n", sb_power);
        if (sb_power == PWR_STDBY)
        {
            sb_power = PWR_ON;
            Serial.println("SB ON");
            doSbPowerOnAction();
        }
        else
        {
            sb_power = PWR_STDBY;
            printf("SB OFF Line 188\n");
            doSbPowerStdByAction();
            oledDisplayLogo();
        }
    }

    if (keyCode == KEYCODE_SOURCE)
    {
        lastStatusArray[0]++;
        if (lastStatusArray[0] > 5)
        {
            lastStatusArray[0] = 1;
        }
        setLastData(); // Update last status in memeory
        mutePcm();
        btConnected = false; // Added to solve the BT held up
        doSourceselection(lastStatusArray[0]);
        unmutePcm();
    }

    if (keyCode == KEYCODE_MUTE)
    {
        // printf("Before MUTE change status od sb_Mute %d\n", sb_Mute);

        if (sb_Mute == MUTE_OFF)
        {
            // sb_Mute = MUTE_ON;
            // Serial.println("MUTE ON");
            setMuteOn();
        }
        else
        {
            // sb_Mute = MUTE_OFF;
            // Serial.println("MUTE OFF");
            setMuteOff();
        }
    }
}

void doSbPowerOnAction()
{
    sb_power = PWR_ON;
    setMuteOn();
    digitalWrite(PIN_STDBY, LOW); // SB ON
    doSourceselection(lastStatusArray[0]);
    setMuteOff();
}

void doSbPowerStdByAction()
{
    // Put SB into Stand By
    sb_power = PWR_STDBY;
    // btDisconnect();
    btConnected = false;
    setMuteOn();
    digitalWrite(PIN_STDBY, HIGH); // SB OFF
    oledDisplayLogo();             // Display boot up logo
}

void setMuteOn()
{
    // MUTE ON
    // mutePcm();
    changeDspMuteOn();
    // digitalWrite(MUTE_PIN, HIGH);
    // Serial.println("MUTE ON");
    sb_Mute = MUTE_ON;
}

void setMuteOff()
{
    // MUTE ON
    // unmutePcm();
    changeDspMuteOff();
    // digitalWrite(MUTE_PIN, LOW);
    // Serial.println("MUTE OFF");
    sb_Mute = MUTE_OFF;
}

void doSourceselection(uint8_t source)
{
    if (source == SOURCE_HDMI_CEC)
    {
        if (isTvCecOn)
        {
            set_tvArcOn();
        }
    }
    else
    {
        if (isTvCecOn)
        {
            set_tvArcTermination();
        }
    }
    switch (source)
    {
    case SOURCE_AUX:
        Serial.println("AUX mode Selected");
        if (!sourceSelectAux())
        {
            Serial.printf("ERROR: AUX could not be selected");
            stopHere();
        }
        // btDisconnect();
        btConnected = false;
        break;
    case SOURCE_BT:
        Serial.println("BT mode Selected");
        // 	if (!btEnabled)
        // 	{
        // 		// Start Blutooth Module
        // 		if (!btModeBegin())
        // 		{
        // 			Serial.printf("ERROR: Blue Tooth could not be started");
        // 			stopHere();
        // 		}
        // 		btConnected = true;
        // 		btEnabled = true;
        // 	}
        // 	else
        // 	{
        // 		btReConnect();
        // 		btConnected = true;
        // 	}
        if (!sourceSelectBT())
        {
            Serial.printf("ERROR: BT could not be selected");
            stopHere();
        }
        // stopHere();
        break;
    case SOURCE_TOSLINK:
        Serial.println("TOSLINK mode Selected");
        if (!sourceSelectToslink())
        {
            Serial.printf("ERROR: TOSLINK could not be selected");
            stopHere();
        }
        // btDisconnect();
        btConnected = false;
        break;
    case SOURCE_HDMI_CEC:
        Serial.println("HDMI CEC ARC mode Selected");
        if (!sourceSelectHdmiCec())
        {
            Serial.printf("ERROR: HDMI CEC ARC could not be selected");
            stopHere();
        }
        // btDisconnect();
        btConnected = false;
        arcState = 0;
        doNotAck = false;
        // hotPlugReset();
        break;
    case SOURCE_USB:
        Serial.println("USB mode Selected");
        if (!sourceSelectUsb())
        {
            Serial.printf("ERROR: USB could not be selected");
            stopHere();
        }
        // btDisconnect();
        btConnected = false;
        break;
    default:
        // Do nothing
        break;
    }
    if (sb_power == PWR_STDBY)
    {
        oledDisplayLogo();
    }
}

void getLastStatus()
{
    // GET Last status
    preferences.begin("lastStatus", false);
    // Simulate first time switch on
    //  createNewLastData();
    //  End simulation
    // uint8_t temp = preferences.getUChar("powerLastStatus", 0);
    uint8_t temp = preferences.getUChar("firstTime", 0);
    preferences.end();
    if (temp != 1)
    {
        Serial.printf("First time Inatalization\n");
        if (!createNewLastData())
        {
            Serial.printf("ERROR: Could not intilizse Last Data");
            stopHere();
        }
    }
    // Read the latest Last data
    if (!getLastData())
    {
        Serial.printf("ERROR: Reading Last Status");
        while (1)
        {
        }
    };
    lastValueStruct.powerLastStatus = lastStatusArray[0];
    lastValueStruct.sourceLastStatus = lastStatusArray[1];
    lastValueStruct.volumeLastStatus = lastStatusArray[2];
    lastValueStruct.muteLastStatus = lastStatusArray[3];
    lastValueStruct.modeLastStatus = lastStatusArray[4];

    btDeviceStatus.btDeviceVolume = lastStatusArray[1];
    if (lastStatusArray[0] == SOURCE_BT)
    {
        btDeviceStatus.bt_Usb_Mode = 0;
    }
    else
    {
        btDeviceStatus.bt_Usb_Mode = 0;
    }
    btDeviceStatus.btPlayerStatus = 0;
}

void updateBtVolume(uint8_t newVolume)
{

    // lastStatusArray[1]
    if (newVolume < 0)
    {
        newVolume = 0;
    }
    if (newVolume > 32)
    {
        newVolume = 32;
    }
    lastStatusArray[1] = newVolume;
    changeDspVolume(lastStatusArray[1]);
    if (lastStatusArray[0] == SOURCE_BT || lastStatusArray[0] == SOURCE_USB)
    {
        writeVolumeDataToBTUSB(lastStatusArray[1]);
    }
}

// HALT FUNCTION FOR DEBUGGING AND ERRORS which cannot be recoverd
void stopHere()
{
    Serial.println("STOPPING HERE FOREEVER..............");
    while (1)
    {
        // STOP HERE
    }
}

void generateDebugPulse(uint8_t intervel)
{
    digitalWrite(PIN_CEC_TEST, LOW);
    delayMicroseconds(intervel);
    digitalWrite(PIN_CEC_TEST, HIGH);
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
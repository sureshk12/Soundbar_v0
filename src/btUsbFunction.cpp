#include <Arduino.h>
#include <Wire.h>

#include "global.h"
#include "supportFun.h"
#include "sb_I2c.h"
#include "btUsbFunction.h"

// Static Function

static uint8_t keyCodeStatic;
static long pulseTriggerTime;

// global
extern bool pendingBtUsbKeyProcess;
extern uint8_t currIoData;
extern uint8_t lastStatusArray[NO_OF_LASTSTATUS];
extern struct btDeviceStatusStruct btDeviceStatus;

// prototypes
bool initilizeBtUsb_BK828881();
bool initilizeBtUsb_PCF8574();
bool doKeyBtUsbAction(uint8_t keyCode);
bool switchToBtMode();
bool switchToUsbMode();
void doBtUsbTask();
void writeVolumeDataToBTUSB(byte volLevel);
void doCheckBtDeviceStatus();

bool initilizeBtUsb_BK828881()
{
    byte error = 1;
    byte errorCount = 1;
    while (error != 0)
    {
        Wire.beginTransmission(BT_USB_BK82881_I2C_ADDR);
        byte error = Wire.endTransmission();
        if (error == 0)
        {
            Serial.printf("BT_USB_BK82881 Found :-) error = %d\n", error);
            return true;
        }
        else
        {
            Serial.printf("%d : BT_USB_BK82881 NOT FOUND :-(\n", errorCount);
            delay(1000);
            errorCount++;
            if (errorCount > 5)
            {
                stopHere();
            }
        }
    }
    return true;
}

bool initilizeBtUsb_PCF8574()
{
    byte error = 1;
    byte errorCount = 1;
    while (error != 0)
    {
        Wire.beginTransmission(BT_USB_PCF8574_I2C_ADDR);
        byte error = Wire.endTransmission();
        if (error == 0)
        {
            Serial.println("BT_USB_PCF8574 Found :-)");
            return true;
        }
        else
        {
            Serial.printf("%d : BT_USB_PCF8574 NOT FOUND :-(\n", errorCount);
            delay(1000);
            errorCount++;
            if (errorCount > 5)
            {
                stopHere();
            }
        }
    }
    return true;
}

bool doKeyBtUsbAction(uint8_t keyCode)
{
    bool shortPulse;
    uint8_t mcuPin;
    // Write to correcponding key on I/O Expander
    // keycode = KEYCODE_VOLUME_MINUS, KEYCODE_VOLUME_PLUS, KEYCODE_PREVIOUS, KEYCODE_NEXT, KEYCODE_PLAY_PAUSE
    // Serial.printf("Iam in doKeyBtUsbAction with Keyvode %d, CuurIOData %x\n", keyCode, currIoData);
    switch (keyCode)
    {
    /* Not require as Vol+ and Vol - directly managed
    case KEYCODE_VOLUME_PLUS:
        keyCodeStatic = BT_USB_VOLPLUS_NEXT;
        shortPulse = true;
        break;
    */
    case KEYCODE_NEXT:
        keyCodeStatic = BT_USB_VOLPLUS_NEXT;
        shortPulse = false;
        mcuPin = PIN_BT_USB_NEXT;
        break;
    /* Not require as Vol+ and Vol - directly managed
    case KEYCODE_VOLUME_MINUS:
        keyCodeStatic = BT_USB_VOLMINUS_PREV;
        shortPulse = true;
        break;
     */
    case KEYCODE_PREVIOUS:
        keyCodeStatic = BT_USB_VOLMINUS_PREV;
        shortPulse = false;
        mcuPin = PIN_BT_USB_PREV;
        break;
    case KEYCODE_PLAY_PAUSE:
        keyCodeStatic = BT_USB_PLAYPAUSE_PAIRING;
        shortPulse = true;
        mcuPin = PIN_BT_USB_PLAYPAUSE;
        break;
    case KEYCODE_PAIRING:
        keyCodeStatic = BT_USB_PLAYPAUSE_PAIRING;
        shortPulse = false;
        mcuPin = PIN_BT_USB_PLAYPAUSE;
        break;
    }
    // Serial.printf("Value before of currIodata is %04x and keyCodeStatic is %04x\n", currIoData, keyCodeStatic);

    currIoData = (keyCodeStatic ^ 0xFF) & currIoData;
    // Serial.printf("Old currIoData before I2C %x\n", currIoData);
    // Serial.printf("Value after of currIodata is %04x and keyCodeStatic is %04x\n", currIoData, keyCodeStatic);
    if (shortPulse)
    {
        pulseTriggerTime = millis() + SHORT_PULSE_DURATION;
    }
    else
    {
        pulseTriggerTime = millis() + LONG_PULSE_DURATION;
    }
    // Serial.printf("Delta time is %lu pulseTriggerTime =%lu and curr nillis is = %lu\n", (pulseTriggerTime - millis()), pulseTriggerTime, millis());
    pendingBtUsbKeyProcess = true;
    // writeByteNoByteAddrToI2c(BT_USB_PCF8574_I2C_ADDR, currIoData);
    // Serial.printf("mcuPin Made LOW is %d, pulseTriggerTime %ld\n", mcuPin, pulseTriggerTime);
    digitalWrite(mcuPin, LOW);
    while (millis() < pulseTriggerTime)
    {
        // do Nothing
    }
    // currIoData = currIoData | keyCodeStatic;
    // writeByteNoByteAddrToI2c(BT_USB_PCF8574_I2C_ADDR, currIoData);
    digitalWrite(mcuPin, HIGH);
    // Serial.printf("mcuPin Made HIGH is %d, pulseTriggerTime %ld\n", mcuPin, pulseTriggerTime);
    // Serial.printf("New currIoData After I2C %x\n", currIoData);
    return true;
}

bool switchToUsbMode()
{
    // Serial.printf("Iam at 145 switchToUSB and BEFORE currIoData is %02x \n", currIoData);
    currIoData = (0xBF & currIoData); // was (0x40 ^ 0xFF) & currIoData
    // Serial.printf("Iam at 147 Iam at 145 switchToUSB and AFTER currIoData is %02x \n", currIoData);
    // return writeByteNoByteAddrToI2c(BT_USB_PCF8574_I2C_ADDR, currIoData);
    digitalWrite(PIN_BT_USB_MODE, LOW);
    return true;
}
bool switchToBtMode()
{
    // Serial.printf("Iam at 86 and BEFORE currIoData is %02x \n", currIoData);
    currIoData = (0x40 | currIoData);
    // Serial.printf("Iam at 88 and AFTER currIoData is %02x \n", currIoData);
    // return writeByteNoByteAddrToI2c(BT_USB_PCF8574_I2C_ADDR, currIoData);
    digitalWrite(PIN_BT_USB_MODE, HIGH);
    return true;
}

void doBtUsbTask()
{
    if (pendingBtUsbKeyProcess)
    {
        if (millis() > pulseTriggerTime)
        {
            // Serial.println("Iam @ 96 btUsbFunction");
            currIoData = (keyCodeStatic | currIoData);
            writeByteNoByteAddrToI2c(BT_USB_PCF8574_I2C_ADDR, currIoData);
            pendingBtUsbKeyProcess = false;
        }
    }
}

void writeVolumeDataToBTUSB(byte volLevel)
{
    byte volLevelBTUSb = volLevel / 2;
    changeBtUsbVolLevel(volLevelBTUSb);
}

void doCheckBtDeviceStatus()
{
    getBtDevStatus();
    if (btDeviceStatus.btDeviceVolume != (lastStatusArray[1] / 2))
    {

        // updateBtVolume(btDeviceStatus.btDeviceVolume * 2);
    }
    // check others? Mode , player status
}
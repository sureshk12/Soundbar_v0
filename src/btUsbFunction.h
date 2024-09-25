#pragma once
#include <Arduino.h>

bool initilizeBtUsb_BK828881();
bool initilizeBtUsb_PCF8574();
bool doKeyBtUsbAction(uint8_t keyCode);
bool switchToBtMode();
bool switchToUsbMode();
void doBtUsbTask();
void writeVolumeDataToBTUSB(byte volLevel);
void doCheckBtDeviceStatus();

#pragma once
#include <Arduino.h>

bool intilizePCM9211();
bool sourceSelectAux();
bool sourceSelectBT();
bool sourceSelectToslink();
bool sourceSelectHdmiCec();
bool sourceSelectUsb();
bool mutePcm();
bool unmutePcm();
void changeDspVolume(uint8_t volLevel);
void changeDspMuteOn();
void changeDspMuteOff();

bool intilizeOled();
void oledDisplayString(char *str);
void oledDisplayLogo();
void oledDisplaySource();
void oledDisplayVolume();
void doDisplayResetToSource();

bool intilizeBtPcf8574();
void getBtDevStatus();

uint8_t writeOneByteToI2c(uint8_t i2cAddr, uint8_t i2cReg, uint8_t i2cData);
uint8_t writeByteNoByteAddrToI2c(uint8_t i2cAddr, uint8_t i2cData);
int writeManyByteToI2c(uint8_t i2cAddr, uint8_t i2cReg, uint8_t *i2cData, uint8_t i2cNoOfBytes);
int readOneByteFromI2d(uint8_t i2cAddr, uint8_t i2cReg, uint8_t *i2cData);
int readManyBytefromI2c(uint8_t i2cAddr, uint8_t i2cReg, uint8_t *i2cData, uint8_t i2cNoOfBytes);
bool changeBtUsbVolLevel(byte volLevelBtUsb);
// void readBtUsbStatus();

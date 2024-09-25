#pragma once

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
void changeDspVolume(uint8_t volLevel);
void updateBtVolume(uint8_t newVolume);

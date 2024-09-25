#pragma once

bool initilizeAudioDsp();
bool writeRam(uint8_t ramArray[][5], uint8_t ramsize, uint8_t ramBank);
bool writeReg(uint8_t regArray[][2], uint8_t regsize);

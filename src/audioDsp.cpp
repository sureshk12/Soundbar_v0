#include <Arduino.h>
#include <Wire.h>
#include "global.h"
#include "sb_I2c.h"
#include "BK328881.h"

#include "audioDsp.h"

// Local variables
int writeDelay1 = 1;
int writeDelay2 = 3;
// Global variables

// Prototypes
bool initilizeAudioDsp();
bool writeRam(uint8_t ramArray[][5], uint8_t ramsize, uint8_t ramBank);
bool writeReg(uint8_t regArray[][2], uint8_t regsize);

// Functions
bool initilizeAudioDsp()
{

    writeRam(m_ram1_tab, 0xE0, 1);
    writeRam(m_ram2_tab, 0x60, 2);
    writeReg(m_reg_tab, 0x7F);
    digitalWrite(PIN_STDBY, HIGH);
    return true;
}

bool writeRam(uint8_t ramArray[][5], uint8_t ramsize, uint8_t ramBank)
{
    uint8_t countRam = 0;
    while (countRam < (ramsize + 1))
    {
        // Serial.printf("{%#02x,%#02x,%#02x,%#02x,%#02x}\n", ramArray[countRam][0], ramArray[countRam][1], ramArray[countRam][2], ramArray[countRam][3], ramArray[countRam][4]);
        Wire.beginTransmission(AMP_DSP_I2C_ADDR);
        Wire.write(0x1D);
        Wire.write(ramArray[countRam][0]);
        Wire.endTransmission(true);
        delay(writeDelay1);

        Wire.beginTransmission(AMP_DSP_I2C_ADDR);
        Wire.write(0x1E);
        Wire.write(ramArray[countRam][1] & 0x0F);
        Wire.write(ramArray[countRam][2]);
        Wire.write(ramArray[countRam][3]);
        Wire.write(ramArray[countRam][4]);
        Wire.endTransmission(true);
        delay(writeDelay1);

        Wire.beginTransmission(AMP_DSP_I2C_ADDR);
        Wire.write(0x32);
        if (ramBank == 1)
        {
            Wire.write(0x01);
        }
        else if (ramBank == 2)
        {
            Wire.write(0x41);
        }
        Wire.endTransmission(true);
        delay(writeDelay1);

        Wire.beginTransmission(AMP_DSP_I2C_ADDR);
        Wire.write(0x32);
        if (ramBank == 1)
        {
            Wire.write(0x00);
        }
        else if (ramBank == 2)
        {
            Wire.write(0x40);
        }
        Wire.endTransmission(true);

        delay(writeDelay2);

        countRam++;
    }
    return true;
}
bool writeReg(uint8_t regArray[][2], uint8_t regsize)
{
    uint8_t countReg = 0;
    while (countReg < (regsize + 1))
    {
        // Serial.printf("{%#02x,%#02x}\n", regArray[countReg][0], regArray[countReg][1]);
        Wire.beginTransmission(AMP_DSP_I2C_ADDR);
        Wire.write(regArray[countReg][0]);
        Wire.write(regArray[countReg][1]);
        Wire.endTransmission(true);
        delay(writeDelay2);
        countReg++;
    }

    return true;
}
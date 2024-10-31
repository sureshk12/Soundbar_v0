#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_SSD1306.h>
// #include <Adafruit_SH110x.h>
#include "supportFun.h"
#include "sb_I2c.h"
// #include "modeBt.h"
#include "btUsbFunction.h"
#include "global.h"
#include "cecTransmit.h"

// Global variables
extern bool btConnected;
extern uint8_t lastStatusArray[NO_OF_LASTSTATUS];
extern bool resetDisplayToSource;
extern long prevResetDisplayToSource;
extern uint8_t currIoData;
extern uint8_t sb_Volume;
extern bool sb_Mute;
extern uint8_t sb_power;
extern bool isTvCecOn;
extern struct btDeviceStatusStruct btDeviceStatus;

// Prototypes
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

bool changeBtUsbVolLevel(uint8_t volLevelBtUsb);
// void readBtUsbStatus();

uint8_t writeOneByteToI2c(uint8_t i2cAddr, uint8_t i2cReg, uint8_t i2cData);
uint8_t writeByteNoByteAddrToI2c(uint8_t i2cAddr, uint8_t i2cData);

bool intilizePCM9211()
{
    uint8_t pcm9211_Init_Data[] = {
        0X40, 0X33,
        0X40, 0XC0,
        0X31, 0X1A,
        0X33, 0X22,
        0X20, 0X00,
        0X24, 0X00,
        0X26, 0X81,
        0X33, 0X22,
        0X21, 0X00,
        0x22, 0x01,
        0X23, 0X04,
        0X27, 0X00,
        0X2F, 0X04,
        0X30, 0X02,
        0X32, 0X22,
        0X34, 0XC2,
        0X37, 0X00,
        0X6A, 0X00,
        0X6B, 0X11,
        0X6D, 0X00,
        0x6F, 0x48};
#if EVM == 0
    if (writeOneByteToI2c(EVM_PCF8574_I2C_ADDR, 0x04, 0x04) != 0)
    {
        Serial.println("Could not intilize EVM_PC8574");
        return false;
    }
    delay(10);
    // writeOneByteToI2c(EVM_PCF8574_I2C_ADDR, 0x04, 0x04);
    // delay(10);
#endif

    for (int x = 0; x < sizeof(pcm9211_Init_Data); x++)
    {
        if ((writeOneByteToI2c(PCM9211_I2C_ADDR, pcm9211_Init_Data[x], pcm9211_Init_Data[x + 1])) != 0)
        {
            Serial.println("Could not Wite intilize data to PCM9211");
            return false;
        }
        delay(10);
        x++;
    }
    return true;
}

bool sourceSelectAux()
{
    // Switch PCM9211 to AUX (ADC)
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6B, 0x22);
    delay(10);
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6C, 0x22);
    char str[4] = "AUX";
    oledDisplayString(str);
    return true;
}

bool sourceSelectBT()
{
    // Switch BT_USB to BT
    switchToBtMode();
    // Switch PCM9211 to BT (I2S)
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6B, 0x33);
    delay(10);
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6C, 0x33);
    char str[3] = "BT";
    oledDisplayString(str);
    return true;
}

bool sourceSelectToslink()
{
// Switch PCM9211 to BT (RXIN2)
#if EVM == 0
    writeOneByteToI2c(EVM_PCF8574_I2C_ADDR, 0x04, 0x04);
    delay(10);
    writeOneByteToI2c(EVM_PCF8574_I2C_ADDR, 0x04, 0x04);
    delay(10);
#endif
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x34, 0xC2);
    delay(10);
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6B, 0x11);
    delay(10);
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6C, 0x11);
    char str[4] = "OPT";
    oledDisplayString(str);
    return true;
}

bool sourceSelectHdmiCec()
{
    // Switch PCM9211 to BT (RXIN0)
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x34, 0x40);
    delay(10);
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6B, 0x11);
    delay(10);
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6C, 0x11);
    char str[4] = "ARC";
    oledDisplayString(str);
    return true;
}

bool sourceSelectUsb()
{
    switchToUsbMode();

    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6B, 0x33);
    delay(10);
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6C, 0x33);
    char str[4] = "USB";
    oledDisplayString(str);
    return true;
}

bool mutePcm()
{
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6D, 0xA0);
    delay(1);
    return true;
}

bool unmutePcm()
{
    writeOneByteToI2c(PCM9211_I2C_ADDR, 0x6D, 0x00);
    delay(1);
    return true;
}

uint8_t writeOneByteToI2c(uint8_t i2cAddr, uint8_t i2cReg, uint8_t i2cData)
{
    Wire.beginTransmission(i2cAddr);
    Wire.write(i2cReg);
    Wire.write(i2cData);
    return Wire.endTransmission(true);
}

uint8_t writeByteNoByteAddrToI2c(uint8_t i2cAddr, uint8_t i2cData)
{
    Wire.beginTransmission(i2cAddr);
    Wire.write(i2cData);
    return Wire.endTransmission(true);
}

uint8_t volumeLevelArray[37][2] = {
    {0x00, 0x1A},
    {0x00, 0X20},
    {0x00, 0X29},
    {0x00, 0X34},
    {0x00, 0X41},
    {0x00, 0x52},
    {0x00, 0x68},
    {0x00, 0x82},
    {0x00, 0xA4},
    {0x00, 0xCF},
    {0X01, 0X04},
    {0X01, 0X48},
    {0X01, 0X9D},
    {0X02, 0X07},
    {0X02, 0X8E},
    {0X03, 0X37},
    {0X04, 0X0C},
    {0X05, 0X19},
    {0X06, 0X6A},
    {0X08, 0X14},
    {0X0A, 0X2B},
    {0X0C, 0XCD},
    {0X10, 0X1D},
    {0X14, 0X49},
    {0X19, 0X8A},
    {0X20, 0X27},
    {0X28, 0X7A},
    {0X32, 0XF5},
    {0X40, 0X27},
    {0X50, 0XC3},
    {0X65, 0XAD},
    {0X80, 0X00},
    {0XA1, 0X24},
    {0XCA, 0XDE},
    {0XFF, 0X65},
    {0X41, 0X85},
    {0X94, 0XC6}};

uint8_t ampDspVolumeData[32] = {
    93,
    90,
    87,
    84,
    81,
    78,
    75,
    72,
    69,
    66,
    63,
    60,
    57,
    54,
    51,
    48,
    45,
    42,
    39,
    36,
    33,
    30,
    27,
    24,
    21,
    18,
    15,
    12,
    9,
    6,
    3,
    0};

void changeDspVolume(uint8_t volLevel)
{
    Wire.beginTransmission(AMP_DSP_I2C_ADDR);
    Wire.write(AMP_DSP_VOLUME_ADDR);
    Wire.write(ampDspVolumeData[volLevel]);
    Wire.endTransmission(true);

    // Change the SB volume value to percentage and report to TV
    sb_Volume = volLevel * 3;
    if (volLevel == 31)
    {
        sb_Volume = sb_Volume + 2;
    }
    if (volLevel == 32)
    {
        sb_Volume = sb_Volume + 4;
    }
    if (lastStatusArray[0] == SOURCE_HDMI_CEC && isTvCecOn)
    {
        report_sbVolumeMuteStatus();
    }

    if (lastStatusArray[0] == SOURCE_BT || lastStatusArray[0] == SOURCE_USB)
    {
        writeVolumeDataToBTUSB(volLevel);
    }

    // Serial.printf("Volume = %d\n", volLevel);
    if (sb_power != PWR_STDBY)
    {
        oledDisplayVolume();
    }
}

void displayCecMuteStatus()
{    
    if (sb_Mute == MUTE_ON)
    {
        printf("UNMUTE\n");
        sb_Volume = lastStatusArray[1] * 3;
        if (lastStatusArray[1] == 31)
        {
            sb_Volume = sb_Volume + 2;
        }
        if (lastStatusArray[1] == 32)
        {
            sb_Volume = sb_Volume + 4;
        }
        if (lastStatusArray[0] == SOURCE_HDMI_CEC && isTvCecOn)
        {
            report_sbVolumeMuteStatus();
        }
    }
    else
    {
        // Change the SB volume value to percentage and report to TV
        printf("MUTE\n");
        if (lastStatusArray[0] == SOURCE_HDMI_CEC && isTvCecOn)
        {
            sb_Volume = 0;
            report_sbVolumeMuteStatus();
        }
    }
}

void changeDspMuteOn()
{
    Wire.beginTransmission(AMP_DSP_I2C_ADDR);
    Wire.write(AMP_DSP_MUTE_ADDR);
    Wire.write(AMP_DSP_MUTE_ON_DATA);
    Wire.endTransmission(true);

    // Chaage sb_volume
    sb_Volume = 0;
    // // Change the SB volume value to percentage and report to TV
    // if (lastStatusArray[0] == SOURCE_HDMI_CEC && isTvCecOn)
    // {
    //     report_sbVolumeMuteStatus();
    // }
    displayCecMuteStatus();
    // sb_Mute = MUTE_ON;
    // printf("After MUTE change status od sb_Mute %d\n", sb_Mute);
    char str[5] = "MUTE";
    oledDisplayString(str);
}

void changeDspMuteOff()
{
    Wire.beginTransmission(AMP_DSP_I2C_ADDR);
    Wire.write(AMP_DSP_MUTE_ADDR);
    Wire.write(AMP_DSP_MUTE_OFF_DATA);
    Wire.endTransmission(true);

    // // Change the SB volume value to percentage and report to TV
    // sb_Volume = lastStatusArray[1] * 3;
    // if (lastStatusArray[1] == 31)
    // {
    //     sb_Volume = sb_Volume + 2;
    // }
    // if (lastStatusArray[1] == 32)
    // {
    //     sb_Volume = sb_Volume + 4;
    // }
    // if (lastStatusArray[0] == SOURCE_HDMI_CEC && isTvCecOn)
    // {
    //     report_sbVolumeMuteStatus();
    // }
    displayCecMuteStatus();
    // sb_Mute = MUTE_OFF;
    // printf("After MUTE change status od sb_Mute %d\n", sb_Mute);

    oledDisplaySource();
}

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Adafruit_SH1106 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool intilizeOled()
{
    Serial.println("Started OLED Intilization");
    long startOledTime = millis();
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        return false;
    }
    Serial.print("Time taken is : ");
    Serial.println(millis() - startOledTime);
    Serial.println("Finished OLED Intilization");
    char str[6] = "ONIDA";
    oledDisplayString(str);
    return true;
}

void oledDisplayString(char *str)
{
    display.clearDisplay();
    // if(strlen(str) > 4)
    display.setTextSize(4);
    display.setTextColor(WHITE);
    uint8_t xpos = 0;
    // Serial.printf("Length = %d\n", strlen(str));
    switch (strlen(str))
    {
    case 1:
        xpos = 44;
        break;
    case 2:
        xpos = 34;
        break;
    case 3:
        xpos = 24;
        break;
    case 4:
        xpos = 14;
        break;
    case 5:
        xpos = 4;
        break;
    default:
        xpos = 0;
        break;
    }
    display.setCursor(xpos, 2);
    display.println(str);
    display.display();
}

void oledDisplayLogo()
{
    char str[6] = "ONIDA";
    oledDisplayString(str);
}

void oledDisplaySource()
{
    char sourceStr[5][4] = {"AUX", "BT", "OPT", "ARC", "USB"};
    oledDisplayString(sourceStr[lastStatusArray[0] - 1]);
}

void oledDisplayVolume()
{
    char str[6] = "Vol";
    if (lastStatusArray[1] < 10)
    {
        str[3] = '0';
        str[4] = lastStatusArray[1] + 48;
    }
    else if (lastStatusArray[1] < 20)
    {
        str[3] = '1';
        str[4] = lastStatusArray[1] + 38;
    }
    else if (lastStatusArray[1] < 30)
    {
        str[3] = '2';
        str[4] = lastStatusArray[1] + 28;
    }
    else if (lastStatusArray[1] < 33)
    {
        str[3] = '3';
        str[4] = lastStatusArray[1] + 18;
    }
    oledDisplayString(str);
    resetDisplayToSource = true;
    prevResetDisplayToSource = millis();
}

void doDisplayResetToSource()
{
    if (resetDisplayToSource)
    {
        if ((millis() - prevResetDisplayToSource) > 3000)
        {
            resetDisplayToSource = false;
            oledDisplaySource();
        }
    }
}

bool changeBtUsbVolLevel(uint8_t volLevelBtUsb)
{
    Wire.beginTransmission(BT_USB_BK82881_I2C_ADDR);
    Wire.write(BT_USB_BK82881_WRITEVOLLEVEL_I2C_SUBADDR);
    Wire.write(volLevelBtUsb);
    return Wire.endTransmission();
    Wire.endTransmission();
}

// void readBtUsbStatus() {
//     Wire.beginTransmission(BT_USB_BK82881_I2C_ADDR);
//     Wire.write(BT_USB_BK82881_READSTATUS_I2C_SUBADDR);
//     Wire.endTransmission(false);
//     //Serial.printf("Iam in line number 423 ofsb_I2c.cpp \n");
//     //delay(3);
//     // Wire.beginTransmission(BT_USB_BK82881_I2C_ADDR);
//     Wire.requestFrom(BT_USB_BK82881_I2C_ADDR, 3);
//     while (Wire.available())
//     {
//         int data = Wire.read();
//         Serial.printf("Data received : %d\n", data);
//     }
//     Serial.printf("Iam in line number 431 ofsb_I2c.cpp \n");
//     // Wire.endTransmission();
//     stopHere();
// }

void getBtDevStatus()
{
    Wire.beginTransmission(BT_USB_BK82881_I2C_ADDR);
    Wire.write(BT_USB_BK82881_READVOLLEVEL_I2C_SUBADDR);
    Wire.endTransmission(false);
    Wire.requestFrom(BT_USB_BK82881_I2C_ADDR, 3);
    uint8_t bufArray[3];
    uint8_t arrayPos = 0;
    while (Wire.available())
    {
        bufArray[arrayPos] = Wire.read();
        arrayPos++;
    }
    if (btDeviceStatus.btPlayerStatus != bufArray[2])
    {
        btDeviceStatus.btPlayerStatus = bufArray[2];
        Serial.printf("BT Play Staus Toggled to %d\n", btDeviceStatus.btPlayerStatus);
    }
    if (btDeviceStatus.bt_Usb_Mode != bufArray[1])
    {
        btDeviceStatus.bt_Usb_Mode = bufArray[1];
        if (btDeviceStatus.bt_Usb_Mode == 0)
        {
            Serial.printf("BT USB Mode changed to BT\n");
        }
        else
        {
            Serial.printf("BT USB Mode changed to USB\n");
        }
    }

    if (btDeviceStatus.btDeviceVolume != bufArray[0])
    {
        btDeviceStatus.btDeviceVolume = bufArray[0];
        Serial.printf("Volume changed to %d\n", btDeviceStatus.btDeviceVolume);
    }
    // Serial.printf("BT Device Volume = %x, Mode = %x, PLAY Status = %x\n", btDeviceStatus.btDeviceVolume, btDeviceStatus.bt_Usb_Mode, btDeviceStatus.btPlayerStatus);
}

bool intilizeBtPcf8574()
{
    Wire.beginTransmission(BT_USB_PCF8574_I2C_ADDR);
    Wire.write(0xFF);
    currIoData = 0xFF;
    if (!Wire.endTransmission(true))
    {
        Serial.println("BT_USB I/O Expander OK");
        return true;
    }
    else
    {
        Serial.println("BT_USB I/O Expander NOT OK");
        return false;
    }
    // stopHere();
}

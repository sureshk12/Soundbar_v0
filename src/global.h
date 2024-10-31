#pragma once
#include <Arduino.h>

// BOARD TYPE
#define ESP32_TYPE WROOM
#define BOARD_VER 56

// PIN ASSIGNMENT
#if ESP32_TYPE == WROOM

#if BOARD_VER == 2
#define PIN_STDBY 2
#define PIN_HDMI_HOTPLUG 18
#define PIN_IR 35
#define PIN_MUX_INT0 26
#define PIN_MUX_INT1 33
#define PIN_CEC_TEST 32
#define PIN_CEC_READ 16
#define PIN_CEC_WRITE 17
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#endif
#if BOARD_VER == 4
#define PIN_KEY_POWER 36
#define KEY_VOLPLUS_PIN 39
#define PIN_KEY_VOLMINUS 34
#define PIN_CEC_TEST 33
#define PIN_IR 25
#define AMP_RESET 14
#define PIN_STDBY 12
#define PIN_CEC_READ 16
#define PIN_CEC_WRITE 17
#define PIN_MUX_INT0 18
#define PIN_MUX_INT1 19
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define PIN_HDMI_HOTPLUG 23
#endif
#if BOARD_VER == 5
#define PIN_KEY_POWER 36
#define KEY_VOLPLUS_PIN 35
#define PIN_KEY_VOLMINUS 32
#define PIN_CEC_TEST 33
#define PIN_IR 25
#define PIN_BT_USB_MODE 14 // Changed to BT_USB_MODE
#define PIN_STDBY 12
#define PIN_CEC_READ 16
#define PIN_CEC_WRITE 17
#define PIN_MUX_INT0 18
#define PIN_MUX_INT1 19
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define PIN_HDMI_HOTPLUG 23
#define PIN_BT_USB_NEXT 26
#define PIN_BT_USB_PREV 27
#define PIN_BT_USB_PLAYPAUSE 4
#endif

#if BOARD_VER == 56
#define PIN_KEY_POWER 36
#define PIN_KEY_VOLPLUS 35
#define PIN_KEY_VOLMINUS 32
#define PIN_CEC_TEST 33
#define PIN_IR 25
#define PIN_BT_USB_NEXT 26
#define PIN_BT_USB_PREV 27
#define PIN_BT_USB_MODE 14 // Changed to BT_USB_MODE
#define PIN_STDBY 12
#define PIN_BT_USB_PLAYPAUSE 4
#define PIN_CEC_READ 16
#define PIN_CEC_WRITE 17
#define PIN_MUX_INT0 18
#define PIN_MUX_INT1 19
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define PIN_HDMI_HOTPLUG 23
#endif

#if BOARD_VER == 6
#define PIN_KEY_POWER 36
#define KEY_VOLPLUS_PIN 35
#define PIN_KEY_VOLMINUS 32
#define PIN_CEC_TEST 33
#define PIN_IR 25
#define PIN_BT_USB_MODE 14 // Changed to BT_USB_MODE
#define PIN_STDBY 12
#define PIN_CEC_READ 16
#define PIN_CEC_WRITE 17
#define PIN_MUX_INT0 18
#define PIN_MUX_INT1 19
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define PIN_HDMI_HOTPLUG 23
#define PIN_BT_USB_NEXT 26
#define PIN_BT_USB_PREV 27
#define PIN_BT_USB_PLAYPAUSE 4
#endif
#endif

#define EVM 1     // Using EVM
#define AMP_EVM 0 // 0 = Actual Board, 1 = EVM

// I2C Address
#define BT_USB_BK82881_I2C_ADDR 0X46
#define BT_USB_BK82881_WRITEVOLLEVEL_I2C_SUBADDR 0xA0
#define BT_USB_BK82881_READVOLLEVEL_I2C_SUBADDR 0xB0
#define BT_USB_BK82881_READSTATUS_I2C_SUBADDR 0XB0
#define BT_USB_PCF8574_I2C_ADDR 0x20
#define PCM9211_I2C_ADDR 0x40
#if AMP_EVM == 1
#define AMP_DSP_I2C_ADDR 0x3D
#endif
#if AMP_EVM == 0
#define AMP_DSP_I2C_ADDR 0x38
#endif

// SB Status
// System Constants
#define NO_OF_LASTSTATUS 5

#define PWR_STDBY HIGH
#define PWR_ON LOW
#define MUTE_ON 0
#define MUTE_OFF 1

// SB INPUT Sources
#define SOURCE_AUX 1
#define SOURCE_BT 2
#define SOURCE_TOSLINK 3
#define SOURCE_HDMI_CEC 4
#define SOURCE_USB 5
#define SOURCE_UNKNOWN 0

// AUDIO MODES
#define MODE_USER 1
#define MODE_MUSIC 2
#define MODE_THEATER 3
#define MODE_NEWS 4
#define MODE_UNKNOWN 0

// ADC VALUES of KEYS
// #define LVL5HI 2888
// #define LVL5LO 2720
// #define LVL4HI 2410
// #define LVL4LO 2123
// #define LVL3HI 1815
// #define LVL3LO 1485
// #define LVL2HI 1078
// #define LVL2LO 809
// #define LVL1HI 503
// #define LVL1LO 354

// AMP DSP CODES
#define AMP_DSP_MUTE_ADDR 0x02
#define AMP_DSP_MUTE_ON_DATA 0X40
#define AMP_DSP_MUTE_OFF_DATA 0X00
#define AMP_DSP_VOLUME_ADDR 0x03

// IR KEY CODES
#define IRCODE_PREVIOUS 3245148407
#define IRCODE_NEXT 3245181047
#define IRCODE_PLAY_PAUSE 3245191247
#define IRCODE_VOLUME_MINUS 3245174927
#define IRCODE_VOLUME_PLUS 3245199407
#define IRCODE_STANDBY 3245191757
#define IRCODE_MUTE 3245195837
#define IRCODE_SOURCE 3245193287
#define IRCODE_MUSIC 3245187677
#define IRCODE_MOVIE 3245203997
#define IRCODE_NEWS 3245175437
#define IRCODE_TREBLE_PLUS 3245168807
#define IRCODE_TREBLE_MINUS 3245163197
#define IRCODE_BASS_PLUS 3245205527
#define IRCODE_BASS_MINUS 3245152487
#define IRCODE_PAIRING 3245160647

// KEY CODES
#define KEYCODE_SOURCE 1
#define KEYCODE_VOLUME_PLUS 2
#define KEYCODE_STANDBY 3
#define KEYCODE_VOLUME_MINUS 4
#define KEYCODE_MUTE 5
#define KEYCODE_PLAY_PAUSE 6
#define KEYCODE_NEXT 7
#define KEYCODE_PREVIOUS 8
#define KEYCODE_MUSIC 9
#define KEYCODE_MOVIE 10
#define KEYCODE_NEWS 11
#define KEYCODE_TREBLE_PLUS 12
#define KEYCODE_TREBLE_MINUS 13
#define KEYCODE_BASS_PLUS 14
#define KEYCODE_BASS_MINUS 15
#define KEYCODE_PAIRING 16

// BT_USB Key Codes
#define BT_USB_RESET 0x04             // HIGH = RESET
#define BT_USB_PLAYPAUSE_PAIRING 0x08 // PLAYPAUSE = SHORT PULSE, PAIRING = LONG PULSE
#define BT_USB_VOLPLUS_NEXT 0x10      // VOLPLUS = SHORT PULSE, NEXT = LONG PULSE
#define BT_USB_VOLMINUS_PREV 0x20     // VOLMINUS = SHORT PULSE, PREV = LONG PULSE
#define BT_USB_MODE 0x40              // BT = LOW, USB = HIGH
#define SHORT_PULSE_DURATION 300      // Duration of shortpulse in mS
#define LONG_PULSE_DURATION 1000      // Duration of longpulse in mS

// CEC CONSTANTS
#define HDMI_HOTPLUG_DETECTION_TIME 100 // HDMI checking time in mS

#define CEC_BUFFER_MAX 10
#define CEC_BUFFER_DATASIZE_MAX 16

#define CEC_START_LOW_MIN 3500
#define CEC_START_LOW_MAX 3900
#define CEC_BITZERO_LOW_MIN 1300
#define CEC_BITZERO_LOW_MAX 1700
#define CEC_BITONE_LOW_MIN 400
#define CEC_BITONE_LOW_MAX 800

#define CEC_BUS_IDLE_INTERVEL 5000
#define CEC_STARTBIT_LOW_TIME 3700
#define CEC_STARTBIT_HIGH_TIME 800
#define CEC_ZERO_LOW_TIME 1500
#define CEC_ZERO_HIGH_TIME 900
#define CEC_ONE_LOW_TIME 600
#define CEC_ONE_HIGH_TIME 1800

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

struct LastDataStruct
{
    uint8_t powerLastStatus;
    uint8_t sourceLastStatus;
    uint8_t volumeLastStatus;
    uint8_t muteLastStatus;
    uint8_t modeLastStatus;
};

struct CecDataStruct
{
    uint8_t cecdata;
    uint8_t cecEom;
    uint8_t cecAck;
};

struct btDeviceStatusStruct
{
    uint8_t btDeviceVolume; // 0 to 16 (0x00 to 0x10)
    uint8_t bt_Usb_Mode;    // 0 = BT, 2 = UDISK
    uint8_t btPlayerStatus; // 0 = BT not Playing, 1 - BT Playing
};

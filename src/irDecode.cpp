#include "Arduino.h"
#include "irDecode.h"
#include "supportFun.h"
#include "global.h"

// Global Variables


// local Variables
static uint8_t prevPinData = HIGH;
static long durationTime = 0;
static long irPrevTime = millis();
static int timeArray[32] = {};
static uint8_t bitCount = 0;
static uint32_t irData = 0;

uint8_t getIrKeyCode(uint32_t irData);
void doIrDecode();

void doIrDecode()
{
    // Remote
    uint8_t pinData = digitalRead(PIN_IR);
    if (pinData == LOW && prevPinData == HIGH)
    {
        durationTime = micros() - irPrevTime;
        irPrevTime = micros();
        prevPinData = LOW;
        timeArray[bitCount] = durationTime;
        bitCount++;
    }

    if (pinData == HIGH && prevPinData == LOW)
    {
        prevPinData = HIGH;
    }

    if (durationTime > 12150 && durationTime < 14850)
    {
        bitCount = 0;
        // startIr = true;
    }

    if (bitCount > 31)
    {
        irData = 0;
        for (int x = 0; x < 32; x++)
        {
            irData = irData << 1;
            if (timeArray[x] > 1634)
            {
                irData = irData ^ 1;
            }
            // Serial.printf("%d, ",timeArray[x]);
            timeArray[x] = 0;
        }

        // Serial.print("IR DATA is = "); Serial.println(irData);
        uint8_t butNum = getIrKeyCode(irData);
        Serial.printf("REMOTE INPUT %d\n", butNum);
        // doKeyIrAction(butNum, &lastValueStruct);
        processFunction(butNum);
        // Serial.println("Returned from dokeyIrAction - REMOTE");
        bitCount = 0;
        // startIr = false;
        prevPinData = HIGH;
    }
}

uint8_t getIrKeyCode(uint32_t irData)
{
    uint8_t butNum;
    switch (irData)
    {
    case IRCODE_PREVIOUS: // PREVIOUS
        butNum = KEYCODE_PREVIOUS;
        break;
    case IRCODE_NEXT: // Next
        butNum = KEYCODE_NEXT;
        break;
    case IRCODE_PLAY_PAUSE: // PLAY / PAUSE
        butNum = KEYCODE_PLAY_PAUSE;
        break;
    case IRCODE_VOLUME_MINUS: // Volume -
        butNum = KEYCODE_VOLUME_MINUS;
        break;
    case IRCODE_VOLUME_PLUS: // Volume +
        butNum = KEYCODE_VOLUME_PLUS;
        break;
    case IRCODE_STANDBY: // STANDBY
        butNum = KEYCODE_STANDBY;
        break;
    case IRCODE_MUTE: // MUTE
        butNum = KEYCODE_MUTE;
        break;
    case IRCODE_SOURCE: // SOURCE/INPUT
        butNum = KEYCODE_SOURCE;
        break;
    case IRCODE_MUSIC: // MUSIC
        butNum = KEYCODE_MUSIC;
        break;
    case IRCODE_MOVIE: // MOVIE
        butNum = KEYCODE_MOVIE;
        break;
    case IRCODE_NEWS: // News
        butNum = KEYCODE_NEWS;
        break;
    case IRCODE_TREBLE_PLUS: // TREBLE +
        butNum = KEYCODE_TREBLE_PLUS;
        break;
    case IRCODE_TREBLE_MINUS: // TREBLE -
        butNum = KEYCODE_TREBLE_MINUS;
        break;
    case IRCODE_BASS_PLUS: // BASS +
        butNum = KEYCODE_BASS_PLUS;
        break;
    case IRCODE_BASS_MINUS: // BASS -
        butNum = KEYCODE_BASS_MINUS;
        break;
    case IRCODE_PAIRING: // PAIRING
        butNum = KEYCODE_PAIRING;
        break;
    default:
        break;
    }
    return butNum;
}

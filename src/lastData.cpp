#include <Arduino.h>
#include <Preferences.h>

#include "lastData.h"
#include "global.h"

// Global Variables
extern uint8_t lastStatusArray[5];

bool createNewLastData(void);
bool getLastData();
bool setLastData();
void printLastStatus();

bool createNewLastData(void)
{
    Preferences preferences;
    preferences.begin("lastStatus", false);
    preferences.clear();
    preferences.putUChar("firstTime", 0x1);
    // preferences.putUChar("powerLastStatus", PWR_ON);
    preferences.putUChar("source", SOURCE_AUX);
    preferences.putUChar("volumelevel", 50);
    // preferences.putUChar("muteStatus", MUTE_OFF);
    preferences.putUChar("mode", MODE_MUSIC);
    preferences.end();
    return true;
}

bool getLastData()
{
    Preferences preferences;
    preferences.begin("lastStatus", true);
    // lastStatusArray[0] = preferences.getUChar("powerLastStatus");
    lastStatusArray[0] = preferences.getUChar("source");
    // Serial.printf("READ lastStatusArray[0] SOURCE is %d\n", lastStatusArray[0]);
    lastStatusArray[1] = preferences.getUChar("volumelevel");
    // Serial.printf("READ lastStatusArray[1] VOLUME is %d\n", lastStatusArray[1]);
    // lastStatusArray[3] = preferences.getUChar("muteStatus");
    lastStatusArray[2] = preferences.getUChar("mode");
    // Serial.printf("READ lastStatusArray[2] MODE is %d\n", lastStatusArray[2]);
    preferences.end();
    return true;
}

bool setLastData()
{
    Preferences preferences;
    preferences.begin("lastStatus", false);
    // preferences.putUChar("powerLastStatus", lastStatusArray[0]);
    preferences.putUChar("source", lastStatusArray[0]);
    // Serial.printf("WROTE lastStatusArray[0] SOURCE is %d\n", lastStatusArray[0]);
    preferences.putUChar("volumelevel", lastStatusArray[1]);
    // Serial.printf("WROTE lastStatusArray[1] VOLUME is %d\n", lastStatusArray[1]);
    // preferences.putUChar("muteStatus", lastStatusArray[3]);
    preferences.putUChar("mode", lastStatusArray[2]);
    // Serial.printf("WROTE lastStatusArray[2] MODE is %d\n", lastStatusArray[2]);
    preferences.end();
    return true;
}

// ONLY TO DEBUG
void printLastStatus()
{
    Serial.printf("source = %d, volume = %d, AudioMode = %d\n", lastStatusArray[0], lastStatusArray[1], lastStatusArray[2]);
}
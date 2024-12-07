#include <Arduino.h>
// #include "BluetoothA2DPSink.h"

#include "modeBT.h"
#include "sb_I2c.h"
#include "global.h"

// Defination
// BluetoothA2DPSink a2dp_sink;

// Global variables
extern uint8_t lastStatusArray[NO_OF_LASTSTATUS];

static uint8_t btReConnectCount = 3;
static long btReConnectPrevTime = millis();

// File variables
bool playPass = false; // Playing


/*

// Prototypes
bool btModeBegin();
void doKeyBTAction(uint8_t actionNum);
void btReConnect();
void btDisconnect();
void setBtConnectFlagToFalse();
void doBtTask();



void avrc_metadata_callback(uint8_t id, const uint8_t *text)
{
    // Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
}

// a2dp_sink.set_connected(connected);

void setBtConnectFlagToFalse()
{
    btConnected = false;
}

void btReConnect()
{

    if (!btConnected)
    {
        // Serial.println("BT not Connected will connect");
        a2dp_sink.set_connected(true);
        btConnected = true;
        btReConnectCount = 3;
        btReConnectPrevTime = millis();

        // delay(500);
        // a2dp_sink.play();
        // delay(500);
        // a2dp_sink.play();
        // delay(500);
        // a2dp_sink.play();
        playPass = true;
    }
    else
    {
        // Serial.println("Already connected , no action");
    }
}

void btDisconnect()
{
    if (btConnected)
    {
        Serial.println("BT Connected will Disconnect");
        a2dp_sink.set_connected(false);
        btConnected = false;
    }
    else
    {
        Serial.println("BT Disconnecte still will Disconnect");
        a2dp_sink.set_connected(false);
    }
}

void doKeyBTAction(uint8_t actionNum)
{
    // Serial.printf("BT ACTION No %d\n", actionNum);
    if (actionNum == KEYCODE_PLAY_PAUSE)
    { // PLAY / PAUSE
        // Play / Pause
        if (playPass)
        {
            // Pause
            Serial.println("pause");
            a2dp_sink.pause();
            playPass = false;
        }
        else
        {
            // play
            Serial.println("play");
            a2dp_sink.play();
            playPass = true;
        }
    }
    else if (actionNum == KEYCODE_PREVIOUS)
    { // PREVIOUS
        Serial.println("Previous");
        a2dp_sink.previous();
    }
    else if (actionNum == KEYCODE_NEXT)
    { // NEXT
        Serial.println("Next");
        a2dp_sink.next();
    }
    else if (actionNum == KEYCODE_VOLUME_MINUS)
    { // VOLUME MINUS
        // Serial.println("Vol - BT");
        uint8_t volValue = a2dp_sink.get_volume();
        if (lastStatusArray[1] < (volValue / 4))
        {            
            if (volValue >  4)
            {
                volValue = volValue - 4;
                Serial.println("Vol - BT");
            }
            a2dp_sink.set_volume(volValue);
        }
        else
        {
            lastStatusArray[1] = lastStatusArray[1] - 1;
            if (lastStatusArray[1] > 0)
            {
                lastStatusArray[1] = lastStatusArray[1] - 1;
                Serial.println("Vol - SB");
            }
            changeDspVolume(lastStatusArray[1]);
        }
        
        // Serial.printf("Volume level = %d\n", volValue );
    }
    else if (actionNum == KEYCODE_VOLUME_PLUS)
    { // VOLUME PLUS
        // Serial.println("Vol + BT");
        uint8_t volValue = a2dp_sink.get_volume();
        if (lastStatusArray[1] > (volValue / 4))
        {
            volValue = volValue + 4;
            Serial.println("Vol + BT");
            if (volValue > 127)
            {
                volValue = 127;
            }
            a2dp_sink.set_volume(volValue);            
        }
        else
        {
            lastStatusArray[1] = lastStatusArray[1] + 1;
            Serial.println("Vol + SB");
            if (lastStatusArray[1] > 32)
            {
                lastStatusArray[1] = 32;
            }
            changeDspVolume(lastStatusArray[1]);
        }

        // Serial.printf("Volume level = %d\n", volValue );
    }
    //}
}

bool btModeBegin()
{
    // Start Blue tooth
    i2s_pin_config_t my_pin_config = {
        .mck_io_num = 0, // was I2S_PIN_NO_CHANGE
        .bck_io_num = 4,
        .ws_io_num = 5,
        .data_out_num = 25,
        .data_in_num = I2S_PIN_NO_CHANGE};
    a2dp_sink.set_pin_config(my_pin_config);
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.set_auto_reconnect(true, 1000);
    // a2dp_sink.set_bits_per_sample(24);
    a2dp_sink.start("MyMusic");
    delay(1000);
    a2dp_sink.play();
    delay(1000);
    a2dp_sink.play();
    delay(1000);
    a2dp_sink.play();

    btConnected = true;
    btReConnectCount = 3;
    btReConnectPrevTime = millis();

    playPass = true;
    return true;
}

void doBtTask()
{
    if (btReConnectCount > 0 && btConnected)
    {
        if ((millis() - btReConnectPrevTime) > 1000)
        {
            btReConnectCount--;
            btReConnectPrevTime = millis();
            a2dp_sink.play();
            //Serial.printf("BTCount %d\n", btReConnectCount);
        }
    }
}
*/
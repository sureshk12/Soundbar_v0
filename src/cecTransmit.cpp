#include <Arduino.h>
#include "cecTransmit.h"
#include "cecReceive.h"
#include "supportFun.h"
#include "global.h"

// Global Variables SB State
extern uint8_t sb_Volume;
extern uint8_t sb_power;
uint8_t tv_Audio_Status_Volume_Mute = 0;
bool tv_Power_Status = false;
bool tvArc = false;

extern bool cecRxFlag;
bool cecTxFlag = false;
bool startedToSendData = false;
bool cecDataSemafore = false;
bool cecTxDisableErrChk = true;
uint8_t cecTxErrorFlag = 0;
bool cecTxReady = false;
uint8_t cecTxErrorCount = 0;

// Local variables
static uint8_t cecTxAllDataBuffer[CEC_BUFFER_MAX][CEC_BUFFER_DATASIZE_MAX];
static uint8_t cecTxAlldataBufferLength[CEC_BUFFER_MAX];
static uint8_t cecTxAllDataCount;

static uint8_t cecTxDataBuffer[CEC_BUFFER_DATASIZE_MAX];
static uint8_t cecTxDataSize = 0;
static uint8_t cecTxStatus = 0;
static long cecTxPrevTime = micros();
static uint8_t cecTxBitCount = 0;
static uint8_t cecTxBytePos = 0;
static bool cecTxIsBitLow = true;
static uint8_t andBitPosArray[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
static uint8_t txDataArray[CEC_BUFFER_DATASIZE_MAX];
static uint8_t txDataLength = cecTxAlldataBufferLength[0];
static bool eomTxFlag = false;
static bool ackTxReceived = false;
static int ackTxReceivedCount = 0;
static long ackHighTime = 0;
static int cecTxErrorIntervel = 100;
static long cecTxErrorPrevTime = millis();

// Prototypes
void doAutoReply();
void cecTxData(); // Used Locally to send Tx
void doCecTxTask();
void cecTxSelfAddress();
void report_sbOsdName();                    // reports Soundbar Name
void set_tvSystemAudioMode(uint8_t on_off); // Requests TV to Set System audio mode
void report_sbVolumeMuteStatus();           // report Volume level and Mute ststus to TV
void report_sbSystemAudioModeStatus();      // report SB SystemAudioModeStatus
void get_TvPowerStatus();                   // Get TV Power status
void report_sbPowerStatus();                // report SB power status
void report_sbShortAudioDescriptor();       // report SB short audio descriptor
void set_tvArcOn();                         // Request TV to switch on ARC
void set_tvArcTermination();                // Request TV to terminate ARC
void broadcast_oneByte();
void broadcast_oneByteNew();
void broadcast_sbSystemAudioMode(); // Broadcast SB Syfetem Audio Mode
void broadcast_sbPhysicalAddress(); // Broadcast SB Physical address
void broadcast_sbDeviceVendorId();  // Broadcast SB Device ID
void reduceCecTxAllDataCount();
// void generateDebugPulse();
void serialPrintTxBuffer();

void cecTxSelfAddress()
{
    cecTxDataBuffer[0] = 0x55;
    cecTxDataSize = 1;
    cecTxData();
}
void report_sbOsdName()
{
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0x47;  // Opcode
    cecTxDataBuffer[2] = 0x53;  //"S"
    cecTxDataBuffer[3] = 0x6F;  //"o"
    cecTxDataBuffer[4] = 0x75;  //"u"
    cecTxDataBuffer[5] = 0x6E;  //"n"
    cecTxDataBuffer[6] = 0x64;  //"d"
    cecTxDataBuffer[7] = 0x42;  //"B"
    cecTxDataBuffer[8] = 0x61;  //"a"
    cecTxDataBuffer[9] = 0x72;  //"r"
    cecTxDataBuffer[10] = 0x00; //"\0"
    cecTxDataSize = 11;
    cecTxData();
}

void set_tvSystemAudioMode(uint8_t on_off)
{
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0x72; // Opcode
    cecTxDataBuffer[2] = on_off;
    cecTxDataSize = 3;
    cecTxData();
}

void report_sbVolumeMuteStatus()
{
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0x7A;

    // if (sb_Mute == MUTE_ON)
    // {
    //     cecTxDataBuffer[2] = sb_Volume & 0x8F;
    //     Serial.printf("sb_volume = %d\n", sb_Volume);
    // }
    // else
    // {
    //     cecTxDataBuffer[2] = sb_Volume;
    // }
    cecTxDataBuffer[2] = sb_Volume;
    cecTxDataSize = 3;
    cecTxData();
    Serial.printf("sb_volume = %d\n", sb_Volume);
}

void report_sbSystemAudioModeStatus()
{ // report SB SystemAudioModeStatus
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0x7E;
    cecTxDataBuffer[2] = 0x01;
    cecTxDataSize = 3;
    cecTxData();
}

void get_TvPowerStatus()
{
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0x8F;
    cecTxDataSize = 2;
    cecTxData();
}

void report_sbPowerStatus()
{
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0x90;
    if (sb_power == PWR_ON)
    {
        cecTxDataBuffer[2] = 0x00;
    }
    else
    {
        cecTxDataBuffer[2] = 0x01;
    }
    cecTxDataSize = 3;
    cecTxData();
}

void report_sbShortAudioDescriptor()
{
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0xA3;
    cecTxDataBuffer[2] = 0x00;
    cecTxDataSize = 3;
    cecTxData();
}

void set_tvArcOn()
{
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0xC0;
    cecTxDataSize = 2;
    cecTxData();
}

void set_tvArcTermination()
{
    cecTxDataBuffer[0] = 0x50;
    cecTxDataBuffer[1] = 0xC5;
    cecTxDataSize = 2;
    cecTxData();
}

void broadcast_oneByte()
{
    cecTxDataBuffer[0] = 0x5F;
    cecTxDataSize = 1;
    cecTxData();
}

void broadcast_oneByteNew()
{
    cecTxDataBuffer[0] = 0x5F;
    cecTxDataSize = 1;
    cecTxData();
}

void broadcast_sbSystemAudioMode()
{
    cecTxDataBuffer[0] = 0x5F;
    cecTxDataBuffer[1] = 0x72;
    cecTxDataBuffer[2] = 0x00;
    cecTxDataSize = 3;
    cecTxData();
}
void broadcast_sbPhysicalAddress()
{
    // Serial.printf("[148]");
    cecTxDataBuffer[0] = 0x5F;
    cecTxDataBuffer[1] = 0x84;
    cecTxDataBuffer[2] = 0x10;
    cecTxDataBuffer[3] = 0x00;
    cecTxDataBuffer[4] = 0x05;
    cecTxDataSize = 5;
    cecTxData();
}
void broadcast_sbDeviceVendorId()
{
    cecTxDataBuffer[0] = 0x5F;
    cecTxDataBuffer[1] = 0x87;
    cecTxDataBuffer[2] = 0x52;
    cecTxDataBuffer[3] = 0x53;
    cecTxDataBuffer[4] = 0x52;
    cecTxDataSize = 5;
    cecTxData();
}

void doCecTxTask()
{
    // Serial.println("[206]");
    // generateDebugPulse();
    if (cecTxFlag && cecTxErrorFlag == 0)
    {
        // Serial.println("[209]");

        if (cecTxAllDataCount > 0 && !startedToSendData)
        {
            // get data to send
            // Serial.println("[212]");
            txDataLength = cecTxAlldataBufferLength[0];
            for (int x = 0; x < txDataLength; x++)
            {
                txDataArray[x] = cecTxAllDataBuffer[0][x];
            }
            startedToSendData = true;
            cecTxStatus = 0;

            // cecTxErrorFlag = 0;
            //  generateDebugPulse();
        }

        if (cecTxStatus == 0)
        {
            // Take Control of CEC Bus, if bus Busy return
            // Serial.println("[234]");
            if (digitalRead(PIN_CEC_READ) == HIGH)
            {
                // Serial.println("[224]");
                cecRxFlag = false; // DISABLE RX
                cecTxStatus = 1;
                cecTxPrevTime = micros();
                // Serial.printf("cecTxStatus = %d\n", cecTxStatus);
            }
            else
            {
                // ERROR CEC Bus Low
                // Serial.println("[231]");
                cecTxErrorFlag = 1;

                // return;
            }
            return;
        }

        if (cecTxStatus == 1)
        {
            // Serial.println("[253]");

            if ((micros() - cecTxPrevTime) > CEC_BUS_IDLE_INTERVEL)
            {
                // Bus aquired
                // Serial.println("[257]");
                cecTxPrevTime = micros();
                cecTxStatus = 2;
                // generateDebugPulse();
            }
            else
            {
                // Serial.println("[264]");
                if (digitalRead(PIN_CEC_READ) == LOW)
                {
                    // Bus not free
                    // generateDebugPulse();
                    Serial.println("[275]");
                    cecTxPrevTime = micros();
                    cecTxStatus = 0;
                    cecTxErrorFlag = 2; // Error 2: Bus Not free
                                        // return;
                }
            }
            return;
        }

        if (cecTxStatus == 2)
        {
            // Serial.println("[267]");
            // Start start Low
            cecTxPrevTime = micros();
            digitalWrite(PIN_CEC_WRITE, HIGH);
            cecTxStatus = 3;

            // generateDebugPulse();
            return;
        }

        if (cecTxStatus == 3)
        {
            // Serial.println("[279]");
            if ((micros() - cecTxPrevTime) > CEC_STARTBIT_LOW_TIME)
            {
                // Start bit High
                digitalWrite(PIN_CEC_WRITE, LOW);
                cecTxPrevTime = micros();
                cecTxStatus = 4;
                // generateDebugPulse();
                // Serial.println("[279]");
                // while(1) {}
            }
            return;
        }

        if (cecTxStatus == 4)
        {
            // Serial.println("[295]");

            if ((micros() - cecTxPrevTime) > CEC_STARTBIT_HIGH_TIME) // CEC_STARTBIT_HIGH_TIME

            {
                // Start bit completed
                digitalWrite(PIN_CEC_WRITE, HIGH); // start of Bit 7
                cecTxPrevTime = micros();
                cecTxStatus = 5;
                cecTxBitCount = 0;
                // generateDebugPulse();
            }
            else
            {
                int intervelTime = micros() - cecTxPrevTime;
                if ((intervelTime > 10) && (intervelTime < (CEC_STARTBIT_HIGH_TIME - 10)))
                {
                    if (digitalRead(PIN_CEC_READ) == LOW)
                    {
                        // Bus not free
                        // Serial.println("[215]");
                        cecTxPrevTime = micros();
                        cecTxStatus = 0;
                        cecTxErrorFlag = 3; // Error 3: Start not sucessful
                                            // return;
                    }
                }
            }
            return;
        }

        if (cecTxStatus == 5)
        {
            // Serial.println("[328]");
            cecTxBitCount = 0;
            cecTxBytePos = 0;
            cecTxIsBitLow = true;
            cecTxPrevTime = micros();
            cecTxStatus = 6;
            return;
        }

        if (cecTxStatus == 6)
        {
            // Serial.println("[337]");
            int lowTime = 0;
            int highTime = 0;

            if (cecTxBitCount < 8)
            {
                if ((txDataArray[cecTxBytePos] & andBitPosArray[cecTxBitCount]) == 0)
                {
                    lowTime = CEC_ZERO_LOW_TIME;
                    highTime = CEC_ZERO_HIGH_TIME;
                }
                else
                {
                    lowTime = CEC_ONE_LOW_TIME;
                    highTime = CEC_ONE_HIGH_TIME;
                }
            }
            if (cecTxBitCount == 8)
            {
                if (cecTxBytePos < (txDataLength - 1))
                {
                    lowTime = CEC_ZERO_LOW_TIME;
                    highTime = CEC_ZERO_HIGH_TIME;
                }
                else
                {
                    lowTime = CEC_ONE_LOW_TIME;
                    highTime = CEC_ONE_HIGH_TIME;
                    eomTxFlag = true;
                }
            }
            if (cecTxBitCount == 9)
            {
                lowTime = CEC_ONE_LOW_TIME;
                highTime = CEC_ONE_HIGH_TIME;
                ackHighTime = CEC_ONE_HIGH_TIME;
            }

            if (cecTxIsBitLow)
            {
                if ((micros() - cecTxPrevTime) > lowTime)
                {
                    digitalWrite(PIN_CEC_WRITE, LOW);
                    cecTxIsBitLow = false;
                    cecTxPrevTime = micros();
                    ackTxReceived = false;
                    ackTxReceivedCount = 0;
                }
            }
            else
            {
                // To do check Acknowledge
                // Mark End of Low period of ack
                if (cecTxBitCount == 9)
                {
                    // digitalWrite(PIN_CEC_TEST, LOW);
                    // delayMicroseconds(20);
                    // digitalWrite(PIN_CEC_TEST, HIGH);
                }
                int intervelTime = micros() - cecTxPrevTime;
                if (intervelTime > highTime)
                {
                    if (eomTxFlag && cecTxBitCount == 9)
                    {
                        digitalWrite(PIN_CEC_WRITE, LOW);
                    }
                    else
                    {
                        digitalWrite(PIN_CEC_WRITE, HIGH);
                    }
                    cecTxIsBitLow = true;
                    cecTxPrevTime = micros();
                    cecTxBitCount++;
                }
                if ((intervelTime > 10) && (intervelTime < (highTime - 10)))
                {
                    if (cecTxBitCount < 8)
                    {
                        if (digitalRead(PIN_CEC_READ) == LOW)
                        {
                            // generateDebugPulse();
                            //  Serial.println("ERROR: 330");
                            cecTxErrorFlag = 4; // error in sending databits.
                        }
                    }

                    if (cecTxBitCount == 9)
                    {
                        if (digitalRead(PIN_CEC_READ) == LOW)
                        {
                            // generateDebugPulse();
                            ackTxReceivedCount++;
                            if (ackTxReceivedCount > 1)
                            {
                                ackTxReceived = true;

                                if (ackTxReceivedCount == 3)
                                {
                                    // Serial.println(ackTxReceived ? "ACK RECEIVED" : "NO ACK");
                                }
                            }
                        }
                    }
                }
            }

            if (cecTxBitCount > 9)
            {
                if (((txDataArray[0] & 0x0F) == 0x0F) || (txDataArray[0] == 0x55))
                {
                    ackTxReceived = true;
                }
                if (!ackTxReceived)
                {

                    cecTxErrorFlag = 5; // No acknowldeg received
                    cecTxBytePos = 0;
                    cecTxStatus = 0;
                    eomTxFlag = false;
                    cecDataSemafore = false;
                    startedToSendData = false;
                    digitalWrite(PIN_CEC_WRITE, LOW);
                    if (!cecTxDisableErrChk)
                    {
                        reduceCecTxAllDataCount();
                        cecRxFlag = true;
                        Serial.print("Tried sending Tx Data : ");
                        for (uint8_t x = 0; x < txDataLength; x++)
                        {
                            Serial.printf("%02x, ", txDataArray[x]);
                        }
                        Serial.println("but No Acknowldgement");
                        cecTxReady = true;
                    }
                }
                else
                {
                    if (cecTxBytePos < (txDataLength - 1))
                    {
                        cecTxBytePos++;
                        cecTxBitCount = 0;
                    }
                    else
                    {
                        cecTxStatus = 0;
                        digitalWrite(PIN_CEC_WRITE, LOW);

                        eomTxFlag = false;
                        Serial.print("Tx Data : ");
                        for (uint8_t x = 0; x < txDataLength; x++)
                        {
                            Serial.printf("%02x, ", txDataArray[x]);
                        }
                        Serial.println();

                        reduceCecTxAllDataCount();
                        cecRxFlag = true;
                        cecTxFlag = false;
                        startedToSendData = false;
                        cecTxErrorFlag = 0;
                        cecTxReady = true;
                    }
                }
            }
            return;
        }
    }

    if (cecTxErrorFlag > 0 && cecTxDisableErrChk)
    {
        // ERROR MANAGEMENT
        // generateDebugPulse();
        // Serial.printf("[522] cecTxErrorCount %d\n", cecTxErrorCount);
        if (cecTxErrorCount == 0)
        {
            // Serial.printf("[525] ERROR %d\n", cecTxErrorFlag);
            cecTxFlag = false;
            cecTxErrorPrevTime = millis();
            cecTxErrorIntervel = 25;
            cecTxErrorCount++;
        }

        if ((millis() - cecTxErrorPrevTime) > cecTxErrorIntervel)
        {
            cecTxErrorCount++;
            // Serial.printf("[534]");

            if (cecTxErrorCount > 5)
            {
                // cecTxErrorCount = 0;
                reduceCecTxAllDataCount();
                cecRxFlag = true;
                cecTxFlag = false;
                startedToSendData = false;
                // cecTxErrorFlag = 0;
                cecTxReady = true;
            }
            else
            {

                switch (cecTxErrorFlag)
                {
                case 1:
                    Serial.println("CEC BUS LOW");
                    cecTxErrorIntervel = 1000;
                    break;
                case 2:
                    Serial.println("BUS Not Free");
                    cecTxErrorIntervel = 1000;
                    break;
                case 3:
                    Serial.println("Start Bit error");
                    cecTxErrorIntervel = 100 * cecTxErrorCount;
                    break;
                case 4:
                    Serial.println("data could not be sent");
                    cecTxErrorIntervel = 100 * cecTxErrorCount;
                    break;
                case 5:
                    Serial.println("No Acknowldgement");
                    cecTxErrorIntervel = 50; //* cecTxErrorCount;
                    break;
                default:
                    Serial.println("Unknown error in CEC Transmit!!");
                    break;
                }

                // cecRxFlag = true;
                // cecTxFlag = false;
                // startedToSendData = false;
                // cecTxErrorFlag = 0;
                // cecTxReady = true;

                cecTxFlag = true;
                cecTxErrorFlag = 0;
                cecTxErrorPrevTime = millis();
            }
        }
    }
}

void cecTxData()
{
    // Serial.println("[570]");
    cecDataSemafore = true;
    cecTxAlldataBufferLength[cecTxAllDataCount] = cecTxDataSize;
    for (uint8_t x = 0; x < cecTxDataSize; x++)
    {
        cecTxAllDataBuffer[cecTxAllDataCount][x] = cecTxDataBuffer[x];
    }
    cecTxAllDataCount++;
    if (cecTxAllDataCount > 10)
    {
        cecTxAllDataCount = 10;
    }

    cecTxReady = false;
    cecTxErrorFlag = 0;
    cecDataSemafore = false;
    cecTxFlag = true;

    // serialPrintTxBuffer();
    // generateDebugPulse();
}

uint8_t cecRxDataCount = 0;
uint8_t cecRxDataBuffer[CEC_BUFFER_DATASIZE_MAX] = {};

void doAutoReply()
{
    if (cecRxIsDataAvaialble() > 0)
    {

        cecRxGetdata(cecRxDataBuffer, &cecRxDataCount);
        if (cecRxDataCount > 1)
        {
            // Serial.printf("Start of Loop cecRxDataCount = %d\n", cecRxDataCount);
            if(cecRxDataBuffer[1] == 0x44)
            {
                if(cecRxDataBuffer[2] == 0x41) {
                    printf("Volume plus pressed\n");
                    processFunction(KEYCODE_VOLUME_PLUS);
                } else if (cecRxDataBuffer[2] == 0x42)
                {
                    printf("Volume minus pressed\n");
                    processFunction(KEYCODE_VOLUME_MINUS);
                }
                else if (cecRxDataBuffer[2] == 0x43)
                {
                    printf("Mute key pressed\n");
                    processFunction(KEYCODE_MUTE);
                } else 
                {
                    //Do nothing
                }
            }
            else if (cecRxDataBuffer[1] == 0x46)
            {
                report_sbOsdName(); //<Give OSD Name> Request OSD name (from SB)
            }
            else if (cecRxDataBuffer[1] == 0x71)
            {
                report_sbVolumeMuteStatus(); //<Give Audio Status> Request Audio status (from SB)
            }
            else if (cecRxDataBuffer[1] == 0x7A)
            {
                tv_Audio_Status_Volume_Mute = cecRxDataBuffer[2]; //<Report audio status> reported TV Volume and Mute status
            }
            else if (cecRxDataBuffer[1] == 0x7D)
            {
                report_sbSystemAudioModeStatus(); //<Give System Audio Mode Status> request from SB
            }
            else if (cecRxDataBuffer[1] == 0x83)
            {
                broadcast_sbPhysicalAddress();
                // Serial.println("[110]");						//<Give Physical Address> SB to broadcast reply to 0x5F
            }
            else if (cecRxDataBuffer[1] == 0x8C)
            {
                broadcast_sbDeviceVendorId(); //<Give Vendor Device Id> SB to broadcast reply to 0x5F
            }
            else if (cecRxDataBuffer[1] == 0x8F)
            {
                report_sbPowerStatus(); //<Give Device Power Status> request power status (from SB)
            }
            else if (cecRxDataBuffer[1] == 0x90)
            {
                // tv_Power_Status
                if (cecRxDataBuffer[2] == 0x00) //<Report power status> 00 TV is ON 01 TV is StandBy (To SB)
                {
                    tv_Power_Status = true; // TV is ON
                    Serial.println("TV ON");
                }
                else
                {
                    tv_Power_Status = false; // TV is OFF
                    Serial.println("[653]TV OFF");
                }
            }
            else if (cecRxDataBuffer[1] == 0xA4)
            {
                report_sbShortAudioDescriptor(); //<Request short audio Descriptor>
            }
            else if (cecRxDataBuffer[1] == 0xC1)
            {
                tvArc = true; // ARC enabled on TV
            }
            else if (cecRxDataBuffer[1] == 0xC2)
            {
                tvArc = false; // ARC DISABLED on TV
            }
            else if (cecRxDataBuffer[1] == 0xC3)
            {
                set_tvArcOn(); //<Request ARC intitation> Tv requests SB to send ARC intialization request
            }
            else if (cecRxDataBuffer[1] == 0xC4)
            {
                set_tvArcTermination(); //<request ARC Termination> Tv requests SB to send ARC Termination request
            }
            else
            {
                // Do Nothing
            }
        }
    }
}

// Support functions

void serialPrintTxBuffer()
{
    for (int x = 0; x < cecTxAllDataCount; x++)
    {
        Serial.printf("Data Array [%d] : ", x);
        for (int y = 0; y < cecTxAlldataBufferLength[x]; y++)
        {
            Serial.printf("%02x", cecTxAllDataBuffer[x][y]);
        }
        Serial.println();
    }
}

void reduceCecTxAllDataCount()
{
    cecDataSemafore = true;
    for (int x = 0; x < cecTxAllDataCount; x++)
    {
        for (int y = 0; y < cecTxAlldataBufferLength[x]; y++)
        {
            cecTxAllDataBuffer[x][y] = cecTxAllDataBuffer[x + 1][y];
        }
        cecTxAlldataBufferLength[x] = cecTxAlldataBufferLength[x + 1];
    }
    cecTxAllDataCount--;
    if (cecTxAllDataCount < 0)
    {
        cecTxAllDataCount = 0;
    }
    cecDataSemafore = false;
    // if(cecTxErrorFlag > 0) {
    // 	cecTxReady = true;
    // }
}

// void generateDebugPulse()
// {
// 	digitalWrite(PIN_CEC_TEST, LOW);
// 	delayMicroseconds(20);
// 	digitalWrite(PIN_CEC_TEST, HIGH);
// }

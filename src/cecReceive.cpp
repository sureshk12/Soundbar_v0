#include <Arduino.h>
#include "global.h"
#include "cecReceive.h"
#include "cecTransmit.h"
#include "supportFun.h"


// Global Variables
extern bool isCecBusOk;
extern bool isTvCecOn;
extern bool isTvOn;
extern bool sb_Mute;
extern uint8_t sb_Volume;
extern uint8_t sb_power;
extern uint8_t tv_Audio_Status_Volume_Mute;
extern bool tv_Power_Status;
extern bool tvArc;

extern bool cecRxFlag;
extern bool cecTxFlag;
extern bool cecDataSemafore;
extern bool cecTxDisableErrChk;
extern uint8_t cecTxErrorFlag;
extern bool doNotAck;
extern bool stopKeyHDMI;

// Local Variables
static bool cecRxDataSemafore;
static uint8_t cecRxAllDataBuffer[CEC_BUFFER_MAX][CEC_BUFFER_DATASIZE_MAX];
static uint8_t cecRxAlldataBufferLength[CEC_BUFFER_MAX];
static uint8_t cecRxAllDataCount;

static long cecRxPrevTime = micros();
static uint8_t cecRxPrevState = HIGH;
static uint8_t cecRxCurrBitValue = 0;
static uint8_t cecRxCurrByteBitPos = 0;
static uint8_t cecRxCurrByteData = 0;
static bool cecRxInBit8Pos = false;
static bool cecRxInBit9Pos = false;
static bool cecRxInBit10Pos = false;
static uint8_t cecRxByteCount = 0;
static uint8_t cecRxByteArray[CEC_BUFFER_DATASIZE_MAX] = {};
static bool cecRxPleaseAck = false;
static bool cecRxAckStartLow = false;
static long cecRxAckTimeIntervel = micros();
static bool cecRxEomFlag = false;

// Prototypes
void intilizeCecReceive();
uint8_t cecRxIsDataAvaialble();
uint8_t cecRxGetdata(uint8_t *receiveData, uint8_t *receiveDataLength);
void doStandardreply();

// Functions
void intilizeCecReceive()
{
}

uint8_t cecRxIsDataAvaialble()
{
	return cecRxAllDataCount;
}

uint8_t cecRxGetdata(uint8_t *receiveData, uint8_t *receiveDataLength)
{

	if (cecRxDataSemafore)
	{
		return 0;
	}

	if (cecRxAllDataCount > 0)
	{
		// Serial.printf("cecRxAllDataCount %d \n", cecRxAllDataCount);
		for (uint8_t x = 0; x < cecRxAlldataBufferLength[0]; x++)
		{
			receiveData[x] = cecRxAllDataBuffer[0][x];
		}
		*receiveDataLength = cecRxAlldataBufferLength[0];

		// Delete data 0 and move all data by count 1
		for (uint8_t x = 0; x < (cecRxAllDataCount - 1); x++)
		{
			for (uint8_t y = 0; y < cecRxAlldataBufferLength[x + 1]; y++)
			{
				cecRxAllDataBuffer[x][y] = cecRxAllDataBuffer[x + 1][y];
			}
			cecRxAlldataBufferLength[x] = cecRxAlldataBufferLength[x + 1];
		}
		cecRxAllDataCount--;
		return cecRxAllDataCount + 1;
	}
	else
	{
		return 0;
	}
}

void doCecReadTask()
{
	if (cecRxFlag)
	{
		// generateDebugPulse();
		//Serial.println("[Rx:103]");
		uint8_t cecRxCurrPinState = digitalRead(CEC_READ_PIN);

		// if((cecRxCurrPinState == HIGH) && (cecRxPrevState == HIGH) && ((micros() - cecRxPrevTime) > 4000)) {
		// 	//IDLE Sate
		// 	cecRxFlag = false;
		// 	return;
		// }

		if ((cecRxCurrPinState == LOW) and (cecRxPrevState == HIGH))
		{
			//generateDebugPulse();
			if (cecRxPleaseAck &&cecRxCurrByteBitPos == 9 && !doNotAck)
			{
				//Serial.println(cecRxAckStartLow ? "cecRxAckStartLow = TRUE" : "cecRxAckStartLow = FALSE");
				if (!cecRxAckStartLow)
				{
					// generateDebugPulse();

					digitalWrite(CEC_WRITE_PIN, HIGH);
					cecRxAckStartLow = true;
					cecRxAckTimeIntervel = micros();
				}
			}
			cecRxPrevTime = micros();
			cecRxPrevState = LOW;
		}

		if (cecRxAckStartLow)
		{
			if ((micros() - cecRxAckTimeIntervel) > 1400)
			{ // was 1500
				digitalWrite(CEC_WRITE_PIN, LOW);
				cecRxAckStartLow = false;
				cecRxPleaseAck = false;
			}
		}

		if ((cecRxCurrPinState == HIGH) and (cecRxPrevState == LOW))
		{
			// generateDebugPulse();
			// LOW TO HIGH
			long lowTime = micros() - cecRxPrevTime;
			cecRxPrevTime = micros();
			cecRxPrevState = HIGH;

			if (lowTime > CEC_START_LOW_MIN && lowTime < CEC_START_LOW_MAX)
			{
				// START BIT
				//generateDebugPulse(30);
				cecRxCurrByteBitPos = 0;
				cecRxCurrByteData = 0;
				cecRxByteCount = 0;
				cecRxFlag = true;
				stopKeyHDMI = true;
			}
			else if (lowTime > CEC_BITZERO_LOW_MIN && lowTime < CEC_BITZERO_LOW_MAX)
			{
				// BIT LOW
				cecRxCurrBitValue = 0x00;
				cecRxCurrByteData = cecRxCurrByteData << 1;
				cecRxCurrByteData = cecRxCurrByteData | 0;
				cecRxCurrByteBitPos++;
				// if (cecRxCurrByteBitPos == 1)
				// {
				// 	cecTxFlag = false;
				// }
			}
			else if (lowTime > CEC_BITONE_LOW_MIN && lowTime < CEC_BITONE_LOW_MAX)
			{
				// BIT HIGH
				cecRxCurrBitValue = 0x01;
				cecRxCurrByteData = cecRxCurrByteData << 1;
				cecRxCurrByteData = cecRxCurrByteData | 1;
				cecRxCurrByteBitPos++;
				// if (cecRxCurrByteBitPos == 1)
				// {
				// 	cecTxFlag = false;
				// }
			}
		}

		if (cecRxCurrByteBitPos == 8)
		{
			if (!cecRxInBit8Pos)
			{

				cecRxInBit8Pos = true;
				cecRxByteArray[cecRxByteCount] = cecRxCurrByteData;
				// Serial.printf("Data : %02x\n",  cecRxCurrByteData);
				cecRxByteCount++;

				if ((cecRxByteArray[0] & 0x0F) == 0x05)
				{
					cecRxPleaseAck = true;
				}
				else
				{
					cecRxPleaseAck = false;
				}

				if ((cecRxByteArray[0] & 0xF0) == 0x00)
				{
					isTvCecOn = true;
				}
				if ((cecRxByteArray[0] & 0x05) == 0x05)
				{
					isTvOn = true;
				}
			}
		}
		if (cecRxCurrByteBitPos == 9)
		{
			cecRxInBit8Pos = false;
			if (!cecRxInBit9Pos)
			{
				if (cecRxCurrBitValue == 1)
				{
					cecRxEomFlag = true;
				}
				cecRxInBit9Pos = true;
				cecRxInBit10Pos = false;
			}
		}
		static long cecRxAuxHighPrevTime = micros();
		if (cecRxCurrByteBitPos == 10)
		{
			cecRxAuxHighPrevTime = micros();
			cecRxCurrByteBitPos = 11;
		}

		if (cecRxCurrByteBitPos == 11)
		{
			
			if((micros() - cecRxAuxHighPrevTime) > 1000) {
				//generateDebugPulse(50);
				cecRxInBit9Pos = false;
				if (!cecRxInBit10Pos)
				{

					if (cecRxEomFlag)
					{
						if (cecRxByteArray[0] == 05)
						{
							cecRxDataSemafore = true;
							cecRxAlldataBufferLength[cecRxAllDataCount] = cecRxByteCount;
							for (uint8_t x = 0; x < cecRxByteCount; x++)
							{
								cecRxAllDataBuffer[cecRxAllDataCount][x] = cecRxByteArray[x];
							}
							cecRxAllDataCount++;
							if (cecRxAllDataCount > 10)
							{
								cecRxAllDataCount = 10;
							}
							cecRxDataSemafore = false;
							// Serial.printf("Number Of Bytes %d\n", cecRxByteCount);
							if (cecRxByteCount > 1)
							{
								Serial.print("Rx Data: ");
								for (int x = 0; x < cecRxByteCount; x++)
								{
									Serial.printf("%02x, ", cecRxByteArray[x]);
								}
								Serial.println();
								
							}							
						}
						stopKeyHDMI = false;
					}
					
				}
				cecRxInBit9Pos = false;

				cecRxDataSemafore = false;
				cecRxCurrBitValue = 0;
				cecRxCurrByteBitPos = 0;
				cecRxCurrByteData = 0;
				cecRxPleaseAck = false;
				cecRxAckStartLow = false;
				cecRxEomFlag = false;
				
			}
		}
	}
}

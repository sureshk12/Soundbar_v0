#pragma once

void intilizeCecReceive();
uint8_t cecRxIsDataAvaialble();
uint8_t cecRxGetdata(uint8_t *receiveData, uint8_t *receiveDataLength);
void doCecReadTask();

#pragma once

bool btModeBegin();
void doKeyBTAction(uint8_t actionNum);
void btReConnect();
void btDisconnect();
void setBtConnectFlagToFalse();
void doBtTask();
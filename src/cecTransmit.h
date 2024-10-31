#pragma once

void doAutoReply();
void cecTxData();                           // Used Locally to send Tx
void doCecTxTask();
void cecTxSelfAddress();
void report_sbOsdName();                    // reports Soundbar Name
void set_tvSystemAudioMode(uint8_t on_off); // Requests TV to Set System audio mode
void report_sbVolumeMuteStatus();           // report Volume level and Mute ststus to TV
void report_sbSystemAudioModeStatus();      // report SB SystemAudioModeStatus
void get_TvPowerStatus();                   // Get TV Power status
void report_sbPowerStatus();                // report SB power status
void report_sbShortAudioDescriptor();       //report SB short audio descriptor
void set_tvArcOn();                         // Request TV to switch on ARC
void set_tvArcTermination();                // Request TV to terminate ARC
void broadcast_oneByte();
void broadcast_oneByteNew();
void broadcast_sbSystemAudioMode(); // Broadcast SB Syfetem Audio Mode
void broadcast_sbPhysicalAddress();         // Broadcast SB Physical address
void broadcast_sbDeviceVendorId();          // Broadcast SB Device ID
void reduceCecTxAllDataCount();
//void generateDebugPulse();
void serialPrintTxBuffer();



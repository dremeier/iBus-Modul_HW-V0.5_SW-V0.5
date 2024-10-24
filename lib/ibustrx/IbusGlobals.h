#ifndef IbusGlobals_h
#define IbusGlobals_h
#include <Arduino.h>

/*
globale Variable für IbusTrx.cpp
*/

byte IBUS_IKE_MESSAGE[32] = { 0x30 , 0x00 , 0x80 , 0x1A , 0x34 , 0x00 };      // IKE TextFeld ansprechen
uint8_t maxChars = 20;                                                        // maximale Chars im IKE Display






#endif
// bluetooth.h
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>

// Deklaration der Funktionen
void parseTrackInfo(String data);
void printTimeFromMilliseconds(unsigned long milliseconds);
void parseTrackStat(String data);
void parsePlayStatus(String data);
void altModeRoutine();
void PCMD3140_lesen();
void PCMD3140_schreiben();
void BT_Toggle();

#endif // BLUETOOTH_H

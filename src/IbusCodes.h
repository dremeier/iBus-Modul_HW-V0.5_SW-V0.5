// ibusCodes.h

/*
  iBus extern Variablen and Funktions declaire for the main.cpp
*/
#ifndef IbusCodes_h
#define IbusCodes_h
#include <Arduino.h>
#include <IbusTrx.h>                    // IbusTrx library selbst abgeändert und erweitert
#include <BH1750.h> 
#include "timer.h"                      // Timer Lib um Delay zu vermeiden

extern IbusTrx ibusTrx;                 // IbusTrx instance
extern BH1750 lightMeter;
extern Timer BluetoothToggleTimer;      // Bluetooth ein/aus


// Deklaration der Variablen als extern
extern uint8_t cleanIKE[6];
extern uint8_t BlinkerRe[13];
extern uint8_t BlinkerLi[13];
extern uint8_t BlinkerAus[13];
extern uint8_t LCMdimmReq[4];
extern uint8_t LCMBlinkerAdd[2];
extern uint8_t BCcoolbeginn[7];
extern uint8_t BCcoolend[5];
extern uint8_t door_open_driver[5];
extern uint8_t ZV_lock[6];
extern uint8_t Heimleuchten[16];
extern uint8_t Tankinhalt[5];
extern uint8_t SthzEIN[5];
extern uint8_t SthzAUS[5];

// ########################## Funktionserklärungen ############################
void iBusMessage();
void Coolant(uint8_t temp);        // Funktion zur Anzeige der Kühlmitteltemperatur im Bordmonitor
void Daemmerung();                 // Funktion zur Messung der Umgebungshelligkeit und Steuerung des Heimleuchtens
void ClearToSend();

#endif

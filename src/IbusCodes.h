// ibusCodes.h

/*
  iBus extern Variablen and Funktions declaire for the main.cpp
*/
#ifndef IbusCodes_h
#define IbusCodes_h
#include <Arduino.h>
#include <IbusTrx.h>              // IbusTrx library selbst abgeändert und erweitert
#include <BH1750.h> 
#include <Snooze.h>               // Teensy Snooze Lib

extern IbusTrx ibusTrx;           // IbusTrx instance
extern BH1750 lightMeter;         // Lichtsensor Instance
extern SnoozeUSBSerial usb;
extern SnoozeDigital digital;     // this is the pin's wakeup driver
extern SnoozeBlock config_sleep;  // install driver into SnoozeBlock

// Deklaration der Variablen als extern
extern uint8_t cleanIKE[6];
extern uint8_t BlinkerRe[13];
extern uint8_t BlinkerLi[13];
extern uint8_t BlinkerAus[13];
extern uint8_t LCMdimmReq[4];
extern uint8_t LCMBlinkerAdd[2];
extern byte BCcool[16];                // Array für die Kühlmitteltemperaturanzeige im Bordmonitor
extern uint8_t BCcoolbeginn[6];
extern uint8_t BCcoolend[7];
extern uint8_t door_open_driver[5];
extern uint8_t ZV_lock[6];
extern uint8_t Heimleuchten[16];
extern uint8_t Tankinhalt[5];
extern uint8_t SthzEIN[5];
extern uint8_t SthzAUS[5];
extern uint8_t RecalculateConsumption1[];

// ########################## Funktionserklärungen ############################
void iBusMessage();
void processStatusMessage(uint8_t byte1, uint8_t byte2);
void Coolant(uint8_t temp);        // Funktion zur Anzeige der Kühlmitteltemperatur im Bordmonitor
void Daemmerung();                 // Funktion zur Messung der Umgebungshelligkeit und Steuerung des Heimleuchtens
void ClearToSend();
void updateNavZoom();             // Automatischer Navigations Zoom je nach Geschwindigkeit
void SpiegelAnklappen();          // Beim Verriegeln Seitenspiegel anklappen
void BlinkerUnblock();            // Blinkersperre zwischen zwei Blinksignalen aufheben
void resetConsumption1();         // reset consumptrion1

#endif

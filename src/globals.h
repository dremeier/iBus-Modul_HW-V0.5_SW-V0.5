#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <IPAddress.h>  // Einbinden der IPAddress-Bibliothek für IP-Adressen
#include <INA219_WE.h>

//#define BTConfig // Einschlaten der Bluetooth Configuration
/*
    Debugging über die serielle Schnittstelle
    Wenn DEBUG aktiviert ist, wird Serial für Debugging verwendet.
    Ansonsten bleiben die Debug-Befehle inaktiv.
*/
#define DEBUG Serial              // USB Port on Teensy   // debugging aktivieren, if comment in then you have debugging over serial.print
#ifdef DEBUG
    #define debug(...) DEBUG.print(__VA_ARGS__)
    #define debugln(...) DEBUG.println(__VA_ARGS__)
    #define debugbegin(...) DEBUG.begin(__VA_ARGS__)
    #define debugf(...) DEBUG.printf(__VA_ARGS__)
#else
    #define debug(...)
    #define debugln(...)
    #define debugbegin(...)
    #define debugf(...)
#endif

//########## UART und PIN SETTINGS ####################################
#define BTcom Serial1           // Bluetooth Modul UART
#define ibusPort Serial2        // Serial für Ibus = Serial2 = RxPIN 7 und TxPin 8 , CleartoSent = Pin 5
#define SaraCom Serial3         // Serieller Port für die Kommunikation mit Sara-R5
#define Sara_PWR_ON 21          // Sara Power Pin mit internem Pull-up, 0,1s bis 2s LOW, dann HIGH
#define Sara_RI 22              // Ring Indicator Pin für Sara
#define Sara_DTR 20             // Data Terminal Ready Pin, high= PSV allowed, low= no Power Saving
#define Sara_RST 10             // Reset Pin im neuen Design für Sara-R5
//const byte ledPin = 13;       // buildin LED
#define SthzRelais 9            // SSR Relais für die Standheizung
const byte senSta = 5;          // sen/Sta output from Melexis TH3122 PIN9.
const byte TH_EN = 6;           // EN input from Melexis TH3122 PIN2.
const byte TH_RESET = 4;        // Reset output from Melexis TH3122 PIN14.
//
#define sys_ctl 12              // BT sys_ctl Pin zum Ein/Ausschalten des Modules
#define PCM_EN 23               // Enable LDOs für PCM I2S Power
#define PCMD3140_ADDRESS 0x4E   // I2C-Adresse des PCMD3140 PDM-Micro

//################ Timer ###################################
//extern const long RandomTempTimer;    // Timer für eine zufällige Außentemperatur (in Millisekunden)

// ########################## MQTT TOPICS ##############################
// MQTT Topic Struktur für Sara -> iobroker Publishes (Senden)
#define topic1a  "Metrics"    // Topic für alle Metriken
#define topic1b  "Sth"        // Topic für die Standheizung
#define topic1c  "GPS"        // Topic für GPS-Daten
#define topic1d  "Sara"       // Topic für alle Daten rund um Sara-R5
#define topic1e  "Subs"       // Topic für die Daten aus Subscriptions

// Metrics Unterthemen
#define topic2a1  "RPM"       // Motordrehzahl (RPM)
#define topic2a2  "Speed"     // Geschwindigkeit (Speed)
#define topic2a3  "Fuel"      // Tankinhalt (Fuel)
#define topic2a4  "AvrSpeed"  // Durchschnittsgeschwindigkeit (Average Speed)
#define topic2a5  "AvrFuel"   // Durchschnittlicher Kraftstoffverbrauch (Average Fuel)
#define topic2a6  "Bat"       // Batteriespannung (Battery Voltage)(kommt vom INA219)
#define topic2a7  "Ign"       // Zündung an/aus (Ignition State)
#define topic2a8  "Driver"    // Fahrer-ID (Driver ID)
#define topic2a9  "Lock"      // Türen verriegelt (Lock State)
#define topic2a10 "Atmp"      // Außentemperatur (Ambient Temperature)
#define topic2a11 "Ktmp"      // Kühlwassertemperatur (Coolant Temperature)
#define topic2a12 "Strom"     // Strom des gesamten iBus-Modules (kommt vom INA219)

// Standheizung Unterthemen
#define topic2b1  "Stat"      // Status der Standheizung (Status)
#define topic2b2  "RemT"      // Verbleibende Laufzeit der Standheizung (Remaining Time)

// Sara-R5 Daten
#define topic2d1 "PsdIPAddress"  // PSD interne IP-Adresse
#define topic2d2 "HwVer"         // Hardware-Version
#define topic2d3 "SwVer"         // Software-Version
#define topic2d4 "Sara_ID"       // Sara-R5 ID
#define topic2d5 "FwVer"         // Firmware-Version
#define topic2d6 "SIM_ID"        // SIM-Karten-ID

// Subscription Daten
#define topic2e1 "SthzON"        // Standheizung einschalten (Sthz ON)
#define topic2e2 "gpsDist"       // GPS Abstand (gpsDist)
#define topic2e3 "gpsAngel"      // GPS Winkel (gpsAngel)
#define topic2e5 "AVerr"         // Automatische Verriegelung (AVerr)
#define topic2e6 "SpiegAnkl"     // Spiegel anklappen (SpiegAnkl)
#define topic2e7 "NaviScale"     // Navigation Maßstab anpassen (NaviScale)
#define topic2e8 "BcReset"       // Bordcomputer zurücksetzen (BcReset)
#define topic2e9 "TipBlink"      // Tippblinken (TipBlink)
#define topic2e10 "usLight"       // US-Standlicht

// ####################### Externe globale Variablen #######################
// Diese Variablen speichern den aktuellen Zustand von verschiedenen Fahrzeugwerten

extern String fix;                  // GPS Fix Typ
extern uint8_t sat;                 // Anzahl der sichtbaren Satelliten
extern float lat;                   // GPS-Breitengrad
extern float lon;                   // GPS-Längengrad
extern uint8_t altitude;            // Höhe über Meeresspiegel (GPS)
extern float kwassertemp;           // Kühlwassertemperatur in °C
extern float outTemp;               // Außentemperatur in °C
extern uint8_t coolant;             // Kühlmitteltemperatur in °C
extern bool ZVlocked;                   // Türen verriegelt (true/false)
extern String driverID;             // Aktuelle Fahrer-ID
extern bool Ignition;               // Zündung an/aus (true/false)
extern float bat;                   // Batteriespannung in Volt
extern float strom;                 // Strom des gesamten iBus-Modules (kommt vom INA219)
extern float avrFuel;               // Durchschnittlicher Kraftstoffverbrauch (L/100km)
extern uint16_t avrSpeed;           // Durchschnittsgeschwindigkeit (km/h)
extern uint16_t fuel;               // Aktueller Tankinhalt (L)
extern uint16_t speed;              // Aktuelle Geschwindigkeit (km/h)
extern uint16_t rpm;                // Aktuelle Motordrehzahl (U/min)
extern uint8_t remT;                // Verbleibende Laufzeit der Standheizung (Minuten)
extern uint8_t SthzRuntime;         // maximale Laufzeit der Standheizung

// Standheizungsstatus
// Subscriptions Variablen
extern bool stat;                      // Status der Standheizung (an/aus)
extern bool SthzON;                    // Sollwert der Standheizung
extern int gpsAbstand;                 // Abstand in Metern bis neue GPS-Daten gesendet werden
extern int gpsWinkel;                  // Winkel in Grad bis neue GPS-Daten gesendet werden
extern bool AVerr;                     // Automatische Verriegelung bei Geschwindigkeit > 30 km/h
extern bool SpiegAnkl;                 // Beim Abschließen Spiegel automatisch anklappen


// GPS-Abstand und -Winkel für die Übermittlung von Daten
extern int gpsAbstand;              // Abstand in Metern bis zur nächsten Datenübertragung
extern int gpsWinkel;               // Winkeländerung in Grad bis zur nächsten Datenübertragung

// Sara-R5 spezifische Daten
extern String HardwareV;            // Hardware-Version der Sara-R5 Einheit
extern String SoftwareV;            // Software-Version der Sara-R5 Einheit
extern String Model_ID;             // Modell-ID der Sara-R5 Einheit
extern String FirmwareV;            // Firmware-Version der Sara-R5 Einheit
extern String SIM_ID;               // SIM-Karten-ID
extern String PsdIPAddress;         // Interne IP-Adresse des PSD-Profils

// #################### Vorherige Werte (zur Vergleichsprüfung) ####################
// Diese Variablen speichern die vorherigen Zustände der Fahrzeugdaten zur Überprüfung, ob sich Werte geändert haben
extern String prevPsdIPAddress;
extern String prevFix;
extern uint8_t prevSat;
extern float prevLat;
extern float prevLon;
extern float prevKwassertemp;
extern float prevAussentemp;
extern bool prevZVlocked;
extern String prevDriverID;
extern bool prevIgnition;
extern float prevBat;
extern float prevAvrFuel;
extern uint16_t prevAvrSpeed;
extern uint16_t prevFuel;
extern uint16_t prevSpeed;
extern uint16_t prevRpm;
extern uint8_t prevRemT;
extern int prevgpsAbstand;
extern int prevgpsWinkel;
extern bool prevStat;
extern bool prevSthzON;
extern bool prevAutomVerriegeln;           // Automatische Verriegelung bei Geschwindigkeit > 30 km/h
extern bool prevSpiegelAnklappen;
extern bool prevNaviScale;
extern bool prevBcResetten;
extern bool prevTippblinken;
extern float prevstrom; 
extern bool prevusLight;                  // Flag für US-Standlicht

// MQTT-spezifische Variablen
extern String myClientID;          // MQTT-Client-ID
extern unsigned long previousMillis;// Vorherige Zeitmarke für Timer
extern bool RI_state_last;         // Letzter Zustand von RI (Ring Indicator)
extern bool ledState;              // Zustand der LED

// ############################### GPS ###############################
//extern PositionData gps;               // GPS-Positionsdaten
//extern SpeedData spd;                  // Geschwindigkeitsdaten
//extern ClockData clk;                  // Uhrzeitdaten

// IBUS
// ######################## Timer und Debug Flags ######################
extern bool IKEclear;                  // Flag zum Löschen des IKE (Kombiinstrument) Textes
extern long msTimer;                   // Timer für Zykluszeiten
extern long msSleep;                   // Zeit für System-Schlaf
extern bool sysSleep;                  // Flag für den Sleep-Timer
extern bool th_reset;                  // Testflag für Reset
extern unsigned long SleepTime;        // Zeit bis zum Einschlafen des TH3122
extern unsigned int t_clearIKE;        // Zeit in Millisekunden bis der Text im IKE gelöscht wird
extern bool Tippblinken;               // Flag zum Aktivieren/Deaktivieren der Tippblinker-Funktion
extern bool AutomVerriegeln;           // Automatische Verriegelung bei Geschwindigkeit > 30 km/h
extern bool ZVlockDone;                    // Flag für "geschlossene" Türen bei Geschwindigkeit
extern uint8_t BlinkcountLi;           // Zähler für Blinker links
extern uint8_t BlinkcountRe;           // Zähler für Blinker rechts
//extern byte LCMdimm[16];               // Ausgelesener Dimmwert des LCM (Lichtsteuergerät)
extern byte LCMdimm;
extern byte LCMBlinker[16];            // Array für das Zusammenbauen des Blinker-Befehls mit LCM-Dimmwerten

//extern uint8_t turn;                   // Flag für Blinker links oder rechts
extern uint8_t LCMdimmOK;              // Flag für erfolgreich ausgelesene LCM-Dimmwerte
//extern bool SpiegelAnklappen;          // Spiegel anklappen beim Schließen
extern bool NaviScale;                 // Navigationsmaßstab an die Geschwindigkeit anpassen
extern bool BcResetten;                // Durchschnittsverbrauch automatisch zurücksetzen
extern bool usLight;                  // Flag für US-Standlicht
extern bool usLightTrigger;
extern byte usLightByte21;

// ######################## Helligkeitssensor und Heimleuchten ######################
extern const int NUM_READINGS;
extern int ldrValues[];
extern int ldrindex;
extern int sum;                        // Summe der Messwerte für die Berechnung des Durchschnitts
extern int average;                    // Durchschnittswert der Helligkeitssensoren
extern bool dunkel;                    // Flag, um festzustellen, ob es dunkel ist
extern bool heiml;                     // Flag für Heimleuchten (wenn das Auto geöffnet wird)
//extern bool MotorOff;                  // Flag für Motor aus (zum Schalten der Zentralverriegelung)
extern bool DvrdoorFr;                 // Flag für offene Fahrertür

// Ober- und Untergrenze für den LDR-Wert (Lichtabhängiger Widerstand, Lichtsensor)
extern const int UPPER_THRESHOLD;      // Obere Grenze für den LDR-Wert
extern const int LOWER_THRESHOLD;      // Untere Grenze für den LDR-Wert

//'''''''''''' BLUETOOTH SPEZIFISCHE VARIABLEN ''''''''''''''''''''''''''
extern bool lese_i2c;
extern bool sysctl_on;
extern bool sysctl_off;
extern unsigned int sysctl_timer;
extern bool BT_state;
extern bool trackInfoReceived;
extern bool trackStatReceived;
extern bool playStatReceived;
extern String receivedData;
extern unsigned long totalTrackTime; // Globale Variable zur Speicherung der Gesamtzeit des Tracks in Millisekunden
extern int hours;
extern int minutes;
extern int secs;

extern bool inAltMode;
extern bool inTransferMode;


// ################################# globale funktions ########################################
extern INA219_WE ina219;

void readINA219();



#endif // GLOBALS_H

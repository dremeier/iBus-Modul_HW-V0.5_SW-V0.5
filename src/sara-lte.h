// sara-lte.h

#ifndef SARA_LTE_H
#define SARA_LTE_H

#include <Arduino.h>
#include <EEPROM.h>
#include <SparkFun_u-blox_SARA-R5_Arduino_Library.h>  // SARA R5 Bibliothek
#include "globals.h"
#include <IbusTrx.h>
#include "IbusCodes.h"  
#include "timer.h"              // Timer Lib um Delay zu vermeiden

extern Timer readMqttTimer;                  // readMqttData()
extern Timer mqttConnectTimer;               // mqtt Connect Delay
extern Timer publishSaraTimer;               // um einmalig die Sara-Daten zu senden
extern Timer SthzMinuteTimer;

extern IbusTrx ibusTrx;                // IbusTrx instance

// Deklaration globaler Variablen mit extern
extern SARA_R5 mySARA;
extern String fix;
extern uint8_t sat;
extern float lat;
extern float lon;
extern float kwassertemp;
extern float aussentemp;
extern uint8_t remT;
extern bool stat;
extern bool lock;
extern String driverID;
extern bool ing;
extern float bat;
extern float avrFuel;
extern uint16_t avrSpeed;
extern uint16_t fuel;
extern uint16_t speed;
extern uint16_t rpm;
extern String myClientID;
extern unsigned long previousMillis;
extern bool RI_state_last;
extern bool ledState;

/* GPS */
// declared in globals.cpp
extern PositionData gps;
extern SpeedData spd;
extern ClockData clk;

// Standheizungs timer
extern unsigned long lastSthzMillis;  // Zum Speichern des letzten Zeitpunkts für den Timer
//extern const unsigned long interval = 60000;  // 1 Minute in Millisekunden
extern bool SthztimerRunning;  // Status des Timers


// ################ Funktionserklärungen (Deklarationen) ###################################################
/* Funktions ins Sara-lte.cpp: */
void readMqttData();                                        // lese die MQTT-Subscription ein
void publishSubData();                                      // MQTT senden der Subscription daten
void publishMetricsData();                                  // Deklaration von publishMetricsData
void publishSaraData();                                     // MQTT senden der Sara und Internen Daten nur einmal nach start
void publishGpsData(double distance, double angle);         // MQTT senden von GPS Daten
void PollGPSData();                                         // Anfordern der GPS-Daten
void printGPS(void);                                        // Auslesen der GPS-Variablen
void processMQTTcommandResult(int command, int result);     // MQTT-Befehlsergebnis verarbeiten
void processPSDAction(int result, IPAddress ip);            // Holen der IP-Adresse
void pingResponseCallback(int retry, int p_size, String remote_hostname, IPAddress ip, int ttl, long rtt); // Auswertung des Pings
void mqttCommandResultCallback(int command, int result);    // Auswertung von +UUMQTTC, Die Rückgabe wenn wir uns beim MQTT-Broker anmelden mit +UMQTTC=1,1
void mqttConnect();
void mqttdisconnect();
void mqttSubscribe();                                       // Subscribe to the channel topic
void updateSthzTimer();                                     // Timer-Funktion für Standheizung aufrufen
void checkNetworkRegistration();                            // Überprüfen des Netzwerkstatus
void epsRegistrationCallback(SARA_R5_registration_status_t status, unsigned int tac, unsigned int ci, int Act);
void registrationCallback(SARA_R5_registration_status_t status, unsigned int lac, unsigned int ci, int Act);
void readRssi();
void activateInternet();



// Template-Funktionen zum Schreiben in das EEPROM (muss hier in der header-Datei stehen)
template <class T> int EEPROM_writeAnything(int ee, const T& value) {
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++) {
        EEPROM.write(ee++, *p++);
    }
    return i;
}
template <class T> int EEPROM_readAnything(int ee, T& value) {
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++) {
        *p++ = EEPROM.read(ee++);
    }
    return i;
}


#endif  // SARA_LTE_H

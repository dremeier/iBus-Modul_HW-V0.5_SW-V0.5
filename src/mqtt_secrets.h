#ifndef MQTT_SECRETS_H
#define MQTT_SECRETS_H

#include <Arduino.h>

// ioBroker via MQTT Publish
#define brokerName "yxz.dnshome.de" // MQTT Broker
#define brokerPort 0000 // MQTT port (TCP, encryption, ausgedachter Port)
#define myUsername "user"
#define myPassword "pass"
/*
Sara-R5 -> trac.dnshome.de -> pfsense -> Nginx profil SSL-Offloading -> ioBroker
*/

// Sicherheitsprofil-Nummer definieren (Wählen Sie eine Nummer zwischen 0 und 4)
uint8_t secProfileNum = 0;


#endif

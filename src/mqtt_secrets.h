#ifndef MQTT_SECRETS_H
#define MQTT_SECRETS_H

#include <Arduino.h>

// ioBroker via MQTT Publish
#define brokerName "trac.dnshome.de" // MQTT Broker
#define brokerPort 8894 // MQTT port (TCP, encryption, ausgedachter Port)
#define myUsername "mqttone"
#define myPassword "andre2106"
/*
alternatie:
MQTT Einstellungen:
Host: 192.168.107.220 (iobroker.ame)
PORT: 1893
Client: z.B. Sonnenrollo
Benutzer: mqttbroker
Password: andre2106
*/

/*
Sara-R5 -> trac.dnshome.de -> pfsense -> Nginx profil SSL-Offloading -> ioBroker
*/

// Sicherheitsprofil-Nummer definieren (Wählen Sie eine Nummer zwischen 0 und 4)
uint8_t secProfileNum = 0;


#endif

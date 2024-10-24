// bluetooth.cpp
#include "bluetooth.h"
#include "IbusCodes.h"
#include "globals.h"

extern bool BT_state; 
extern bool inAltMode;
extern bool sysctl_on;

// zerlegen der Track informationen in SONG, ARTIST, ALBUM
void parseTrackInfo(String data) {
    int index1 = data.indexOf('=') + 1;
    int index2 = data.indexOf((char)0xFF, index1);
    int index3 = data.indexOf((char)0xFF, index2 + 1);

    String song = data.substring(index1, index2);  
    String artist = data.substring(index2 + 1, index3);
    String album = data.substring(index3 + 1, data.length() - 2); // Ignoriere das '\r' am Ende

    debugln("Song:   " + song);
    debugln("Artist: " + artist);
    debugln("Album:  " + album);
/* Erklärung zu ^^
indexOf(): Diese Funktion sucht nach einem bestimmten Zeichen oder einer Zeichenkette in einer größeren Zeichenkette und gibt die Position des ersten Vorkommens zurück. In diesem Fall wird nach einem bestimmten Zeichen (= oder <FF>) in der Zeichenkette data gesucht.
substring(): Diese Funktion extrahiert einen Teil der Zeichenkette aus einer größeren Zeichenkette, beginnend bei einer Startposition bis zu einer Endposition oder bis zum Ende der Zeichenkette. Hier wird es verwendet, um Teile der Zeichenkette data basierend auf den Indizes zu extrahieren, die durch indexOf() gefunden wurden.
In dem spezifischen Codeausschnitt:
indexOf('='): Findet die Position des =-Zeichens in der Zeichenkette data.
indexOf((char)0xFF, index1): Sucht nach dem <FF>-Zeichen ab der Position index1.
data.substring(index1, index2): Extrahiert einen Teil der Zeichenkette data zwischen den Positionen index1 und index2.
Das Ganze läuft darauf hinaus, dass indexOf() die Positionen von Zeichen in einer Zeichenkette findet und substring() dann Teile der Zeichenkette zwischen diesen Positionen extrahiert. In diesem Fall werden diese Funktionen verwendet, um den Songnamen, Künstlernamen und Albumnamen aus einer Zeichenkette zu extrahieren, die durch bestimmte Trennzeichen getrennt ist.
*/
}

// auswerten der Track-Zeiten
void printTimeFromMilliseconds(unsigned long milliseconds) {
    unsigned long seconds = (milliseconds + 500) / 1000;
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;

    debug("Time: ");
    debug(hours);
    debug(":");
    if (minutes < 10) debug("0");
    debug(minutes);
    debug(":");
    if (secs < 10) debug("0");
    debugln(secs);
}

// zerlegen der Track-Zeiten
void parseTrackStat(String data) {
    int index1 = data.indexOf('=') + 1;
    int index2 = data.indexOf(',', index1);
    int index3 = data.indexOf(',', index2 + 1);

    String param1 = data.substring(index1, index2);
    String param2 = data.substring(index2 + 1, index3);
    String param3 = data.substring(index3 + 1, data.length() - 2); // Ignoriere das '\r' am Ende

    if (totalTrackTime == 0) {
        totalTrackTime = param3.toInt();
        debug("Total Track Time: ");
        printTimeFromMilliseconds(totalTrackTime);
    } else {
        unsigned long elapsedTime = param2.toInt();
        debug("Elapsed Time:     ");
        printTimeFromMilliseconds(elapsedTime);

        unsigned long remainingTime = totalTrackTime - elapsedTime;
        debug("Remaining Time:   ");
        printTimeFromMilliseconds(remainingTime);
        debugln("");
    }
}

// zerlegen des Play-Status
void parsePlayStatus(String data) {
    char statusChar = data.charAt(1);

    switch (statusChar) {
        case '0':
            debugln("Stopped");
            BT_state = false;
            break;
        case '1':
            debugln("Playing");
            BT_state = true;
            break;
        case '2':
            debugln("Paused");
            break;
        case '3':
            debugln("Fast Forwarding");
            break;
        case '4':
            debugln("Fast Rewinding");
            break;
        default:
            debugln("Unknown Status");
            break;
    }
}

// Bluetooth Modul ein/aus schalten
void BT_Toggle() {
    digitalWrite(sys_ctl, HIGH);
    debugln("Toggle Bluetooth");
    sysctl_on = false;
}


/*
void altModeRoutine() {
    debugln("____ Start Bluetooth Console ___");
    while (inAltMode) {
        delay(10);
        if (DEBUG.available() > 0) {
            char receivedChar = DEBUG.read();
            if (receivedChar == 'y') {
                unsigned long altTimer = millis();
                while (millis() - altTimer < 1000) {
                    if (DEBUG.available() > 0) {
                        char nextChar = DEBUG.read();
                        if (nextChar == 'q') {
                            debugln("____ Exit Bluetooth Console ___");
                            inAltMode = false;
                            break;
                        }
                    }
                }
                break;
            } else {
                BTcom.write(receivedChar);
                DEBUG.write(receivedChar);
            }
        }
        if (BTcom.available()) {
            char receivedChar = BTcom.read();
            DEBUG.write(receivedChar);
        }
    }
}

*/
/*
void PCMD3140_lesen() {
    Wire1.beginTransmission(PCMD3140_ADDRESS);
    Wire1.write(0x7); // Hier die gewünschte Registeradresse eintragen
    Wire1.endTransmission();

    Wire1.requestFrom(PCMD3140_ADDRESS, 1); // Anzahl der Bytes, die gelesen werden sollen

    if (Wire1.available()) {
        byte value = Wire1.read();
        debug("Wert des Registers ");
        debug(0x7);
        debug(" : ");
        debugln(value, HEX);
    } else {
        debugln("Fehler beim Lesen des Registers");
    }
}

void PCMD3140_schreiben() {
    Wire1.beginTransmission(PCMD3140_ADDRESS);
    Wire1.write(0x07); // Hier die gewünschte Registeradresse eintragen
    Wire1.write(0x60);
    Wire1.endTransmission();
}

*/

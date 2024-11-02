#include "globals.h"
#include <IPAddress.h>  // Einbinden der IPAddress-Bibliothek
#include <INA219_WE.h>

#define I2C_ADDRESS 0x40
INA219_WE ina219 = INA219_WE(&Wire1, I2C_ADDRESS);

// Definition der globalen Variablen
//################## I2C #################################
bool lese_i2c = false;
//'''''''''''' BLUETOOTH SPEZIFISCHE VARIABLEN ''''''''''''''''''''''''''
bool sysctl_on =false ;
bool sysctl_off =false ;
unsigned int sysctl_timer;

bool trackInfoReceived = false;
bool trackStatReceived = false;
bool playStatReceived = false;
String receivedData;
unsigned long totalTrackTime = 0; // Globale Variable zur Speicherung der Gesamtzeit des Tracks in Millisekunden
int hours = 0;
int minutes = 0;
int secs = 0;
bool BT_state =false;
bool inAltMode = false;
bool inTransferMode = false;


// ################### Mqtt Metric Variablen ######################
String fix;                         // GPS fix type
uint8_t sat = 0;                    // Anzahl Satelliten
float lat;                          // Längengrad
float lon;                          // Breitengrad
uint8_t altitude;                   // GPS Höhe
float kwassertemp;                  // Kühlwassertemperatur in C°
float outTemp;                      // Außen Temperatur
uint8_t coolant;                    // Kühlmittel Temperatur
bool ZVlocked = 1;                  // Türen Verriegelt 
String driverID = "Nil";            // Fahrer ID
bool Ignition;                      // Zündung an/aus
float bat = 12.8;                   // Batterie Spannung in V
float avrFuel = 11.3;               // Durchschnittlicher Kraftstoffverbrauch in L/100Km
uint16_t avrSpeed = 85;             // Durchschnittsgeschwindigkeit in Km/h
uint16_t fuel = 41;                 // Tankinhalt in L
uint16_t speed = 0;                 // Geschwindigkeit in Km/h
uint16_t rpm = 0;                   // Motordrehzahl
uint8_t SthzRuntime = 25;           // maximale Laufzeit der Standheizung
uint8_t remT = SthzRuntime;         // Restzeit der Standheizung in Minuten

// ###################### Subscribtion Variablen ######################
bool stat = 0;                      // Status der Standheizung 0 oder 1
bool SthzON = 0;                    // Sollwert der Standheizung
int gpsAbstand = 150;               // Abstand in Metern bis neue Daten zum Server geschickt werden
int gpsWinkel = 30;                 // Winkel in Grad bis neue Daten zum Server geschickt werden

// ######################## Mqtt Sara Variablen #######################
String HardwareV = "HW 0.5";
String SoftwareV = "SW 0.5.3";
String Model_ID;
String FirmwareV;
String SIM_ID;
String PsdIPAddress;                // IP-Adresse vom PSD-Profil
/* _________________________________________ SW History: ______________________________________________
SW 0.3.0
    erster merge mit Sara_R5 und iBus Sketch, Ibus funktionierte, jedoch kam nur jede zweite Antwort raus, BH1750 keine Funktion - Fehler in der Initialiesierung.

SW 0.3.1
    bug - BH1750 initialisierung geändert in: if (!lightMeter.begin(BH1750::CONTINUOUS_LOW_RES_MODE, 0x23, &Wire1))..... Wire1 war entscheident

SW 0.3.2
    bug - beim senden von Zündung aus, Teensy friert ein, Schleife in Automatischverriegeln behoben

SW 0.3.3
    Ibus wird immer gesendet sobald eine pause von 25ms herrscht und Interrupt -> "ClearToSend"
    siehe logik hinter "Daemmerung();" im Loop - Ein Test unter realen Bedungen machen !

SW 0.3.4
    Variablen von IbusCodes.h zusammengelegt nach globals.h und globals.cpp
    Auslagern der Ibus-Funktionen nach IbusCodes.cpp
    Add Subscriptions für Spiegelanklappen, Tippblinken, Autom.Verriegeln, BC-Resetten, NaviScale

SW 0.4.0
    erster merge mit Bluetooth Modul

SW 0.4.1
    INA219 Spannungs und Strom Sensor testhalber implementiert, incl. MQTT daten

SW 0.5.0
    Transformation auf Hardware Plattform V0.5 mit der HW 0.4, Pinbelegung D13->D23 PCM_EN hat sich geändert

SW 0.5.1
    Umbau der MQTT Verbindung
    Verbindungsaugbau bei Verlust (URC)
    MQTT Subscribeabfrage nicht mehr im Zeitintervall sondern nach Vorliegen einer Nachricht (URC 6.1)

SW 0.5.2 
    Reale Anpassung an HW 0.5 mit Sara-R510M8S-01b        

SW 0.5.3
    automatischer Navigations Zoom je nach Geschwindigkeit

*/

// Timer-Definitionen
//const long RandomTempTimer = 60000;     // Timer für eine zufällige Außentemperatur (in Millisekunden)

// Globale Variablen zur Speicherung der vorherigen Zustände
String prevPsdIPAddress;
String prevFix;
uint8_t prevSat;
float prevLat;
float prevLon;
float prevKwassertemp;
float prevAussentemp;
bool prevZVlocked;
String prevDriverID;
bool prevIgnition;
float prevBat;
float strom;                 // Strom des gesamten iBus-Modules (kommt vom INA219)
float prevAvrFuel;
uint16_t prevAvrSpeed;
uint16_t prevFuel;
uint16_t prevSpeed;
uint16_t prevRpm;
uint8_t prevRemT;
bool prevStat;
bool prevSthzON;
int prevgpsAbstand;                 // Abstand in Metern bis neue Daten zum Server geschickt werden
int prevgpsWinkel;                  // Winkel in Grad bis neue Daten zum Server geschickt werden
bool prevAutomVerriegeln;           // Automatische Verriegelung bei Geschwindigkeit > 30 km/h
bool prevTippblinken;
bool prevSpiegelAnklappen;
bool prevBcResetten;
bool prevNaviScale;
float prevstrom;

// MQTT static or subscribed values:
String myClientID = "BMW7er";       // Mqtt Client ID
unsigned long previousMillis = 0;
bool RI_state_last = false;
bool ledState = false;

/* ################################ IBus ################################ */
// Variablen für das IBus-System
bool IKEclear = false;              // Flag um das IKE zu löschen
long msTimer = 0;                   // Timer für Zyklen
long msSleep = 0;                   // Zeit für System-Schlaf
bool sysSleep = 0;                  // Flag für Sleep-Timer
bool th_reset = 0;                  // Testflag
unsigned long SleepTime = 600000;   // Zeit bis zum Schlafenlegen des TH3122 (60.000 = 1 Minute)
unsigned int t_clearIKE = 10000;    // Zeit in Millisekunden bis der Text im IKE gelöscht wird
bool Tippblinken ;            // Flag zum Ein- und Ausschalten der Tipp-Blinker-Funktion
bool AutomVerriegeln;        // Automatisches Verriegeln bei Geschwindigkeit > 30Km/h und Entriegeln bei Motor aus
//bool MotorOff = false;              // Flag für Motor aus (Schlüssel aus dem Zündschloss)
bool ZVlockDone = false;              // Flag für verriegelte Türen bei Geschwindigkeit über 30Km/h
uint8_t BlinkcountLi = 0;           // Zähler für den linken Blinker
uint8_t BlinkcountRe = 0;           // Zähler für den rechten Blinker
//byte LCMdimm[16];                   // Ausgelesener Dimmwert
byte LCMdimm;
byte LCMBlinker[16];                // Array für das Zusammenbauen des Blinker-Befehls mit LCM-Dimmwerten
byte BCcool[15];                    // Array für die Kühlmitteltemperatur im Bordmonitor
//uint8_t turn = 0;                   // Flag für Blinker links und rechts
uint8_t LCMdimmOK = 0;              // Flag für erfolgreiche LCM-Dimmwert-Auslesung
//bool SpiegelAnklappen;            // Flag zum Spiegelanklappen beim Schließen
bool NaviScale;                     // Navigations-Maßstab an die Geschwindigkeit anpassen
bool BcResetten;                    // Tagesverbrauch-Durchschnitt automatisch zurücksetzen
bool SpiegAnkl;                     // Beim Abschließen Spiegel automatisch anklappen


/* ############################### Lichtsensor ################################ */
const int NUM_READINGS = 10;        // Anzahl der Messungen für den Durchschnitt
int ldrValues[NUM_READINGS] = {0};  // Array für die Messwerte
int ldrindex = 0;                   // Index für das Speichern der Werte im Array
int sum = 0;                        // Variable zur Berechnung der Summe
int average = 0;                    // Variable für den Durchschnittswert
bool dunkel = false;                // Flag zur Entscheidung über Grenzwerte
bool heiml = false;                 // Flag für Heimleuchten (Fahrertür auf, Licht an)
bool DvrdoorFr = false;             // Flag für Fahrertür auf

/* ######################## Grenzwerte für den LDR-Wert ######################## */
const int UPPER_THRESHOLD = 250;    // Obergrenze für den LDR-Wert (z.B. Tageslicht)
const int LOWER_THRESHOLD = 200;    // Untergrenze für den LDR-Wert (z.B. Dämmerung)

// ######################## Standheizungs timer
//unsigned long lastSthzMillis = 0;  // Zum Speichern des letzten Zeitpunkts für den Timer
bool SthztimerRunning = false;  // Status des Timers


// ################################# globale funktions ########################################
// Einlesen von Strom und Spannung
void readINA219(){                // wird im Intervall vom Timer4 aufgerufen
  float shuntVoltage_mV = 0.0;
  float busVoltage_V = 0.0;
  //float power_mW = 0.0; 
  bool ina219_overflow = false;
  
  ina219.startSingleMeasurement();
  shuntVoltage_mV = ina219.getShuntVoltage_mV();
  busVoltage_V = ina219.getBusVoltage_V();
  strom = ina219.getCurrent_mA();
  strom = strom *2;                     // für den shut mit 0.5 OHM muss der Wert verdoppelt werden
  //power_mW = ina219.getBusPower();
  bat  = busVoltage_V + (shuntVoltage_mV/1000);
  bat = bat +0.2;  // Verlustspannung durch den Filter usw. daher 0.21V addieren
  ina219_overflow = ina219.getOverflow();
  
  //debug("Shunt Voltage [mV]: "); debugln(shuntVoltage_mV);
  //debug("Bus Voltage [V]: "); debugln(busVoltage_V);
  debug("Load Voltage[V]: "); debugln(bat);
  debug("Current[mA]: "); debugln(strom);
  //debug("Bus Power [mW]: "); debugln(power_mW);
  if(ina219_overflow){
    debugln("Overflow! Choose higher PGAIN");
  }
  debugln();
}
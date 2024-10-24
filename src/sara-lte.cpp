// sara-lte.cpp

#include "sara-lte.h"
#include <Arduino.h>
#include "globals.h"
#include "neopixel.h"
#include <ArduinoJson.h>
#include "math_functions.h"
#include "IbusCodes.h"

// GPS Daten
PositionData gps;
SpeedData spd;
ClockData clk;
/***************************************
************** FUNCTIONS ***************
****************************************/

// einlesen der MQTT Daten mit +UMQTTC=6.1
void readMqttData(){
  debugln("--------------------------------------------- READ MQTT DATA !!!!!!");    
  // The MQTT API does not allow getting the size before actually reading the data.
  // So we have to allocate a big enough buffer.
  const int MQTT_MAX_MSG_SIZE = 512;      //1024;
  static uint8_t buf[MQTT_MAX_MSG_SIZE];
  String topic;
  int len = -1;
  int qos = -1;

  if (mySARA.readMQTT(&qos, &topic, buf, MQTT_MAX_MSG_SIZE, &len) == SARA_R5_SUCCESS)
  {
    if (len > 0)
    {
      debugln();
      debug(F("..............................................................read Subscribed MQTT data: "));
      debug(topic);
      debug(" value: ");
      Serial.write((const char *)buf, len);
      debugln();

      // Konvertieren der empfangenen Nachricht in einen String zur leichteren Verarbeitung
      String message = String((char*)buf).substring(0, len);

      // Processing for
      if (topic.endsWith("SthON")) {
        if (message == "1") {
          ledState = true;
          SthzON = 1;  
          updateSthzTimer();       
        } else if (message == "0") {
          ledState = false;
          SthzON = 0;
          updateSthzTimer();
        }
        // Update LED state based on the message
        digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
      }
      else if (topic.endsWith("GpsWinkel")) {
        // Convert message to a float and store it
        gpsWinkel = message.toFloat();
        debug("GPS Winkel: ");
        debugln(gpsWinkel);
        // You can now use the temperatureValue variable as needed
      }
      else if (topic.endsWith("GpsDist")) {
        // Convert message to a float and store it
        gpsAbstand = message.toFloat();
        debug("GPS Abstand: ");
        debugln(gpsAbstand);
        // You can now use the temperatureValue variable as needed
      }
      else if (topic.endsWith("AVerr")) {
          if (message == "1") {
          AutomVerriegeln = 1;
          debugln("Automatisch Verriegeln bei Geschwindigkeit ON");
        } else if (message == "0") {
          AutomVerriegeln = 0;
          debugln ("Automatisch Verriegeln bei Geschwindigkeit AUS");
        }
      } 
      else if (topic.endsWith("SpiegAnkl")) {
          if (message == "1") {
          SpiegAnkl = 1;
          debugln("Spiegel anklappen ON");
        } else if (message == "0") {
          SpiegAnkl = 0;
          debugln ("Spiegel anklappen AUS");
        }
      }
      else if (topic.endsWith("NaviScale")) {
        if (message == "1") {
        NaviScale = 1;
        debugln("Navigations Scale je nach Geschwindigkeit ON");
      } else if (message == "0") {
        NaviScale = 0;
        debugln ("Navigations Scale je nach Geschwindigkeit AUS");
      }
    }
      else if (topic.endsWith("BcReset")) {
        if (message == "1") {
        BcResetten = 1;
        debugln("BC reset für Fuel1 ON");
      } else if (message == "0") {
        BcResetten = 0;
        debugln ("BC reset für Fuel1 AUS");
      }
    }
      else if (topic.endsWith("Tblink")) {
        if (message == "1") {
          Tippblinken = 1;
          debugln("Tippblinken ON");
        } else if (message == "0") {
          Tippblinken = 0;
          debugln ("Tippblinken AUS");
        }
      }
      else if (topic.endsWith("SaraPowerOFF")) {
        if (message == "1") {
        mySARA.modulePowerOff();
        debugln("Sars goes PowerOFF");
        } else if (message == "0") {
          debugln ("Sara alive");
        }
      }
    }
    controlLED(1, CRGB::Green, 0);
    publishSubData();                   // MQTT, publiziere alle Daten die aus der Subscription reingekommen sind, also Echo
  } else {
    readMqttTimer.start();
    debugln("...............read MQTT data failed");
  }
}

//Sende MQTT-Daten der Subscription
void publishSubData() {
  // Prüfen, ob sich eine der Variablen geändert hat
  if (SthzON != prevSthzON || gpsAbstand != prevgpsAbstand || gpsWinkel != prevgpsWinkel ||
   stat != prevStat || prevAutomVerriegeln != AutomVerriegeln || prevSpiegelAnklappen != SpiegAnkl ||
   prevNaviScale != NaviScale || prevBcResetten != BcResetten || prevTippblinken != Tippblinken) {

    // Haben sich die Daten geändert schreibe sie in´s EEprom, die Daten werden im Setup wieder eingelesen      
    if (gpsAbstand != prevgpsAbstand || gpsWinkel != prevgpsWinkel ||
      stat != prevStat || prevAutomVerriegeln != AutomVerriegeln || prevSpiegelAnklappen != SpiegAnkl ||
      prevNaviScale != NaviScale || prevBcResetten != BcResetten || prevTippblinken != Tippblinken){
      EEPROM_writeAnything(0, gpsAbstand);
      EEPROM_writeAnything(sizeof(int), gpsWinkel);
      EEPROM_writeAnything(sizeof(int) + sizeof(gpsWinkel), AutomVerriegeln);
      EEPROM_writeAnything(sizeof(int) + sizeof(gpsWinkel) + sizeof(AutomVerriegeln), SpiegAnkl);
      EEPROM_writeAnything(sizeof(int) + sizeof(gpsWinkel) + sizeof(AutomVerriegeln) + sizeof(SpiegAnkl), NaviScale);
      EEPROM_writeAnything(sizeof(int) + sizeof(gpsWinkel) + sizeof(AutomVerriegeln) + sizeof(SpiegAnkl) + sizeof(NaviScale), BcResetten);
      EEPROM_writeAnything(sizeof(int) + sizeof(gpsWinkel) + sizeof(AutomVerriegeln) + sizeof(SpiegAnkl) + sizeof(NaviScale) + sizeof(BcResetten), Tippblinken);
      debugln("........................................... Schreibe ins EEPROM weil sich Variablen geändert haben");
    }

    // Aktualisiere die gespeicherten Zustände
    prevSthzON = SthzON;
    prevStat = stat;
    prevgpsAbstand = gpsAbstand;
    prevgpsWinkel = gpsWinkel;
    prevAutomVerriegeln = AutomVerriegeln;
    prevSpiegelAnklappen = SpiegAnkl;
    prevNaviScale = NaviScale;
    prevBcResetten = BcResetten;
    prevTippblinken = Tippblinken;


      // Set a callback to process the results of the PSD Action
    //mySARA.setPSDActionCallback(&processPSDAction);

    JsonDocument doc;  //(1024);
    // Füllen des JSON-Dokuments mit Daten
    // topic für Metrics
    doc[topic1e][topic2e1] = SthzON;
    doc[topic1e][topic2e2] = gpsAbstand;
    doc[topic1e][topic2e3] = gpsWinkel;
    //doc[topic1e][topic2e4] = stat; 
    doc[topic1e][topic2e5] = AutomVerriegeln;
    doc[topic1e][topic2e6] = SpiegAnkl;
    doc[topic1e][topic2e7] = NaviScale;
    doc[topic1e][topic2e8] = BcResetten;
    doc[topic1e][topic2e9] = Tippblinken;
    //doc[topic1e][topic2e10] = ??? Engine;

    // Erstellen des JSON-Strings
    String jsonData;
    serializeJsonPretty(doc, jsonData);
    // Konvertiere den Arduino-String in einen C-String
    const char* msg = jsonData.c_str();
    size_t msg_len = jsonData.length();
    // Publizieren des JSON-Strings
    String topic = myClientID + "/JSonData/Subs"; // Nutzen Sie hier einen geeigneten Topic
    //String topic = myClientID + "/subscribe/Tblink";                     // Um ein neuen Topic zu senden
    //mySARA.mqttPublishTextMsg(topic, jsonData.c_str(), 0, true);
    mySARA.mqttPublishBinaryMsg(topic, msg, msg_len, 0, true); // QoS = 0, retain = true
    debugln (".............................................................. sende Subscription daten Paket ");
  }
}

// hier werden die mqtt Daten zum iobroker nach MQTT.1 gesendet, ein JSskript verarbeitet
// den Json String und gibt ihn im iobroker unter 0_userdata.0.BMW-E38 einzeln wieder aus!
void publishMetricsData() {
  // Prüfen, ob sich eine der Variablen geändert hat
  if ( kwassertemp != prevKwassertemp || outTemp != prevAussentemp || lock != prevLock ||
    driverID != prevDriverID || Ignition != prevIgnition || (fabs(bat - prevBat) >= 0.2)  || avrFuel != prevAvrFuel ||
    avrSpeed != prevAvrSpeed || fuel != prevFuel || (fabs(speed - prevSpeed) >= 5) ||             //unter Verwendung der fabs-Funktion aus der cmath-Bibliothek, die den absoluten Wert einer Zahl zurückgibt. Damit wird die Bedingung als true ausgewertet, wenn sich speed um mindestens 5 ändert.
    stat != prevStat || remT != prevRemT) {

    //publishMetricsData();  // Funktion ausführen, wenn sich eine Variable geändert hat

    // Aktualisiere die gespeicherten Zustände
    prevKwassertemp = kwassertemp;
    prevAussentemp = outTemp;
    prevLock = lock;
    prevDriverID = driverID;
    prevIgnition = Ignition;
    prevBat = bat;
    prevAvrFuel = avrFuel;
    prevAvrSpeed = avrSpeed;
    prevFuel = fuel;
    prevSpeed = speed;
    prevRemT = remT;  // Verbleibende Zeit der Standheizung
    prevStat = stat;  // Status der Standheizung

      // Set a callback to process the results of the PSD Action
    mySARA.setPSDActionCallback(&processPSDAction);

    JsonDocument doc;  //(1024);
    // Füllen des JSON-Dokuments mit Daten
    // topic für Metrics
    doc[topic1a][topic2a1] = rpm;
    doc[topic1a][topic2a2] = speed; 
    doc[topic1a][topic2a3] = fuel;
    doc[topic1a][topic2a4] = avrSpeed;
    doc[topic1a][topic2a5] = avrFuel;
    doc[topic1a][topic2a6] = bat;
    doc[topic1a][topic2a7] = Ignition;
    doc[topic1a][topic2a8] = driverID;
    doc[topic1a][topic2a9] = lock;
    doc[topic1a][topic2a10] = outTemp;
    doc[topic1a][topic2a11] = kwassertemp;
    doc[topic1a][topic2a12] = strom;

    // topic für Standheizung
    doc[topic1b][topic2b1] = stat; // Status der Standheizung ist on or off
    doc[topic1b][topic2b2] = remT; // Verbleibende Laufzeit der Sthz

/*
    // Interne Daten
    doc[topic1d][topic2d1] = PsdIPAddress;  // PSD IP-Adresse (interne IP)
    doc[topic1d][topic2d2] = HardwareV;
    doc[topic1d][topic2d3] = SoftwareV;
    doc[topic1d][topic2d4] = Model_ID;
    doc[topic1d][topic2d5] = FirmwareV;
    doc[topic1d][topic2d6] = SIM_ID;
*/

    // Erstellen des JSON-Strings
    String jsonData;
    serializeJsonPretty(doc, jsonData);
    // Konvertiere den Arduino-String in einen C-String
    const char* msg = jsonData.c_str();
    size_t msg_len = jsonData.length();
    // Publizieren des JSON-Strings
    String topic = myClientID + "/JSonData/Metrics"; // Nutzen Sie hier einen geeigneten Topic
    //mySARA.mqttPublishTextMsg(topic, jsonData.c_str(), 0, true);
    mySARA.mqttPublishBinaryMsg(topic, msg, msg_len, 0, true); // QoS = 0, retain = true
    debugln (".............................................................. sende Metrics daten Paket ");
  }
}

// MQTT senden der Sara und Internen Daten nur einmal nach start
void publishSaraData() {
  // Prüfen, ob sich eine der Variablen geändert hat
  //if (SIM_ID != prevSIM_ID ) {
    JsonDocument doc;  //(1024);
    // Füllen des JSON-Dokuments mit Daten
    // Interne Daten
    doc[topic1d][topic2d1] = PsdIPAddress;  // PSD IP-Adresse (interne IP)
    doc[topic1d][topic2d2] = HardwareV;
    doc[topic1d][topic2d3] = SoftwareV;
    doc[topic1d][topic2d4] = Model_ID;
    doc[topic1d][topic2d5] = FirmwareV;
    doc[topic1d][topic2d6] = SIM_ID;

    // Erstellen des JSON-Strings
    String jsonData;
    serializeJsonPretty(doc, jsonData);
    // Konvertiere den Arduino-String in einen C-String
    const char* msg = jsonData.c_str();
    size_t msg_len = jsonData.length();
    // Publizieren des JSON-Strings
    String topic = myClientID + "/JSonData/Sara"; // Nutzen Sie hier einen geeigneten Topic
    //mySARA.mqttPublishTextMsg(topic, jsonData.c_str(), 0, true);
    mySARA.mqttPublishBinaryMsg(topic, msg, msg_len, 0, true); // QoS = 0, retain = true
    debugln (".............................................................. sende Sara daten Paket ");
}

// Nach PollGPSData und printGPS, werden GPS-Winkel und Distanz berechnet und nach Prüfung sende eine Json-Array an MQTT-Broker
void publishGpsData(double distance, double angle) {
  if ((distance > gpsAbstand && angle > gpsWinkel) || fix != prevFix) { // Wenn Winkel und Distanz Größer sind als die Vorgaben dann sende
    
    debug("publishGPS................................. Distance: ");
    debugln(distance); 
    debug("publishGPS................................. Winkel: ");
    debugln(angle);
    
    prevLat = lat;
    prevLon = lon;
    prevFix = fix;

    JsonDocument doc;  //(1024);
    JsonArray data = doc["GPS"].to<JsonArray>(); // Erstellt ein neues Array im Dokument unter dem Schlüssel "GPS"
    data.add(lat);
    data.add(lon);
    data.add(altitude);
    data.add(sat);
    data.add(fix);

    // Erstellen des JSON-Strings
    String jsonData;
    serializeJsonPretty(doc, jsonData);
    const char* msg = jsonData.c_str();
    size_t msg_len = jsonData.length();
    String topic = myClientID + "/JSonData/GPS"; // GPS spezifischer Topic
    mySARA.mqttPublishBinaryMsg(topic, msg, msg_len, 0, true); // QoS = 0, retain = true

    debugln(".............................................................. sende GPS Daten Paket");
  }
}

/*  ####################### GPS ################################################ */
boolean valid;
void PollGPSData() {
    // Call (mySARA.gpsGetRmc to get coordinate, speed, and timing data
    // from the GPS module. Valid can be used to check if the GPS is reporting valid data 
    if (mySARA.gpsGetRmc(&gps, &spd, &clk, &valid) == SARA_R5_SUCCESS)
    {
      controlLED(2, CRGB::Purple, 0);
      printGPS();
    }
    else
    {
      controlLED(2, CRGB::Red, 0);
    }
  debugln ("GPS.......................................hole GPS Daten.");
}

void printGPS(void){
 /*
  debugln();
  debugln("UTC: " + String(gps.utc));
  debug("Time: ");
  if (clk.time.hour < 10) debug('0'); // Print leading 0
  debug(String(clk.time.hour) + ":");
  if (clk.time.minute < 10) debug('0'); // Print leading 0
  debug(String(clk.time.minute) + ":");
  if (clk.time.second < 10) debug('0'); // Print leading 0
  debug(String(clk.time.second) + ".");
  if (clk.time.ms < 10) debug('0'); // Print leading 0
  debugln(String(clk.time.ms));
  debugln("Latitude: " + String(gps.lat, 7));
  debugln("Longitude: " + String(gps.lon, 7));
  debugln("Speed: " + String(spd.speed, 4) + " @ " + String(spd.cog, 4));
  debugln("Date (MM/DD/YY): " + String(clk.date.month) + "/" + 
    String(clk.date.day) + "/" + String(clk.date.year));
  debugln("Magnetic variation: " + String(spd.magVar));
  debugln("Status: " + String(gps.status));
  debugln("Mode: " + String(gps.mode));
  debugln();
 */
  fix = gps.status;                     // GPS-Status A=valid und V=invalid gps data

  if (fix =="A") {
    controlLED(2, CRGB::Blue, 2);    // LED 3 blinkt mit 2 Hz
    lat= gps.lat;
    lon= gps.lon;
    altitude = gps.alt;
    // Nachdem die neuen GPS-Daten da sind, gehe zu math_funktion.cpp und berechne distance und angle zwischen alten und neuen GPS-Daten
    distance = calculateDistance(prevLat, prevLon, lat, lon);
    angle = calculateAngle(prevLat, prevLon, lat, lon);
    publishGpsData(distance, angle); // sende die GPS-Daten
  } else {
    controlLED(2, CRGB::Red, 2);        // LED 3 
    lat=prevLat;  // Wenn keine gültigen GPS Werte kommen setze die letzten bekannten Werte ein
    lon=prevLon;
  }
}

// processMQTTcommandResult (+UUMQTTC) is provided to the SARA-R5 library via a 
// callback setter -- setMQTTCommandCallback. (See the end of setup())
void processMQTTcommandResult(int command, int result) {
    debugln();
    debug(F("MQTT Command Result:  command: "));
    debug(command);
    debug(F("  result: "));
    debugln(result);
    if (result == 0)
        debugln(F(" (fail)"));
    if (result == 1)
        debugln(F(" (success)"));

    // Fehlerprotokollierung
    int error_class;
    int error_code;
    mySARA.getMQTTprotocolError(&error_class, &error_code);
    debug(F("MQTT protocol error:  class: "));
    debug(error_class);
    debug(F("  code: "));
    debugln(error_code);
}

// Aktiviere das PSD-Profil mit dem Befehl AT+UPSDA=0,3
void activateInternet(){
  if (mySARA.performPDPaction(0, SARA_R5_PSD_ACTION_ACTIVATE) == SARA_R5_SUCCESS) {
    // Wenn die Aktivierung erfolgreich war, gib eine Erfolgsmeldung aus
    debugln(F("PSD profile activation successful!"));
    controlLED(0, CRGB::Green, 0);
  } else {
    // Wenn die Aktivierung fehlschlägt, informiere den Benutzer und friere das Programm ein
    debugln(F("Activate Profile failed! Möglicherweise schon aktiv, es geht weiter......."));
    controlLED(0, CRGB::Red, 0);
  }
}

// processPSDAction is provided to the SARA-R5 library via a 
// callback setter -- setPSDActionCallback. (See setup())
void processPSDAction(int result, IPAddress ip)
{
  debugln();
  debug(F("PSD Action:  result: "));
  debug(String(result));
  if (result == 0){
    debug(F(" (success)"));
  } else {
    controlLED(1, CRGB::Red, 4);
    if (mySARA.performPDPaction(0, SARA_R5_PSD_ACTION_ACTIVATE) == SARA_R5_SUCCESS) {
        // Wenn die Aktivierung erfolgreich war, gib eine Erfolgsmeldung aus
        debugln(F("PSD profile activation successful!"));
        controlLED(1, CRGB::Green, 0);
    } else {
        // Wenn die Aktivierung fehlschlägt, informiere den Benutzer und friere das Programm ein
        debugln(F("Activate Profile failed! Möglicherweise schon aktiv, es geht weiter......."));
        controlLED(1, CRGB::Red, 0);
    }
  }

  // IP-Adresse in einen String umwandeln
  PsdIPAddress = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);

  // Debuggen der gespeicherten IP-Adresse im String-Format
  debug(F("Stored IP Address as String: "));
  debugln(PsdIPAddress);
}

// Auswertung des Pings
void pingResponseCallback(int retry, int p_size, String remote_hostname, IPAddress ip, int ttl, long rtt) {
    debug("Ping Response from ");
    debug(remote_hostname);
    debug(" [");
    debug(ip);
    debug("], ");
    debug("size=");
    debug(p_size);
    debug(" bytes, ");
    debug("ttl=");
    debug(ttl);
    debug(", ");
    debug("rtt=");
    debug(rtt);
    debugln(" ms");
}

// URC Auswertung von +UUMQTTC
void mqttCommandResultCallback(int command, int result) {
  if (command == 1 && result == 1) {
    debugln(".......................................................Command 1.1 MQTT Connect erfolgreich ausgeführt.");
    controlLED(1, CRGB::DarkOrange, 0);
    char responseBuffer[128];  // Speicher für die Antwort des Moduls
    if (mySARA.sendCustomCommandWithResponse(
      "+UMQTTC=8,1",      //op_code / Ping on/off 
      "OK",               // Erwartete Antwort (könnte auch z.B. "ERROR" oder eine andere Teilantwort sein)
      responseBuffer,     // Speicher, in den die Antwort geschrieben wird
      1000                // Timeout von 1000ms (1 Sekunde)
      )== SARA_R5_SUCCESS) {
      debugln("..........................PING successful ");
    } else {
      debug(".........................PING Command failed: ");
      debugln(responseBuffer);  // Druckt die Antwort auf die Konsole
    }
    mqttSubscribe();         
  } else if (command == 1 && result == 0) {
    debugln(".......................................................Command 1.0 MQTT Connect ERROR.");
    controlLED(1, CRGB::DarkRed, 2);
    mqttConnectTimer.start(); // 1 sec. Delay dann startet ein mqttConnect
  } else if (command == 0 && result == 1) {
    debugln(".......................................................Command 0.1 MQTT disconnected.");
    controlLED(2, CRGB::DarkRed, 2);
    mqttConnectTimer.start(); // 1 sec. Delay dann startet ein mqttConnect
  } else if (command == 0 && result == 102) {
    debugln(".......................................................Command 0 mit Fehler 102.");
    controlLED(1, CRGB::DarkRed, 2);
    mqttConnectTimer.start(); // 1 sec. Delay dann startet ein mqttConnect
  } else if (command == 0 && result == 100) {
    debugln(".......................................................Command 0 mit Fehler 100 - Timeout.");
    controlLED(1, CRGB::DarkRed, 2);
    mqttConnectTimer.start(); // 1 sec. Delay dann startet ein mqttConnect
  } else if (command == 0 && result == 101) {
    debugln(".......................................................Command 0 mit Fehler 101 - network connection lost .");
    controlLED(1, CRGB::DarkRed, 2);
    activateInternet();
    mqttConnectTimer.start(); // 1 sec. Delay dann startet ein mqttConnect
  } else if (command == 8 && result == 0) {
    debugln(".......................................................URC 8.0 no PING Answer .");
    controlLED(1, CRGB::DarkRed, 3);
    mqttConnectTimer.start(); // 1 sec. Delay dann startet ein mqttConnect
  } else if (command == 4 && result == 1) {
    debugln(".......................................................URC 4.1 Subscribe OK .");
    publishSaraTimer.start();  // um einmalig die Sara-Daten zu senden
  } else if (command == 6 && result >= 1) {             // wenn mehr als eine Nachricht im Speicher liegt....
    debugln("...................................................... URC 6.1 - MQTT Message in the Box .");
    controlLED(1, CRGB::BlueViolet, 8);
    readMqttTimer.start();  // 1 Sec, readMqttData() 
    //readMqttData();
  }
}

// Connect to MQTT +UMQTTC=1
void mqttConnect(){
    char responseBuffer[128];  // Speicher für die Antwort des Moduls
    if (mySARA.sendCustomCommandWithResponse(
      "+UMQTT=10,60,20",   //op_code / Timeout 90 sec. / Linger time 20 sec
      "OK",               // Erwartete Antwort (könnte auch z.B. "ERROR" oder eine andere Teilantwort sein)
      responseBuffer,     // Speicher, in den die Antwort geschrieben wird
      1000                // Timeout von 1000ms (1 Sekunde)
      ) == SARA_R5_SUCCESS) {
      debugln("......... Timeout-Command successful");
    } else {
      debug("Command failed: ");
      debugln(responseBuffer);  // Druckt die Antwort auf die Konsole
    }

  if (mySARA.connectMQTT() == SARA_R5_SUCCESS){
    debugln(F("MQTT connect: AT+OK"));  
  } else {
    debugln(F("MQTT connect: AT+ERROR!"));
  }
}

void mqttdisconnect(){
  if (mySARA.disconnectMQTT() == SARA_R5_SUCCESS){
    debugln(F("MQTT DISconnect: AT+OK"));
  } else {
    debugln(F("MQTT DISconnect: AT+ERROR!"));
  }
}

void checkNetworkRegistration() {
  //debugln("##########..............###########..............Check Rregistation Status.");
  SARA_R5_registration_status_t status = mySARA.registration(); //Command: +CEREG?

  if (status == SARA_R5_REGISTRATION_NOT_REGISTERED) {
    debugln("Not registered with the network.");
  } else if (status == SARA_R5_REGISTRATION_SEARCHING) {
    debugln("Searching for network...");
  } else if (status == SARA_R5_REGISTRATION_HOME) {
    debugln("Registered on home network.");
  } else if (status == SARA_R5_REGISTRATION_ROAMING) {
    debugln("......................................... Registered on roaming network.");
  } else {
    debugln("Registration status unknown.");
  }
}

// Callback-Funktion zur Verarbeitung des Registrierungsstatus (Command: +CREG=2)
void registrationCallback(SARA_R5_registration_status_t status, unsigned int lac, unsigned int ci, int Act) {
  debugln("##########.......................................Check Rregistation (+CREG).");
  if (status == SARA_R5_REGISTRATION_NOT_REGISTERED) {
    debugln("Callback: Not registered with the network.");
  } else if (status == SARA_R5_REGISTRATION_SEARCHING) {
    debugln("Callback: Searching for network...");
  } else if (status == SARA_R5_REGISTRATION_HOME) {
    debugln("Callback: Registered on home network.");
  } else if (status == SARA_R5_REGISTRATION_ROAMING) {
    debugln("Callback: Registered on roaming network.");
  } else if (status == SARA_R5_REGISTRATION_UNKNOWN) {     // +CREG: 4  
    debugln("Callback: SMS-Only on roaming network."); 
    activateInternet();
  } else if (status == SARA_R5_REGISTRATION_ROAMING_SMS_ONLY) {     // +CREG: 7  
    debugln("Callback: SMS-Only on roaming network.");
    activateInternet();
  } else {
    debugln("Callback: Registration status unknown.");
  }

  // Optional: Ausgabe der zusätzlichen Parameter
  debug("LAC: ");
  debugln(lac);
  debug("CI: ");
  debugln(ci);
  debug("Access Technology (Act): ");
  debugln(Act);
}

void readRssi(){
  int rssi = mySARA.rssi();
  debugln(".................................................. Current RSSI: " + String(rssi));

  if (rssi == 99) {  // 99 bedeutet normalerweise "Signal nicht verfügbar"
    debugln("................................................Signal lost or weak!");
  }
}

// Subscribe to the channel topic
void mqttSubscribe(){
  String subscribeTopic = myClientID + "/subscribe/#";      // "/subscribe/#" alles was nach /subscribe kommt wird gelesen
  int maxAttempts = 10;
  int attempt = 0;
  bool subscribed = false;
  while(attempt < maxAttempts && !subscribed) {
    if (mySARA.subscribeMQTTtopic(2, subscribeTopic) == SARA_R5_SUCCESS) { // QoS = 0
      debugln(F("MQTT subscribe: success"));
      controlLED(1, CRGB::Green, 0);
      subscribed = true;
    } else {
      attempt++;
      debug(F("MQTT subscribe: failed! Attempt "));
      debug(attempt);
      debugln(F(" of 10"));
      controlLED(1, CRGB::DarkGoldenrod, 5);
      //mySARA.bufferedPoll(); 
      delay(1000); // Warte 1 Sekunde vor dem nächsten Versuch
    }
  }

  if (!subscribed) {
    debugln(F("Failed to subscribe after 10 attempts."));
    controlLED(1, CRGB::Red, 2);
  }
}

// Timer für die Standheizung und EIN/AUS
void updateSthzTimer() {
  // Wenn Standheizung eingeschaltet ist
  if (SthzON == 1) {
    if (!SthztimerRunning) {
      // Starte den Timer und die Standheizung
      SthztimerRunning = true;
      ibusTrx.write(SthzEIN);
      digitalWriteFast(SthzRelais, HIGH);
      debugln("Standheizung ON");
      SthzMinuteTimer.start();
    } else if (remT > 0) {
      // Reduziere verbleibende Zeit, wenn Timer bereits läuft
      remT--;
      debugln("Standheizung ist noch " + String(remT) + " min. an.");
    }
    
    // Wenn die verbleibende Zeit abgelaufen ist
    if (remT == 0) {
      SthzON = 0;
      remT = SthzRuntime;
      SthztimerRunning = false;
      ibusTrx.write(SthzAUS);
      digitalWriteFast(SthzRelais, LOW);
      SthzMinuteTimer.stop();
      publishSubData();
      debugln("Standheizung AUS, Timer bei 0 Minuten.");
    }
  } else if (SthztimerRunning) {
    // Wenn Standheizung ausgeschaltet ist und Timer noch läuft
    remT = SthzRuntime;
    SthztimerRunning = false;
    SthzON = 0;
    ibusTrx.write(SthzAUS);
    digitalWriteFast(SthzRelais, LOW);
    SthzMinuteTimer.stop();
    debugln("Standheizung AUS, Timer auf 25 Minuten zurückgesetzt.");
  }
}




 
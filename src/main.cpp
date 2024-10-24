/*
Sara-R5 mit Sparkfun Lib und iBusTrx Lib
Sara working Mqtt 3.11 Verbindung mit SSL/TTL zum ioBroker, GPS-Daten, Neopixels
erster Versuch mit Ibus, HW-Version 0.5
by AME 24/10/2024 HW 0.5
*/

#include <Arduino.h>
#include <IPAddress.h>          // um IP-Adressen darzustellen
#include "sara-lte.h"           // Funktionen für Sara-r5 LTE
#include "globals.h"            // globale Variablen
#include "mqtt_secrets.h"       // mqtt Zugang
#include <SparkFun_u-blox_SARA-R5_Arduino_Library.h>      // http://librarymanager/All#SparkFun_u-blox_SARA-R5_Arduino_Library
#include "neopixel.h"           // Funktiuonen für die Neopixel LEDs
#include "timer.h"              // Timer Lib um Delay zu vermeiden
#include "timerManager.h"       // Timer Lib um Delay zu vermeiden
#include <EEPROM.h>             // zum speichern der Variablen in das EEprom
#include "math_functions.h"     // Math Funktion zum berechnen der GPS Entfernung und Winkel
/* ibus libs */
#include <IbusTrx.h>            // IbusTrx library selbst abgeändert und erweitert
#include <Wire.h>
#include "IbusCodes.h"          // hier sind alles Ibus-Codes und Variablen zur Configuration
#include "bluetooth.h"          // Bluetooth funktionen
//#include <Snooze.h>             // Teensy Snooze Lib


//SnoozeUSBSerial usb;
//SnoozeDigital digital;                  // this is the pin's wakeup driver
//SnoozeBlock config_sleep(usb, digital); // install driver into SnoozeBlock

SARA_R5 mySARA(Sara_PWR_ON, Sara_RST, 2);   // init of Sara-R5, PWR-ON-Pin, RST-Pin, maximum number of initialization attempts

Timer pollGpsTimer;           // pollGPSdata
Timer publishSaraTimer;       // einmalig die Sara-Daten zu senden
Timer readMqttTimer;          // readMqttData
Timer readVoltageTimer;       // INA219
Timer BluetoothToggleTimer;   // Bluetooth ein/aus
Timer mqttConnectTimer;       // mqtt Connect Delay
Timer readRssiTimer;          // read Rssi
Timer mqttdisconnectTimer;    // Intervall Disconnect from mqtt
Timer SthzMinuteTimer;        // Minuten Timer für die Standheizung

/***************************************
*********** SET-UP SECTION *************
****************************************/
void setup() {
  /* Neopixel Setup: */
  FastLED.addLeds<LED_TYPE, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  controlLED(0, CRGB::DarkMagenta, 0);    // LED 1 Network
  controlLED(1, CRGB::DarkMagenta, 0);    // LED 2 MQTT
  controlLED(2, CRGB::DarkMagenta, 0);    // LED 3 GPS

  /* ######### PIN und UART ########################################### */
  //pinMode(Sara_PWR_ON, INPUT);        // PWR_ON ist ein Output der in der sparkfun lib configuriert wird, wird hier jedoch auf high-Z gestellt, nur für V0.4
  //pinMode(Sara_RST, INPUT);           // RST ist ein Output, wird hier jedoch auf high-Z gestellt, nur für V0.4
  pinMode(Sara_RI, INPUT_PULLUP);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(SthzRelais,OUTPUT);
  //pinMode(ledPin, OUTPUT);
  pinMode(TH_EN,OUTPUT);                                  // TH3122 enable pin
  pinMode(senSta, INPUT_PULLUP);                          // pin 5 des TH3122 für Clear to send, mit einer diode von Teensy pin > TH3122 pin5
  pinMode(TH_RESET, INPUT_PULLUP);                        // Reset pin des TH3122, dieser ist high wenn TH arbeitet, mit einer diode von Teensy pin > TH3122 pin4
  //wakeup Interrupt: pin, mode, type         
  //  digital.pinMode(TH_RESET, INPUT_PULLUP, RISING);        // SnoozeBlock, wakeup Pin. Wenn TH_RESET high wird, wird der Teensy aufgeweckt
  pinMode(sys_ctl, OUTPUT);           // BT sys_ctl Pin zum Ein/Ausschalten des Modules
  pinMode(PCM_EN, OUTPUT);            // Enable LDOs für PCM I2S Power

  debugbegin(115200);
  delay(100); // Kurze Verzögerung, um sicherzustellen, dass das Gerät bereit ist
  debugln("_________ Debugausgabe des iBus Modules V0.5 _________");
  delay(1000);
  RI_state_last = digitalReadFast(Sara_RI);
  
  // Initialize the SARA-UART Parameter: Serialx, Baud, rtsPin, ctsPin
  if (mySARA.begin(SaraCom, 115200) ) {
    debugln(F("SARA-R5 connected!"));
    controlLED(0, CRGB::Yellow, 0);
  }  
  SaraCom.attachRts(18);
  SaraCom.attachCts(19); // Pin 19 ist der CTS default Pin for Serial3

  debugln();

  /* ***************** iBUS Staff **************************************************** */
  ibusTrx.begin(ibusPort);                                // Hardware Serial Nr. an IbusTrx übergeben
  ibusTrx.senStapin(senSta);                              // senSta Pin an IbusTrx Library übergeben
  attachInterrupt(digitalPinToInterrupt(senSta), ClearToSend, FALLING);   // Interrupt: Wenn senSta LOW wird, gehe zu Funktion ClearToSend
  delay (200);

  Wire1.begin();                                           // I²C init für Lichtsensor BH1750 und PCMD3140
  if (!lightMeter.begin(BH1750::CONTINUOUS_LOW_RES_MODE, 0x23, &Wire1)) {
    debugln("***** BH1750 initialization failed! *****");              // initialise the light Sensor in continius low Resolution Mode
  }
  
  // TH3122  Reset-Status-Abfrage
    //if (digitalRead(TH_RESET) == LOW)
    //{
      //debugln("TH schläft, wecke ihn auf!");
      //th_reset = true;
    //}  

  // TH3122 einschalten und flags setzen
  digitalWrite(TH_EN, HIGH);      // TH3122 enable pin high
  sysSleep = false;               // SystemSleep =false -> System ist aktiv
  msSleep =millis();              // Sleep Timer reset

  debugln("_______________________________________");
  debugln(F("_________ iBusTrx on Teensy 4.0 _________"));
  debugln("_______________________________________"); // iBus lebt


  // ############################ bluetooth Stuff #######################################################
 
  BTcom.begin(115200); // Starte die Serielle Schnittstelle BTcom für die Kommunikation mit dem Gerät
  delay(500); // Kurze Verzögerung, um sicherzustellen, dass das Gerät bereit ist
  debugln("_________ Hallo Bluetooth _________");

  #ifdef BTConfig
    digitalWrite(sys_ctl, HIGH);
    delay(100);
    // Modul einschalten
    digitalWrite(sys_ctl, LOW);
    delay(500); // Kurze Verzögerung, um sicherzustellen, dass das Gerät bereit ist
    digitalWrite(sys_ctl, HIGH);
    delay(100);
    unsigned long BTstartTime = millis();  // Startzeit für den Notausstieg
    unsigned long timeoutDuration = 5000;  // Timeout-Dauer: 5 Sekunden
    debugln("load Config");
    // BT-Modul configurieren
    while (true) {  // Endlose Schleife bis richtige Antwort oder Timeout
      if (BTcom.available()) {
        String response = BTcom.readStringUntil('\n');  // Empfangene Daten als String
        response.trim();  // Entfernt führende und nachfolgende Leerzeichen und neue Zeilenzeichen

        if (response.startsWith("+PROFILE=")) {
          String profileValue = response.substring(9);  // Extrahiere den Wert nach "+PROFILE="

          if (profileValue == "1187") {
            debugln("BT-Modul richtig initialisiert");
            break;  // Beende die Schleife, da die Initialisierung korrekt war
          } else {
            debugln("Falsches Profil, starte Initialisierung...");
            
            // Modul Initialisieren
            BTcom.println("AT+NAME=BMW E38");
            delay(100);
            BTcom.println("AT+I2SCFG=69");
            delay(100);
            BTcom.println("AT+PROFILE=1187");
            delay(100);
            BTcom.println("AT+COD=240408");

            debugln("BT-Modul initialisiert");
            break;  // Beende die Schleife nach der Initialisierung
          }
        }
      }
      // Überprüfe, ob 5 Sekunden vergangen sind
      if (millis() - BTstartTime >= timeoutDuration) {
        debugln("Keine Antwort vom BT-Modul erhalten - Timeout!");
        break;  // Beende die Schleife bei Timeout
      }
    }
    delay(1000);
    // Modul auschalten
    digitalWrite(sys_ctl, LOW);
    delay(500); // Kurze Verzögerung, um sicherzustellen, dass das Gerät bereit ist
    digitalWrite(sys_ctl, HIGH);

    debugln("Bluetooth Modul ist einsatzbereit, jedoch AUS!!!");
  #endif

  // ########################### INA219 Stromsensor Stuff #####################################
   if(!ina219.init()){
    debugln("INA219 not connected!");
   // while(1);
  }
  ina219.setMeasureMode(TRIGGERED);     // für Details siehe Example Comtinius.ino
  ina219.setADCMode(BIT_MODE_10);       // 10 bit Mode damit die Messung schnell ist
  ina219.setPGain(PG_40);               // Auflösung bis 400mA bei shunt 0.1 OHM, bei 0,05 OHM 800mA
  ina219.setBusRange(BRNG_16);

  // ######################## SARA  Debugging on-Off: #####################################################
  mySARA.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial
  //------------------------------ SARA  Connect to the Operator -----------------------------------------------------
  String currentOperator = "";
  bool isConnected = false;
  // Versuche kontinuierlich sich mit einem Betreiber zu verbinden
  while(!isConnected) {
    // Überprüfe, ob wir mit einem Betreiber verbunden sind
    if (mySARA.getOperator(&currentOperator) == SARA_R5_SUCCESS) {
      debug(F("Sara connected to: "));
      debugln(currentOperator);
      controlLED(0, CRGB::DarkGreen, 0);
      isConnected = true;  // Verbindung erfolgreich, beende die Schleife
    } else {
      debugln(F("The SARA is not yet connected to an operator. Retrying..."));
      controlLED(0, CRGB::LightGoldenrodYellow, 0);
      delay(4000);  // Warte 4 Sekunden vor dem nächsten Verbindungsversuch
    }
  }

  // ************************* SARA PSD Profile ******************************************************* 
  // Deactivate the profile
  if (mySARA.performPDPaction(0, SARA_R5_PSD_ACTION_DEACTIVATE) != SARA_R5_SUCCESS){
    debugln(F("..........................................Warning: performPDPaction (deactivate profile) failed. Probably because no profile was active."));
  }

  // Map PSD profile 0 to the 1 CID (hologram), AT+UPSD=0,100,1
  if (mySARA.setPDPconfiguration(0, SARA_R5_PSD_CONFIG_PARAM_MAP_TO_CID, 1) != SARA_R5_SUCCESS){
    debugln(F("......................................... setPDPconfiguration (map to CID) failed!"));
    //controlLED(2, CRGB::GreenYellow, 0);
  }

  // Set the protocol type - this needs to match the defined IP type for the CID (as opposed to what was granted by the network)
  // AT+UPSD=0,0,0 -> PSD profile 0 und IP V4
  if (mySARA.setPDPconfiguration(0, SARA_R5_PSD_CONFIG_PARAM_PROTOCOL, SARA_R5_PSD_PROTOCOL_IPV4) != SARA_R5_SUCCESS){
  // You _may_ need to change the protocol type: ----------------------------------------^
    debugln(F("..........................................setPDPconfiguration (set protocol type) failed!"));
  }

  /*
  // Speichere das aktuelle PSD-profil auf platz 0, AT+UPSDA=0,1
  if (mySARA.performPDPaction(0, SARA_R5_PSD_ACTION_STORE) == SARA_R5_SUCCESS)
  {
    debugln(F("PSD Profil gespeichert"));
  }
  */
  // Aktiviere das PSD-Profil mit dem Befehl AT+UPSDA=0,3
  activateInternet();
  
  // Set a callback to process the results of the PSD Action
  mySARA.setPSDActionCallback(&processPSDAction);

  for (int i = 0; i < 10; i++){
    mySARA.bufferedPoll(); // Keep processing data from the SARA
    delay(10);
  }

  /* ##################################### SARA MQTT ########################################################### */
  // Setze den MQTT Command Callback - wichtig
  mySARA.setMQTTCommandCallback(mqttCommandResultCallback); // URC Auswertung von +UUMQTTC
  // Konfigurieren des Sicherheitsprofils mit dem Root-CA-Zertifikat
  mySARA.configSecurityProfileString(secProfileNum, SARA_R5_SEC_PROFILE_PARAM_ROOT_CA, "ca.crt");
  // Optional: Konfigurieren des Sicherheitsprofils mit Client-Zertifikat und privatem Schlüssel
  mySARA.configSecurityProfileString(secProfileNum, SARA_R5_SEC_PROFILE_PARAM_CLIENT_CERT, "client.crt");
  mySARA.configSecurityProfileString(secProfileNum, SARA_R5_SEC_PROFILE_PARAM_CLIENT_KEY, "client.key");
  // Set up the MQTT command callback, siehe sara-lte.cpp -> void processMQTTcommandResult(int command, int result) {
  // mySARA.setMQTTCommandCallback(processMQTTcommandResult);
  mySARA.setMQTTsecure(true, secProfileNum);                    // Sicherheitsprofil für die MQTT-Verbindung aktivieren
  mySARA.setMQTTclientId(myClientID);                           // Set Client ID (BMW7er)
  mySARA.setMQTTserver(brokerName, brokerPort);                 // Set the broker name and port
  mySARA.setMQTTcredentials(myUsername, myPassword);            // Set the user name and password, nicht notwendig
 
  mqttConnect();

  for (int i = 0; i < 100; i++)  // Wait for ~1 seconds
  {
    mySARA.bufferedPoll(); // Keep processing data from the SARA
    delay(10);
  }

  // Initialisiere alle "prev" Variablen mit den Startwerten
  prevPsdIPAddress = PsdIPAddress;
  prevFix = fix;
  prevSat = sat;
  prevLat = lat;
  prevLon = lon;
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
  prevRpm = rpm;
  prevRemT = remT;
  prevStat = stat;

  //Set Timer in milliseconds  ++++++++++++++++++ TIMER ++++++++++++++++++++++++++++
  pollGpsTimer.setInterval(15000);                // PollGPSData, hole alle 15sec. GPS daten
  publishSaraTimer.setTimeout(40000);             // 40 sec um einmalig die Sara-Daten zu senden   
  readMqttTimer.setTimeout(900);                  // 200ms, readMqttData  
  readVoltageTimer.setInterval(60010);            // INA219 Spannung- Stromsensor
  BluetoothToggleTimer.setTimeout(500);           // Bluetooth ein/aus
  mqttConnectTimer.setTimeout(1000);              // mqtt reConnect Delay 1 sec.
  readRssiTimer.setInterval(3000);
  mqttdisconnectTimer.setInterval(3600000);        // (60min) intervall disconnect from mqtt, 1h = 3600000
  SthzMinuteTimer.setInterval(60000);              // Minuten Timer für die Standheizung

  
  pollGpsTimer.setCallback(PollGPSData);          // PollGPSData()
  publishSaraTimer.setCallback(publishSaraData);  // 40 sec um einmalig die Sara-Daten zu senden  
  readMqttTimer.setCallback(readMqttData);        // 200ms, readMqttData 
  readVoltageTimer.setCallback(readINA219);       // Spannung- Stromsensor
  BluetoothToggleTimer.setCallback(BT_Toggle);    // Bluetooth ein/aus
  mqttConnectTimer.setCallback(mqttConnect);      // mqtt Connect Delay
  readRssiTimer.setCallback(readRssi);
  mqttdisconnectTimer.setCallback(mqttdisconnect);  // intervall disconnect from mqtt
  SthzMinuteTimer.setCallback(updateSthzTimer);     // Minuten Timer für die Standheizung


  pollGpsTimer.start();                           // PollGPSData
  //publishSaraTimer.start();                     // um einmalig die Sara-Daten zu senden / wird in funktion "void mqttCommandResultCallback" aufgerufen
  //readMqttTimer.start();                        // readMqttData() wird in funktion "void mqttCommandResultCallback" aufgerufen
  readVoltageTimer.start();                       // Spannung- Stromsensor
  // readRssiTimer.start();
  mqttdisconnectTimer.start();                    // intervall disconnect from mqtt

 /* ####################### GPS ########################################### */
   // Enable power for the GNSS active antenna. The SARA GPIO2 pin is used to control power for the antenna. We need to pull GPIO2 (Pin 23) high to enable the power.
  mySARA.setGpioMode(mySARA.GPIO2, mySARA.GPIO_OUTPUT, 1);

  // From the u-blox SARA-R5 Positioning Implementation Application Note UBX-20012413 - R01
  // To enable the PPS output we need to:
  // Configure GPIO6 for TIME_PULSE_OUTPUT - .init does this
  // Enable the timing information request with +UTIMEIND=1 - setUtimeIndication()
  // Reset the time offset configuration with +UTIMECFG=0,0 - setUtimeConfiguration()
  // Request PPS output using GNSS+LTE (Best effort) with +UTIME=1,1 - setUtimeMode()
  // The bits that don't seem to be mentioned in the documentation are:
  //   +UTIME=1,1 also enables the GNSS module and so we don't need to call gpsPower.
  //   +UTIME=1,1 only works when the GNSS module if off. It returns an ERROR if the GNSS is already on.

  // Enable the timing information request (URC)
  mySARA.setUtimeIndication(); // Use default (SARA_R5_UTIME_URC_CONFIGURATION_ENABLED)
  
  // Clear the time offset
  mySARA.setUtimeConfiguration(); // Use default offset (offsetNanoseconds = 0, offsetSeconds = 0)
  
  // Set the UTIME mode to pulse-per-second output using a best effort from GNSS and LTE
  mySARA.setUtimeMode(); // Use defaults (mode = SARA_R5_UTIME_MODE_PPS, sensor = SARA_R5_UTIME_SENSOR_GNSS_LTE)

  mySARA.gpsEnableRmc(); // Enable GPRMC messages

  // Lese die Werte aus dem EEPROM
  EEPROM_readAnything(0, gpsAbstand);
  EEPROM_readAnything(sizeof(int), gpsWinkel);
  EEPROM_readAnything(sizeof(int) + sizeof(gpsWinkel), AutomVerriegeln);
  EEPROM_readAnything(sizeof(int) + sizeof(gpsWinkel) + sizeof(AutomVerriegeln), SpiegAnkl);
  EEPROM_readAnything(sizeof(int) + sizeof(gpsWinkel) + sizeof(AutomVerriegeln) + sizeof(SpiegAnkl), NaviScale);
  EEPROM_readAnything(sizeof(int) + sizeof(gpsWinkel) + sizeof(AutomVerriegeln) + sizeof(SpiegAnkl) + sizeof(NaviScale), BcResetten);
  EEPROM_readAnything(sizeof(int) + sizeof(gpsWinkel) + sizeof(AutomVerriegeln) + sizeof(SpiegAnkl) + sizeof(NaviScale) + sizeof(BcResetten), Tippblinken);
  if (gpsAbstand < 10){ gpsAbstand =10;}

  // Holen von internen Sara Informationen
  Model_ID = String(mySARA.getModelID());
  FirmwareV = String(mySARA.getFirmwareVersion());
  SIM_ID = String(mySARA.getCCID());

  checkNetworkRegistration();
    
  mySARA.setRegistrationCallback(registrationCallback);   // Setze den Registrierungs-Callback (Command: +CREG=2)
} //############## Ende Setup ###############################################################
  

/***************************************
*********** LOOP SECTION *************
****************************************/
void loop() {
  updateLEDs();                       // Neopixel LEDs aktualisieren
  publishMetricsData();               // MQTT, Publiziere alle Fahrzeugdaten (Metrics)
  TimerManager::instance().update();  // Update all the timers at once
  mySARA.bufferedPoll();              // Keep processing data from the SARA / Schau nach op daten vorhanden sind
    

  /* ***************** iBUS Staff **************************************************** */
  Daemmerung();     // gehe zu Dämmerung und messe Helligkeit und ggf schalte Heimleuchten ein
  iBusMessage();    // iBus Message auswerten, wenn vorhanden

  // ############################# weiter im Loop ##############
  // Text im IKE löschen
  if (IKEclear)                               // bereit um den Text im IKE zu löschen
  { 
    if (speed > 30)
    {
      ibusTrx.write(cleanIKE);                // lösche den Text
      IKEclear=false;                         // Fertig
    }
    /*
    if ((millis()-msTimer) >= t_clearIKE)     // Zeit abgelaufen?
    {      
      ibusTrx.write(cleanIKE);                // lösche den Text
      IKEclear=false;                         // Fertig
    }*/
  }
  
  if (AutomVerriegeln)                        // Automatisches Verriegln bei Geschwindigkeit > 30Km/h und Entriegeln bei Motor aus
  {
    if (!ZVlocked && (speed > 20) && Ignition)   // wenn speed größer 20 km/h dann ZV verriegeln
    {
      ibusTrx.write(ZV_lock);
      ZVlocked =true;
      debugln("Locked");
    }
    if (!Ignition && ZVlocked)                     // Motor aus / Zündung aus dann Entriegeln
    {
      ibusTrx.write(ZV_lock);
      ZVlocked =false;
      debugln("Unlocked");
    }
  }
  
  // In real testen obe es auch ohne diese Abfrage hier funktioniert
  // Überprüfe, ob das CTS-Signal LOW ist
  if (!digitalReadFast(senSta)){ClearToSend();}

  /* Debug abfrage status TH Reset Pin
  if ((!th_reset) && ( digitalReadFast(TH_RESET)))
  {
    debugln("TH Reset high");
    th_reset = true;
  }
  if ((th_reset) && ( !digitalReadFast(TH_RESET)))
  {
    debugln("TH Reset low");
    th_reset = false;
  }
  */

  // ######################### Bluetooth Stuff ######################################
    // Überprüfe, ob Daten auf BTcom verfügbar sind (Antwort des Geräts)
  if (BTcom.available()) {
    char receivedChar = BTcom.read();
    //DEBUG.write(receivedChar);
    receivedData += receivedChar;

    if (receivedData.endsWith("+TRACKINFO")) {
      trackInfoReceived = true;
      receivedData = "";
    } else if (receivedData.endsWith("+TRACKSTAT")) {
      trackStatReceived = true;
      receivedData = "";
    } else if (receivedData.endsWith("+PLAYSTAT")) {
      playStatReceived = true;
      receivedData = "";
    } 

    if (trackInfoReceived && receivedChar == '\n') {
      parseTrackInfo(receivedData);
      receivedData = "";
      trackInfoReceived = false;
    } else if (trackStatReceived && receivedChar == '\n') {
      parseTrackStat(receivedData);
      receivedData = "";
      trackStatReceived = false;
    } else if (playStatReceived && receivedChar == '\n') {
      parsePlayStatus(receivedData);
      receivedData = "";
      playStatReceived = false;
    }
  }

} // Ende loop










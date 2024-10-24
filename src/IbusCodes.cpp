// ibusCodes.cpp

#include "IbusCodes.h"
#include "globals.h"
#include <BH1750.h>             // GY-302 mit Licht-Sensor BH1750 für Dämmerungsschalter benötigt Wire.h
#include <IbusTrx.h> 


BH1750 lightMeter;              // Falls der Sensor global definiert ist
IbusTrx ibusTrx;                // IbusTrx instance


void iBusMessage(){
  // if there's a message waiting, check it out
  if (ibusTrx.available()){
    // read the incoming message (this copies the message and clears the receive buffer)
    IbusMessage message = ibusTrx.readMessage();

    /* the following addresses are defined in the IbusTrx library:
    M_GM5 = 0x00;  // GM5: body control module
    M_NAV = 0x3B;  // NAV: Navigation / Monitor / Bordcomputer
    M_DIA = 0x3F;  // DIA: diagnostic computer
    M_EWS = 0x44;  // EWS: immobilizer
    M_MFL = 0x50;  // MFL: steering wheel controls
    M_IHKA = 0x5B; // IHKA: climate control panel
    M_RAD = 0x68;  // RAD: radio module
    M_IKEC = 0x80; // IKE: instrument cluster
    M_ALL = 0xBF;  // ALL: broadcast message  (GLO = Global)
    M_TEL = 0xC8;  // TEL: telephone module
    M_LCM = 0xD0;  // LCM: light control module
    M_IKET = 0x30; // IKE TextFeld (Großes Display)
    M_BMB = 0xF0;  // BMB: Bordmonitor Buttons
    M_DSP = 0x6A;  // DSP: Digital Sound Processor
    */
    
    // these two functions return the source and destination addresses of the IBUS message:
    unsigned int source = message.source();
    unsigned int destination = message.destination();
    //unsigned int length = message.length();

     // 00 + BF - Funkschlüssel Auf/Zu, Türen Status
    if ((source == M_GM5) && (destination == M_ALL))
    { 
      switch (message.b(0))                                 // erste Nachrichten Byte
      {
        case 0x72:                                          // Funkschlüssel auf und zu // 00 04 BF 72 xx
          heiml =true;
          switch (message.b(1))                             // zweite Nachrichten Byte
          {
            case 0x22:
              ibusTrx.writeTxt("einsteigen Cordula");       // 00 04 BF 72 22 ck  
              debugln("................. Schlüssel Cordula erkannt");
              break;
            case 0x26:  // Funkschlüssel Andre auf
              ibusTrx.writeTxt("einsteigen Andre");
              break;
            case 0x12:
              ibusTrx.writeTxt("Tschuess Cordula");
              break;
            case 0x16:  // FunkSchlüssel Andre zu
              ibusTrx.writeTxt("Tschuess Andre");
              break;
            default:
              break;
          }
          break;
        
        case 0x7A:                                // status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)
          switch (message.b(1))
          {
            case 0x51:                            // 00 05 BF 7A 51 status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)
              DvrdoorFr = true;
              break;
            
            default:
              DvrdoorFr = false;
              break;
          }
        break;

        default:
          break;
      }
    }

    // Schlüssel im Schloß , Motor aus, Tankinhalt // 44 05 bf 74 xx xx
    if ((source == M_EWS) && (destination == M_ALL) && (message.b(0) == 0x74))
    {
      if (message.b(1) == 0x05)                                     // 44 05 bf 74 05 xx    Motor aus
      {              
        if (message.b(2) == 0x00)
        {
          ibusTrx.writeTxt("bis bald Cordula");                     // schlüssel stellung 1 -> 0, Corula
        }
        if (message.b(2) == 0x05) 
        {
          ibusTrx.writeTxt("bis bald Andre");                       // schlüssel stellung 1 -> 0, Andre
        }
        ibusTrx.write(Tankinhalt);                                  // Tankinhalt Abfragen (Antwort noch ermitteln!)
      }
      if (message.b(1) == 0x04)                                     // 44 05 bf 74 04 xx   Schlüssel wird ins Schloß gesteckt
      {               
        if (message.b(2) == 0x00)
        {
          ibusTrx.writeTxt("gute Fahrt Cordula");                    // 44 05 bf 74 04 00 Cordula 
          IKEclear=true;                                             // bereit um den Text im IKE zu löschen
          msTimer=millis();                                          // aktuelle Zeit in den Timer legen
        }
        if (message.b(2) == 0x05) 
        {
          ibusTrx.writeTxt("gute Fahrt Andre");                      // 44 05 bf 74 04 05 Andre
          IKEclear=true;                                             // bereit um den Text im IKE zu löschen
          msTimer=millis();                                          // aktuelle Zeit in den Timer legen
        }
      } 
    }

    // wenn tippblinken true dann:
    // Blinker: Wenn Blinkerpfeil im IKE links: D0 07 BF 5B 20 00 04 00 17 oder Blinkerpfeil im IKE rechts:  D0 07 BF 5B 40 00 04 00 77
    // D0 07 BF 5B 40 00 04 00 77
    //              |  |  |  |  |
    //              |  |  |  |  Checksum
    //              |  |  |Blinkstatus ist immer 04 , databytes[3]
    //              |  |nil
    //              |Blinkbyte kann sein: links-> 20, 21, 23, 27, 2B, 33, 3B, rechts-> 40, 41, 43, 47, 4B, 53, 5B
    if (Tippblinken == 1) 
    {           
      if ((source == M_LCM) && (destination == M_ALL) && (message.b(0) == 0x5B))      // Abfrage des Blinkerpfeils im IKE mit verschiedenen Lichtbedingungen
      {   
        if (message.b(3) == 0x04) 
        {  
          switch (message.b(1)) 
          {
            case 0x20:
            case 0x21:
            case 0x23:
            case 0x27:
            case 0x2B:
            case 0x33:
            case 0x3B:
              BlinkcountLi++;
              debugln(BlinkcountLi);
              if (BlinkcountLi < 2) 
              {
                turn=1;                      // Blinker links ein/LICHT ein
                debugln("send LCM Dimmer Request Turn=1");
                ibusTrx.write(LCMdimmReq);                                               // Anfrage Dimm-Wert des IKE Cluster vom LCM: 3F 03 D0 0B (E7)                 
              }
              break;

            case 0x40:
            case 0x41:
            case 0x43:
            case 0x47:
            case 0x4B:
            case 0x53:
            case 0x5B:
              BlinkcountRe++;
              debugln(BlinkcountRe);
              if (BlinkcountRe < 2) 
              {
                turn=2; // Blinker rechts ein/LICHT ein
                debugln("send LCM Dimmer Request Turn=2");
                ibusTrx.write(LCMdimmReq);                                               // Anfrage Dimm-Wert des IKE Cluster vom LCM: 3F 03 D0 0B (E7)  
              } 
              break;

            default:
              // Nichts tun für unbekannte Werte
            break;
          }
        } 
        else if (message.b(3) == 0x00) 
        {     // D0 07 BF 5B 40 00 00 00 73 , byte 3 = 00 =! Indicator_sync       // D0 07 BF 5B 00 = Byte 1=  0x00 = All_Off
          BlinkcountLi = 0;
          BlinkcountRe = 0;
          debugln("Reset Counts");
        }

        // wenn mehr als 3 mal geblinkt wurde, Blinker Aus
        if ((BlinkcountLi > 2) || (BlinkcountRe > 2)) 
        {
          BlinkcountLi=0;       // 
          BlinkcountRe=0;
          if (LCMdimmOK)
          {
            memcpy(LCMBlinker, BlinkerAus, sizeof(BlinkerAus));                                                 // BlinkerAus in LCMBlinker speichern
            memcpy(LCMBlinker + sizeof(BlinkerAus), LCMdimm, sizeof(LCMdimm));                                 // hinzufügen von LCMdimm
            memcpy(LCMBlinker + sizeof(BlinkerAus) + sizeof(LCMdimm), LCMBlinkerAdd, sizeof(LCMBlinkerAdd));   // hinzufügen von FF 00
            LCMBlinker[1] = sizeof(LCMBlinker)-1;                                                             // Länge des von LCMBlinker ermitteln und einsetzen an Position 2
            debugln("Blinker AUS");
            ibusTrx.write (LCMBlinker);    
            LCMdimmOK = 0;    
          }
        }        
      }

      // Abfrag des LCM DimmWertes im KombiInstrument
      if (turn == 1 || turn == 2) 
      {
        if ((source == M_LCM) && (destination == M_DIA) && (message.b(0) == 0xA0)){     // Antwort auf den Request des LCM-Status (D0 xx 3F A0 .....)
          debugln("LCM Antwort erkannt");
          for (uint8_t i = 1; i < 17; ++i) 
          {                                               
              LCMdimm[i-1] =message.b(i);         // lese 16 bytes ein = message.b(1-16)  (D0 xx 3F A0 |-> C1 C0 00 20 00 00 00 00 00 A0 00 00 88 14 84 E4 .....)
          }
          LCMdimmOK = 1;
          
          if (turn == 1)    // Blinker links
          {           
            // sende blinker links und die 16 byte der LCMdimm Antwort + FF 00
            memcpy(LCMBlinker, BlinkerLi, sizeof(BlinkerLi));                                                 // BlinkerLi in LCMBlinker speichern
            memcpy(LCMBlinker + sizeof(BlinkerLi), LCMdimm, sizeof(LCMdimm));                                 // hinzufügen von LCMdimm
            memcpy(LCMBlinker + sizeof(BlinkerLi) + sizeof(LCMdimm), LCMBlinkerAdd, sizeof(LCMBlinkerAdd));   // hinzufügen von FF 00
            LCMBlinker[1] = sizeof(LCMBlinker)-1;                                                             // Länge des von LCMBlinker ermitteln und einsetzen an Position 2
            debugln("Links Blinker ein");
            ibusTrx.write (LCMBlinker);
            turn = 0;   // Blinker abfrage erledigt
          }

          if (turn == 2)    // Blinker rechts
          {           
            // sende blinker rechts und die 16 byte der LCMdimm Antwort + FF 00
            memcpy(LCMBlinker, BlinkerRe, sizeof(BlinkerRe));                                                 // BlinkerRe in LCMBlinker speichern
            memcpy(LCMBlinker + sizeof(BlinkerRe), LCMdimm, sizeof(LCMdimm));                                 // hinzufügen von LCMdimm
            memcpy(LCMBlinker + sizeof(BlinkerRe) + sizeof(LCMdimm), LCMBlinkerAdd, sizeof(LCMBlinkerAdd));   // hinzufügen von FF 00
            LCMBlinker[1] = sizeof(LCMBlinker)-1;                                                             // Länge des von LCMBlinker ermitteln und einsetzen an Position 2
            debugln("Rechts Blinker ein");
            ibusTrx.write (LCMBlinker);
            turn = 0;   // Blinker abfrage erledigt
          }
        }       
      }
    }  

    // Geschwindigkeit, RPM, OutTemp, CoolantTemp
    if ((source == M_IKEC) && (destination == M_ALL))
      {
        switch (message.b(0)) 
        {
          case 0x18:                                // 80 05 BF 18 3F 18 (05),IKE --> GLO: Speed/RPM, Speed 126 km/h  [78 mph], 2,400 RPM
              speed = message.b(1) * 2;
              rpm = message.b(2) * 100;
              debug("Speed: ");
              debugln(speed);
              debug("RPM: ");
              debugln(rpm);
            break;

          case 0x19:                                // 80 05 BF 19 2F 5C (50),IKE --> GLO Temperature Outside 47C Coolant 92C
              outTemp = message.b(1);
              //aussentemp = outTemp;
              debug("Aussen Temperatur: ");
              debugln(outTemp);

              coolant = message.b(2);
              kwassertemp = coolant;
              debug("Kuehlmittel Temperatur: ");
              debugln(coolant);
              Coolant (coolant);                    // Gehe in die Funktion um Die Kühlmitteltemperatur im Bordmonitor anzuzeigen
            break;

          case 0x11:                                // Zündungs Nachricht
            	switch (message.b(1))
              {
                case 0x00:
                    switch (message.b(2))
                    {
                      case 0x2A:                      // Zündung Aus  80 05 BF 11 00 2A
                          Ignition = false;
                          debugln("Zündung AUS");
                        break;
                    }
                  break;
                case 0x01:
                    switch (message.b(2))
                    {
                      case 0x2B:                      // Zündung Pos1  80 05 BF 11 01 2B
                          digitalWrite(PCM_EN, HIGH);   // Power für PCM-Section einschlaten
                          digitalWrite(sys_ctl, LOW);   // Bluetooth einschalten
                          sysctl_timer =millis();
                          sysctl_on=true;  
                          debugln("Zündung Pos1");
                        break;
                    }
                  break;
                case 0x03:
                    switch (message.b(2))
                    {
                      case 0x29:                      // Zündung Pos2  80 05 BF 11 03 29
                          Ignition = true;
                          debugln("Zündung AN");
                        break;
                    }
                  break;
              } 
            break;
        }
      }
 
    // messages sent by the steering wheel controls to the radio
    if (source == M_MFL && destination == M_RAD) {
        switch (message.b(0))
      {
        case 0x32: // Lenkrad Lautstärke
          switch (message.b(1))
          {
            case 0x11:            // 50 04 68 32 10
              /* Volume UP */
              debugln("Volume Up");
            break;
          
            case 0x10:            // 50 04 68 32 11
              // Volume Down
              debugln("Volume Down");
            break;
          }  
        break;
      
        case 0x3B: // Lenkrad <>
          switch (message.b(1))
          {
            case 0x01:                  // 50 04 68 3B 01
              BTcom.println("AT+FORWARD");
              debugln("“>” nächster Titel");
            break;
          
            case 0x08:                  // 50 04 68 3B 08
              BTcom.println("AT+BACKWARD");
              debugln("“<” vorheriger Titel");
            break;
          }  
        break;

        default:
          break;
      }
    }

    // steering wheel controls to the Telefone
    if (source == M_MFL && destination == M_TEL) {
        switch (message.b(0))
      {
        case 0x3B: // Lenkrad zum Telefone
          switch (message.b(1))
          {
            case 0x80:            // 50 04 C8 3B 80  Taste "wählen" am Lenkrad"
              BTcom.println("AT+PLAYPAUSE");
              
            break;
          
            case 0x10:            // 50 04 68 32 11
              // Volume Down
              debugln("Volume Down");
            break;
          }  
        break;
        default:
          break;
      }
    }
    
    // Bordmonitor-Tasten zum Radio
    if (source == M_BMB && destination == M_RAD) {
        switch (message.b(0))
      {
        case 0x48: // linker dreh-Encoder Push
          switch (message.b(1))
          {
            case 0x06:            // F0 04 68 48 06
              /* Push-in Radio on/off */
              debugln("Push-in");
              if (BT_state)
              {
                digitalWrite(PCM_EN, LOW); // Power für PCM-Section einschlaten
                debugln("Bluetooth AUS-schalten");
                BT_state =false;
              } else{
                digitalWrite(PCM_EN, HIGH);
                BT_state=true;
                debugln("Bluetooth EIN-schalten");
              }     
              digitalWrite(sys_ctl, LOW);
              //sysctl_timer =millis();
              sysctl_on=true; 
              BluetoothToggleTimer.start();
            break;
          
            case 0x11:            // F0 04 68 48 11 C5
              // BMB-Taste 1 push
              debugln("BMB-Taste 1 push");
              BTcom.println("AT+PLAYPAUSE");
            break;
            case 0x91:            // F0 04 68 48 91 C5
              // BMB-Taste 1 release
              debugln("BMB-Taste 1 release");
            break;
            case 0x01:            // F0 04 68 48 01 C5
              // BMB-Taste 2 push
              debugln("BMB-Taste 2 push");
            break;
            case 0x81:            // F0 04 68 48 81 C5
              // BMB-Taste 2 release
              debugln("BMB-Taste 2 release");
            break;
            case 0x12:            // F0 04 68 48 12 C5
              // BMB-Taste 3 push
              debugln("BMB-Taste 3 push");
            break;
            case 0x92:            // F0 04 68 48 92 C5
              // BMB-Taste 3 release
              debugln("BMB-Taste 3 release");
            break;
            case 0x02:            // F0 04 68 48 02 C5
              // BMB-Taste 4 push
              debugln("BMB-Taste 4 push");
            break;
            case 0x82:            // F0 04 68 48 82 C5
              // BMB-Taste 4 release
              debugln("BMB-Taste 4 release");
            break;
            case 0x13:            // F0 04 68 48 13 C5
              // BMB-Taste 5 push
              debugln("BMB-Taste 5 push");
            break;
            case 0x93:            // F0 04 68 48 93 C5
              // BMB-Taste 5 release
              debugln("BMB-Taste 5 release");
            break;
            case 0x03:            // F0 04 68 48 03 C5
              // BMB-Taste 6 push
              debugln("BMB-Taste 6 push");
            break;
            case 0x83:            // F0 04 68 48 83 C5
              // BMB-Taste 6 release
              debugln("BMB-Taste 6 release");
            break;
          }  
        break;
      
        case 0x32: // // linker dreh-Encoder
          switch (message.b(1))
          {
            case 0x11:                  // 50 04 68 3B 01
              BTcom.println("AT+FORWARD");
              debugln("“>” nächster Titel");
            break;
          
            case 0x08:                  // 50 04 68 3B 08
              BTcom.println("AT+BACKWARD");
              debugln("“<” vorheriger Titel");
            break;
          }  
        break;

        default:
          break;
      }
    }

    // flags und Timer für TH-Sleep zurücksetzen:
    if (sysSleep)
    {
      //debugln("TH_EN high");
      digitalWrite(TH_EN, HIGH);      // TH3122 enable pin high
      sysSleep = false;               // SystemSleep =false -> System ist aktiv
      debugln("SystemTimer reset und System aktiv");
    }
    msSleep =millis();              // Sleep Timer reset

  // nach Inaktivität TH und Teensy in Sleep mode
  }else if ((millis()-msSleep) >= (SleepTime) && (!sysSleep))   // 600.000 ms = 10 Minuten
  {
    debugln("TH_EN low und System schlaeft");
    //digitalWrite(TH_EN, LOW);         // TH3122 enable pin low, TH wird disabled und TH-LDO abgeschaltet
    sysSleep = true;                  // SystemSleep =true -> TH3122 System schläft, dadurch auch keine Neopixels mehr
    delay(50);
  //  Snooze.deepSleep( config_sleep ); // deepSleep ~20mA, sleep ~30mA, in hibernate IBUStrx does not woke up
  }    
  // ############################ iBus message read ENDE #################################
}

// Lichtsensor BH1750 und Heimleuchten
void Daemmerung() {
  if ((millis()-msTimer) >= 800)     // 500ms Zeit abgelaufen?
    {      
      msTimer = millis();
      // Lese den Wert vom LDR
      //int ldrValue = analogRead(LDR_PIN);

      float ldrValue = lightMeter.readLightLevel();
      
      // Berechne den Durchschnitt
      sum -= ldrValues[ldrindex];  // Subtrahiere den alten Wert
      ldrValues[ldrindex] = ldrValue;  // Speichere den neuen Wert
      sum += ldrValue;  // Addiere den neuen Wert
      ldrindex = (ldrindex + 1) % NUM_READINGS;  // Aktualisiere den ldrindex

      average = sum / NUM_READINGS;  // Berechne den Durchschnitt

      // Entscheide anhand der Hysterese, ob das dunkel flag gesetzt wird
      if (average > UPPER_THRESHOLD && dunkel) {
        dunkel = false;
      } else if (average < LOWER_THRESHOLD && !dunkel) {
        dunkel = true;
      }

      // Ausgabe des Durchschnitts und des dunkel-flags
      /*
      debug("Average: ");
      debug(average);
      debug(" - dunkel: ");
      debugln(dunkel);
      dunkel = true;  // nur zum testen wenn kein LDR angeschlossen ist
      */
    }
  if ((dunkel && !Ignition && DvrdoorFr) || (dunkel && heiml))
  {
    debugln("Heimleuchten gesendet!");
    // Ausgabe des Durchschnitts und des dunkel-flags
    debug("Average Lux: ");
    debug(average);
    debug(" - dunkel= ");
    debugln(dunkel);
    //dunkel = true;  // nur zum testen wenn kein LDR angeschlossen ist
    ibusTrx.write (Heimleuchten);
    //Ignition = true;
    DvrdoorFr = false;
    heiml =false;
  }  
}

// Kühlmitteltemperatur im Bordmonitor anzuzeigen
void Coolant (uint8_t coolant){
  uint8_t coolantTemp[3];       // Kühlmitteltemperatur zerlegt und in Hex abgelegt z.B. 128 => 31 32 38
  uint8_t zerlegen = coolant;   // Hilfs Variable zum zerlegen des Decimalwertes
  // zerlegen von coolant
  for (int8_t i = 2; i >= 0; --i) 
  {
    coolantTemp[i] = (zerlegen % 10) + 0x30;  // Die letzte Ziffer der Zahl +30 (in Hex 0x1E) und im Array speichern
    zerlegen /= 10;                           // Eine Ziffer entfernen
  }
  // wenn coolant nur zwei Ziffern hat füge ein Leerzeichen in HEX voran
  if (coolant < 100) 
  {
    coolantTemp[0] = 0x20;    
  }
  /*  for (int i = 0; i < 3; i++) 
  {
    debug(" ");
    debug(coolantTemp[i]);
  }*/
  // Zeichenkette zusammen bauen: 80 0F E7 24 0E 00 20  + coolantTemp + 20 B0 43 20 20 + cksum
  memcpy(BCcool, BCcoolbeginn, sizeof(BCcoolbeginn));                                         // BCcoolbeginn in BCcool speichern
  memcpy(BCcool + sizeof(BCcoolbeginn), coolantTemp, sizeof(coolantTemp));                    // hinzufügen von coolantTemp
  memcpy(BCcool + sizeof(BCcoolbeginn) + sizeof(coolantTemp), BCcoolend, sizeof(BCcoolend));  // hinzufügen von {0x20, 0xB0, 0x43, 0x20, 0x20}
  BCcool[1] = sizeof(BCcool)-1;                                                               // länge der gesammten Zeichenkette berechnen und eintragen
  // sende an den Bordmonitor im BC-Bereich anstelle von Timer 1 oder 2
  ibusTrx.write (BCcool);
}

// Wenn Interrup dann sende die iBus Codes
void ClearToSend() {
  //debugln("................................................................... Interrupt Clear to send");
  ibusTrx.send(); // Nachricht senden, wenn Interrupt ausgelöst wird
}

// ########################### iBus Codes ########################################################
// die checksumme muss nicht mit angegeben werden
uint8_t cleanIKE [6] PROGMEM = {
  0x30, 0x05, 0x80, 0x1A, 0x30, 0x00}; // IKE Anzeige wird gelöscht, wichtig sonst bleibt sie immer an
uint8_t BlinkerRe [13] PROGMEM = {
  0x3F, 0x0C, 0xD0, 0x0C, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //3F0FD00C000040000000000000
uint8_t BlinkerLi [13] PROGMEM = {
  0x3F, 0x0c, 0xD0, 0x0C, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 3F 0B D0 0C 00 00 80 00 00 00 00 00
uint8_t BlinkerAus [13] PROGMEM = {
  0x3F, 0x0C, 0xD0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //3F0FD00C000000000000000000
uint8_t LCMdimmReq [] PROGMEM ={0x3F, 0x03, 0xD0, 0x0B}; // Diagnoseanfrage ans LCM um Helligkeitswert für IKE zu finden
//uint8_t LCMdimmReplay [32] PROGMEM = {0xA0, 0xC1, 0xC0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x88, 0x14, 0x84, 0xE4, 0xFF, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFE};
//size_t LCMdimmReplaylen = 31; // die Größe von LCMdimmReplay, komischerweise ist das Array 32 byte groß aber es funktioniert nur mit 31
uint8_t LCMBlinkerAdd [2]  PROGMEM = {0xFF, 0x00}; // Anhängsel nach dem LCMBlinker zusammenbau
uint8_t BCcoolbeginn [7]  PROGMEM = {0x80, 0x0E, 0xE7, 0x24, 0x0E, 0x00, 0x20}; // Kühlmitteltemperatur im Bordmonitor anzeigen Anfangs-Kette
uint8_t BCcoolend [5] PROGMEM = {0x20, 0xB0, 0x43, 0x20, 0x20}; // Kühlmitteltemperatur im Bordmonitor anzeigen Schluß-Kette (_°C__)
// Heimleuchten Nebel und Bremslichter: 3F0FD00C000000001844000000E5FF00AA
//Fahrertür auf
uint8_t door_open_driver[] PROGMEM = {0x00, 0x05, 0xBF, 0x7A, 0x51};  // status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)
//00 05 BF 7A 51
uint8_t ZV_lock[] PROGMEM = {0x3F, 0x05, 0x00, 0x0C, 0x00, 0x0B}; // Zentralverriegelung öffnen / schließen 3F05000C000B(3D)

uint8_t Heimleuchten[] PROGMEM = 
  {0x3F, 0x0F, 0xD0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x18, 0x44, 0x00, 0x00, 0x00, 0xE5, 0xFF, 0x00}; // Lichter für Heimleuchten: Bremslicht und Nebelleuchten: 3F0FD00C000000001844000000E5FF00

uint8_t Tankinhalt[5] PROGMEM = {0x3F, 0x04, 0x80, 0x0B, 0x0A};    // Tankinhalt abfragen:	3F 04 80 0B 0A (BA)

uint8_t SthzEIN[5] PROGMEM = {0x3B, 0x04, 0x80, 0x41, 0x12};   // Standheizung EIN: 3B 04 80 41 12 (EC) 
uint8_t SthzAUS[5] PROGMEM = {0x3B, 0x04, 0x80, 0x41, 0x11};   // Standheizung AUS: 3B 04 80 41 11 (EC) 

/*
// Example: define the message that we want to transmit
// the message must be defined as an array of uint8_t's (unsigned 8-bit integers)
uint8_t toggleDomeLight[6] = {
  M_DIA, // sender ID (diagnostic interface)
  0x05,  // length of the message payload (including destination ID and checksum)
  M_GM5, // destination ID (body control module)
  GM5_SET_IO, // the type of message (IO manipulation)
  GM5_BTN_DOME_LIGHT, // the first parameter (the IO line that we want to manipulate)
  0x01 // second parameter
  // don't worry about the checksum, the library automatically calculates it for you
};*/
/*
// IBUS message definitions. Add/Remove as needed.
uint8_t KEY_IN [7] PROGMEM = { 
  0x44 , 0x05 , 0xBF , 0x74 , 0x04 , 0x00 , 0x8E }; // Ignition key in
uint8_t KEY_OUT [7] PROGMEM = { 
  0x44 , 0x05 , 0xBF , 0x74 , 0x00 , 0xFF , 0x75 }; // Ignition key out
uint8_t MFL_VOL_UP [6] PROGMEM = { 
  0x50 , 0x04 , 0x68 , 0x32, 0x11 , 0x1F }; // Steering wheel Volume Up
uint8_t MFL_VOL_DOWN [6] PROGMEM = { 
  0x50 , 0x04 , 0x68 , 0x32, 0x10 , 0x1E }; // Steering wheel Volume Down
uint8_t MFL_TEL_VOL_UP [6] PROGMEM = { 
  0x50 , 0x04 , 0xC8 , 0x32, 0x11 , 0xBF }; // Steering wheel Volume Up - Telephone
uint8_t MFL_TEL_VOL_DOWN [6] PROGMEM = { 
  0x50 , 0x04 , 0xC8 , 0x32, 0x10 , 0xBE }; // Steering wheel Volume Down - Telephone
uint8_t MFL_SES_PRESS [6] PROGMEM = { 
  0x50 , 0x04 , 0xB0 , 0x3B, 0x80 , 0x5F }; // Steering wheel press and hold phone button
uint8_t MFL_SEND_END_PRESS [6] PROGMEM = { 
  0x50 , 0x04 , 0xC8 , 0x3B, 0x80 , 0x27 }; // Steering wheel send/end press
uint8_t MFL_RT_PRESS [6] PROGMEM = { 
  0x50 , 0x04 , 0x68 , 0x3B , 0x02, 0x05 }; // MFL R/T press
uint8_t CD_STOP [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x01 , 0x00 , 0x4C }; // CD Stop command
uint8_t CD_PLAY [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x03 , 0x00 , 0x4E }; // CD Play command
uint8_t CD_PAUSE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x02 , 0x00 , 0x4F }; // CD Pause command
uint8_t CD_STOP_STATUS [12] PROGMEM = { 
  0x18 , 0x0A , 0x68 , 0x39 , 0x00 , 0x02 , 0x00 ,  0x3F , 0x00 , 0x07 , 0x01 , 0x78 }; // CD stop request
uint8_t CD_PLAY_STATUS [12] PROGMEM = { 
  0x18 , 0x0A , 0x68 , 0x39 , 0x02 , 0x09 , 0x00 ,  0x3F , 0x00 , 0x07 , 0x01 , 0x71 }; // CD play request
uint8_t INCOMING_CALL [6] PROGMEM = { 
  0xC8 , 0x04 , 0xE7 , 0x2C , 0x05 , 0x02 }; // Incoming phone call
uint8_t PHONE_ON [6] PROGMEM = { 
  0xC8 , 0x04 , 0xE7 , 0x2C , 0x10 , 0x17 }; // Phone On
uint8_t HANDSFREE_PHONE_ON [6] PROGMEM = { 
  0xC8 , 0x04 , 0xE7 , 0x2C , 0x11 , 0x16 }; // Hands Free Phone On
uint8_t ACTIVE_CALL [6] PROGMEM = { 
  0xC8 , 0x04 , 0xE7 , 0x2C , 0x33 , 0x34 }; // Active phone call
uint8_t IGNITION_OFF [6] PROGMEM = { 
  0x80 , 0x04 , 0xBF , 0x11 , 0x00 , 0x2A }; // Ignition Off
uint8_t IGNITION_POS1 [6] PROGMEM = { 
  0x80 , 0x04 , 0xBF , 0x11 , 0x01 , 0x2B }; // Ignition Acc position - POS1
uint8_t IGNITION_POS2 [6] PROGMEM = { 
  0x80 , 0x04 , 0xBF , 0x11 , 0x03 , 0x29 }; // Ignition On position - POS2
uint8_t REMOTE_UNLOCK [6] PROGMEM = { 
  0x00 , 0x04 , 0xBF , 0x72 , 0x22 , 0xEB }; // Remote control unlock
uint8_t DSP_STATUS_REQUEST [5] PROGMEM = { 
  0x68 , 0x03 , 0x6A , 0x01 , 0x00 }; // DSP status request
uint8_t DSP_STATUS_REPLY [6] PROGMEM = { 
  0x6A , 0x04 , 0xFF , 0x02 , 0x00 , 0x93 }; // DSP status reply
uint8_t DSP_STATUS_REPLY_RST [6] PROGMEM = { 
  0x6A , 0x04 , 0xFF , 0x02 , 0x01 , 0x92 }; // DSP status ready after reset to LOC
uint8_t DSP_VOL_UP_1 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x11 , 0x25 }; // Rotary Volume Up 1 step
uint8_t DSP_VOL_UP_2 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x21 , 0x15 }; // Rotary Volume Up 2 step
uint8_t DSP_VOL_UP_3 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x31 , 0x05 }; // Rotary Volume Up 3 step
uint8_t DSP_VOL_DOWN_1 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x10 , 0x24 }; // Rotary Volume Down 1 step
uint8_t DSP_VOL_DOWN_2 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x20 , 0x14 }; // Rotary Volume Down 2 step
uint8_t DSP_VOL_DOWN_3 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x30 , 0x04 }; // Rotary Volume Down 3 step
uint8_t DSP_FUNC_0 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0x30 , 0x00 }; // DSP_Function 0
uint8_t DSP_FUNC_1 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0xE1 , 0xD1 }; // DSP_Function 1
uint8_t DSP_SRCE_OFF [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0xAF , 0x9F }; // DSP Source = OFF
uint8_t DSP_SRCE_CD [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0xA0 , 0x90 }; // DSP Source = CD
uint8_t DSP_SRCE_TUNER [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0xA1 , 0x91 }; // DSP Source = Tuner
uint8_t GO_TO_RADIO [6] PROGMEM = { 
  0x68 , 0x04 , 0xFF , 0x3B, 0x00 , 0xA8 }; // Go  to radio - I think
uint8_t REQUEST_TIME [7] PROGMEM = { 
  0x68 , 0x05 , 0x80 , 0x41, 0x01 , 0x01 , 0xAC}; // Request current time from IKE
uint8_t CLOWN_FLASH [7] PROGMEM = { 
  0x3F , 0x05 , 0x00 , 0x0C , 0x4E , 0x01 , 0x79 }; // Turn on clown nose for 3 seconds
uint8_t BACK_ONE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x08 , 0x00 , 0x45 }; // Back
uint8_t BACK_TWO [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x08 , 0x01 , 0x44 }; // Back
uint8_t LEFT [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x0A , 0x01 , 0x46 }; // Left
uint8_t RIGHT [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x0A , 0x00 , 0x47 }; // Right
uint8_t SELECT [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x07 , 0x01 , 0x4B }; // Select
uint8_t BUTTON_ONE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x01 , 0x4A }; // Button 1
uint8_t BUTTON_TWO [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x02 , 0x49 }; // Button 2
uint8_t BUTTON_THREE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x03 , 0x48 }; // Button 3
uint8_t BUTTON_FOUR [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x04 , 0x4F }; // Button 4
uint8_t BUTTON_FIVE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x05 , 0x4E }; // Button 5
uint8_t BUTTON_SIX [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x06 , 0x4D }; // Button 6
uint8_t CDC_STATUS_REPLY_RST [6] PROGMEM = { 
  0x18 , 0x04 , 0xFF , 0x02 , 0x01 , 0xE0 }; // CDC status ready after reset to LOC
uint8_t CDC_STATUS_REQUEST [5] PROGMEM = { 
  0x68 , 0x03 , 0x18 , 0x01 , 0x72 }; // CDC status request
uint8_t CDC_STATUS_REPLY [6] PROGMEM = { 
  0x18 , 0x04 , 0xFF , 0x02 , 0x00 , 0xE1 }; // CDC status reply
uint8_t CD_STATUS [16] PROGMEM = { 
  0x18 , 0x0E , 0x68 , 0x39 , 0x00 , 0x82 , 0x00 , 0x3F , 0x00 , 0x07 , 0x00 , 0x00 , 0x01 , 0x01 , 0x01 , 0xFC }; // CD status
uint8_t BUTTON_PRESSED [6] PROGMEM = {
  0x68 , 0x04 , 0xFF , 0x3B ,  0x00 , 0xA8 }; // Radio/Telephone control, No_buttons_pressed 
uint8_t VOL_INCREMENT [64] PROGMEM = { 
  0,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
  128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,
  178,180,182,184,186,188,190,192}; // Volume increments
*/

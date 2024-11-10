// ibusCodes.cpp

#include "IbusCodes.h"
#include "globals.h"
#include <BH1750.h>   // GY-302 mit Licht-Sensor BH1750 für Dämmerungsschalter
#include <IbusTrx.h>
#include <Snooze.h>             // Teensy Snooze Lib - diese muss modifiziert werden siehe: https://github.com/duff2013/Snooze/issues/114
#include "neopixel.h"           // Funktiuonen für die Neopixel LEDs

BH1750 lightMeter;    // Lichtsensor Instance
IbusTrx ibusTrx;      // IbusTrx instance
SnoozeUSBSerial usb;
SnoozeDigital digital;                  // this is the pin's wakeup driver
SnoozeBlock config_sleep(digital); // install driver into SnoozeBlock

bool sideMirrorDone;
bool blinkLockedLi = false;            // Sperre für linkes Blinken
bool blinkLockedRe = false;            // Sperre für rechtes Blinken


enum TurnState
{
  TURN_OFF = 0,   // Blinker aus
  TURN_LEFT = 1,  // Blinker links
  TURN_RIGHT = 2, // Blinker rechts
  TURN_RESET = 3  // Blinker zurückgesetzt
};
TurnState turn = TURN_OFF; // turn als TurnState deklarieren

enum SideMirrorState
{
  MIRROR_NONE = 0,   // Kein Status
  MIRROR_OPEN = 1,   // Spiegel ausgeklappt
  MIRROR_CLOSE = 2,  // Spiegel eingeklappt
  MIRROR_UNKNOWN = 3 // Unbekannter Zustand
};
SideMirrorState sideMirror = MIRROR_NONE; // sideMirror als SideMirrorState deklarieren

void iBusMessage()
{
  // if there's a message waiting, check it out
  if (ibusTrx.available())
  {
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
    // unsigned int length = message.length();

    // 00 -> BF - Funkschlüssel Auf/Zu, Türen Status
    if ((source == M_GM5) && (destination == M_ALL))
    {
      switch (message.b(0)) // erste Nachrichten Byte
      {
      case 0x72: // Funkschlüssel auf und zu // 00 04 BF 72 xx
        heiml = true;
        sideMirrorDone = true;
        sideMirror = MIRROR_UNKNOWN;
        switch (message.b(1)) 
        {
        case 0x22:
          ibusTrx.writeTxt("einsteigen Cordula"); // 00 04 BF 72 22 ck
          driverID = "Cordula";
          sideMirror = MIRROR_OPEN;
          break;
        case 0x27: // Funkschlüssel Andre auf
          ibusTrx.writeTxt("einsteigen Andre");
          driverID = "Andre";
          sideMirror = MIRROR_OPEN;
          break;
        case 0x2B: // Funkschlüssel Lennox auf
          ibusTrx.writeTxt("einsteigen Lennox");
          driverID = "Lennox";
          sideMirror = MIRROR_OPEN;
          break;
        case 0x12:
          ibusTrx.writeTxt("Tschuess Cordula");
          sideMirror = MIRROR_CLOSE;
          break;
        case 0x17: // FunkSchlüssel Andre zu
          ibusTrx.writeTxt("Tschuess Andre");
          sideMirror = MIRROR_CLOSE;
          break;
        case 0x0B: // FunkSchlüssel Lennox zu
          ibusTrx.writeTxt("Tschuess Lennox");
          sideMirror = MIRROR_CLOSE;
          break;

        default:
          break;
        }
        break;

      case 0x7A: // Status z.b. Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)
        processStatusMessage(message.b(1), message.b(2));

        break;

      default:
        break;
      }
    }

    // Überprüfen - Schlüssel im Schloß , Motor aus, Tankinhalt // 44 05 bf 74 xx xx
    if ((source == M_EWS) && (destination == M_ALL) && (message.b(0) == 0x74)) // 0x74=Immobiliser status
    {
    /*
      switch (message.b(1))
      {
      case 0x05: // 44 05 bf 74 05 xx    Motor aus
        debugln("Schlüssel Stellung 1->0");
        switch (message.b(2))
        {
        case 0x00:
          ibusTrx.writeTxt("bis bald Cordula"); // Schlüsselstellung 1 -> 0, Cordula
          break;
        case 0x04:
          ibusTrx.writeTxt("bis bald Lennox"); // 44 05 bf 74 05 04 Lennox
          break;
        case 0x05:
          ibusTrx.writeTxt("bis bald Andre"); // Schlüsselstellung 1 -> 0, Andre
          break;
        default:
          break;
        }
        // TODO:  ibusTrx.write(Tankinhalt);                      // 3F 04 80 0B 0A Tankinhalt abfragen (TODO: Antwort noch ermitteln!) keine ANtworterhalten, evtl. mit INPA ermitteln
        break;

        
        case 0x04: // 44 05 bf 74 04 xx Schlüssel wird ins Schloss gesteckt
          IKEclear = true;
          debugln("Schlüssel wurde in das Zündschloss gesteckt");
          switch (message.b(2))
          {
          case 0x00:
            ibusTrx.writeTxt("gute Fahrt Cordula"); // 44 05 bf 74 04 00 Cordula
            break;
          case 0x04:
            ibusTrx.writeTxt("gute Fahrt Lennox"); // 44 05 bf 74 04 04 Lennox
            break;
          case 0x05:
            ibusTrx.writeTxt("gute Fahrt Andre"); // 44 05 bf 74 04 05 Andre

            break;
          default:
            break;
          }
        
        break;

      default:
        break;
      }
    */
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
      // Prüfung, ob eine Blinker-Nachricht empfangen wurde
      if ((source == M_LCM) && (destination == M_ALL) && (message.b(0) == 0x5B))
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
            BlinkerUnblockTimer.start(); // Blinkersperre zwischen zwei Blinksignalen aufheben
            if (!blinkLockedLi)          // Nur fortsetzen, wenn keine Sperre aktiv ist
            {
              BlinkcountLi++;
              debugln(BlinkcountLi);

              if (BlinkcountLi == 1) // Sende nur beim ersten Blinken
              {
                ibusTrx.write(LCMdimmReq);
                turn = TURN_LEFT;
                debugln("LCM Dimmer Request für linken Blinker gesendet");
              }

              if (BlinkcountLi > 2) // Sperre setzen nach 3 Blinken
              {
                turn = TURN_RESET;
                blinkLockedLi = true;
                BlinkcountLi = 0; // Zähler zurücksetzen
              }
            }
            break;

          case 0x40:
          case 0x41:
          case 0x43:
          case 0x47:
          case 0x4B:
          case 0x53:
          case 0x5B:
            BlinkerUnblockTimer.start(); // Blinkersperre zwischen zwei Blinksignalen aufheben
            if (!blinkLockedRe)          // Nur fortsetzen, wenn keine Sperre aktiv ist
            {
              BlinkcountRe++;
              debugln(BlinkcountRe);

              if (BlinkcountRe == 1) // Sende nur beim ersten Blinken
              {
                ibusTrx.write(LCMdimmReq);
                turn = TURN_RIGHT;
                debugln("LCM Dimmer Request für rechten Blinker gesendet");
              }

              if (BlinkcountRe > 2) // Sperre setzen nach 3 Blinken
              {
                turn = TURN_RESET;
                blinkLockedRe = true;
                BlinkcountRe = 0; // Zähler zurücksetzen
                debugln("Rechter Blinker blockiert");
              }
            }
            break;

          default:
            break;
          }
        }
      }

      // Abfrage des LCMdimm-Wertes im KombiInstrument, nur wenn Antwort vom LCM erhalten wird
      if ((source == M_LCM) && (destination == M_DIA) && (message.b(0) == 0xA0) && (message.b(1) == 0xC1))
      {
        debugln("LCM Antwort erkannt");
        LCMdimm = message.b(16); // Lese das 16. Byte als temporäre Variable
        LCMdimmOK = 1;
      }

      // Zusammenbau von LCMBlinker je nach Blinkrichtung
      if (LCMdimmOK && turn != TURN_OFF)
      {
        if (turn == TURN_LEFT)
        {
          memcpy(LCMBlinker, BlinkerLi, sizeof(BlinkerLi));
          debugln("Links Blinker aktiv");
          turn = TURN_OFF;
        }
        else if (turn == TURN_RIGHT)
        {
          memcpy(LCMBlinker, BlinkerRe, sizeof(BlinkerRe));
          debugln("Rechts Blinker aktiv");
          turn = TURN_OFF;
        }
        else if (turn == TURN_RESET)
        {
          memcpy(LCMBlinker, BlinkerAus, sizeof(BlinkerAus));
          debugln("Blinker AUS");
          turn = TURN_OFF;
          LCMdimmOK = 0;
        }

        // Füge das einzelne LCMdimm-Byte und das LCMBlinkerAdd-Array hinzu
        LCMBlinker[sizeof(BlinkerLi)] = LCMdimm; // Setze LCMdimm an die richtige Position
        memcpy(LCMBlinker + sizeof(BlinkerLi) + 1, LCMBlinkerAdd, sizeof(LCMBlinkerAdd));
        LCMBlinker[1] = 0x0F;      // Länge ist immer 0x0F
        ibusTrx.write(LCMBlinker); // Sende den fertigen Blinker-Code
      }
    }


    // US - Tagfahrlicht ein/aus schalten. Hierzu muss das Byte 21 in Block 08 des LCM codiert werden.
    if (usLightTrigger)
    {
      usLightTrigger = false;
      if ((source == M_LCM) && (destination == M_DIA) && (message.b(0) == 0xA0) && (message.b(1) == 0x00))
      {
          debugln("LCM Antwort US-Tagfahrlicht erkannt");
          uint8_t LcmBlock8[32];                                    // Erstelle ein Array zur Speicherung von message.b(0) bis message.b(32)
          for (uint8_t i = 1; i < 33; i++) {                        // Fülle das Array mit den Werten aus message.b(i)
              LcmBlock8[i-1] = message.b(i);
          }
          LcmBlock8[20] = usLightByte21;                            // Ersetze message.b(21) durch usLightByte21
          uint8_t finalMessage[37] = {0x3F, 0x23, 0xD0, 0x09};      // Erstelle den finalen Nachrichtenblock mit dem Präfix 3F 23 D0 09
          memcpy(finalMessage + 4, LcmBlock8, 32);                  // Kopiere das korrigierte Array in den finalen Nachrichtenblock
          ibusTrx.write(finalMessage);
      }
    }
    

    // 80 -> BF Geschwindigkeit, RPM, OutTemp, CoolantTemp
    if ((source == M_IKEC) && (destination == M_ALL))
    {
      switch (message.b(0))
      {
      case 0x18: // 80 05 BF 18 3F 18 (05),IKE --> GLO: Speed/RPM, Speed 126 km/h  [78 mph], 2,400 RPM
        speed = message.b(1) * 2;
        rpm = message.b(2) * 100;
        debug("Speed: ");
        debugln(speed);
        debug("RPM: ");
        debugln(rpm);
        break;

      case 0x19: // 80 05 BF 19 2F 5C (50),IKE --> GLO Temperature Outside 47C Coolant 92C
        outTemp = message.b(1);
        // aussentemp = outTemp;
        debug("Aussen Temperatur: ");
        debugln(outTemp);

        coolant = message.b(2);
        kwassertemp = coolant;
        debug("Kuehlmittel Temperatur: ");
        debugln(coolant);
        Coolant(coolant); // Gehe in die Funktion um Die Kühlmitteltemperatur im Bordmonitor anzuzeigen
        break;

      case 0x11: // Zündungs Status
        switch (message.b(1))
        {
        case 0x00: // Zündung Aus  80 04 BF 11 00
          Ignition = false;
          debugln("Zündung AUS");
          if (driverID == "Andre")
          {
            ibusTrx.writeTxt("bis bald Andre");
          }
          else if (driverID == "Cordula")
          {
            ibusTrx.writeTxt("bis bald Cordula");
          }
          else if (driverID == "Lennox")
          {
            ibusTrx.writeTxt("bis bald Lennox");
          }
          break;

        case 0x01: // Zündung Pos.1 80 04 BF 11 01
          Ignition = true;
          debugln("Zündung Pos.1");
          break;

        case 0x03: // Zündung Pos.2  80 04 BF 11 03
          Ignition = true;
          IKEclear = true;
          debugln("Zündung Pos.2");
          if (driverID == "Andre")
          {
            ibusTrx.writeTxt("gute Fahrt Andre");
          }
          else if (driverID == "Cordula")
          {
            ibusTrx.writeTxt("gute Fahrt Cordula");
          }
          else if (driverID == "Lennox")
          {
            ibusTrx.writeTxt("gute Fahrt Lennox");
          }
          break;
        }
        break;
      }
    }

    // 50 -> 68 messages sent by the steering wheel controls to the radio
    if (source == M_MFL && destination == M_RAD)
    {
      switch (message.b(0))
      {
      case 0x32: // Lenkrad Lautstärke
        switch (message.b(1))
        {
        case 0x11: // 50 04 68 32 10
          /* Volume UP */
          debugln("Volume Up");
          break;

        case 0x10: // 50 04 68 32 11
          // Volume Down
          debugln("Volume Down");
          break;
        }
        break;

      case 0x3B: // Lenkrad <>
        switch (message.b(1))
        {
        case 0x01: // 50 04 68 3B 01
          BTcom.println("AT+FORWARD");
          debugln("“>” nächster Titel");
          break;

        case 0x08: // 50 04 68 3B 08
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
    if (source == M_MFL && destination == M_TEL)
    {
      switch (message.b(0))
      {
      case 0x3B: // Lenkrad zum Telefone
        switch (message.b(1))
        {
        case 0x80: // 50 04 C8 3B 80  Taste "wählen" am Lenkrad"
          BTcom.println("AT+PLAYPAUSE");

          break;

        case 0x10: // 50 04 68 32 11
          // Volume Down
          debugln("Volume Down");
          break;
        }
        break;
      default:
        break;
      }
    }

    // Bordmonitor-Tasten F0 zum Radio 68
    if (source == M_BMB && destination == M_RAD)
    {
      switch (message.b(0))
      {
      case 0x48: // linker dreh-Encoder Push
        switch (message.b(1))
        {
        case 0x06: // F0 04 68 48 06
          /* Push-in Radio on/off */
          debugln("Push-in");
          if (BT_state)
          {
            digitalWrite(PCM_EN, LOW); // Power für PCM-Section einschlaten
            debugln("Bluetooth AUS-schalten");
            BT_state = false;
          }
          else
          {
            digitalWrite(PCM_EN, HIGH);
            BT_state = true;
            debugln("Bluetooth EIN-schalten");
          }
          digitalWrite(sys_ctl, LOW);
          // sysctl_timer =millis();
          sysctl_on = true;
          BluetoothToggleTimer.start();
          break;

        case 0x11: // F0 04 68 48 11 C5
          // BMB-Taste 1 push
          debugln("BMB-Taste 1 push");
          BTcom.println("AT+PLAYPAUSE");
          break;
        case 0x91: // F0 04 68 48 91 C5
          // BMB-Taste 1 release
          debugln("BMB-Taste 1 release");
          break;
        case 0x01: // F0 04 68 48 01 C5
          // BMB-Taste 2 push
          debugln("BMB-Taste 2 push");
          break;
        case 0x81: // F0 04 68 48 81 C5
          // BMB-Taste 2 release
          debugln("BMB-Taste 2 release");
          break;
        case 0x12: // F0 04 68 48 12 C5
          // BMB-Taste 3 push
          debugln("BMB-Taste 3 push");
          break;
        case 0x92: // F0 04 68 48 92 C5
          // BMB-Taste 3 release
          debugln("BMB-Taste 3 release");
          break;
        case 0x02: // F0 04 68 48 02 C5
          // BMB-Taste 4 push
          debugln("BMB-Taste 4 push");
          break;
        case 0x82: // F0 04 68 48 82 C5
          // BMB-Taste 4 release
          debugln("BMB-Taste 4 release");
          break;
        case 0x13: // F0 04 68 48 13 C5
          // BMB-Taste 5 push
          debugln("BMB-Taste 5 push");
          break;
        case 0x93: // F0 04 68 48 93 C5
          // BMB-Taste 5 release
          debugln("BMB-Taste 5 release");
          break;
        case 0x03: // F0 04 68 48 03 C5
          // BMB-Taste 6 push
          BTcom.println("AT+PLAYPAUSE");
          debugln("BMB-Taste 6 push");
          break;
        case 0x83: // F0 04 68 48 83 C5
          // BMB-Taste 6 release
          debugln("BMB-Taste 6 release");
          break;
        }
        break;

      case 0x32: // // linker dreh-Encoder
        switch (message.b(1))
        {
        case 0x11: // F0 04 68 32 11
          BTcom.println("AT+FORWARD");
          debugln("“>” nächster Titel");
          break;

        case 0x10: // F0 04 68 32 10
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
      // debugln("TH_EN high");
      digitalWrite(TH_EN, HIGH); // TH3122 enable pin high
      digitalWrite(Sara_DTR, LOW);         // Sara Powersaving Mode OFF
      sysSleep = false;          // SystemSleep =false -> System ist aktiv
      FastLED.setBrightness(BRIGHTNESS);
      debugln("SystemTimer reset und System aktiv");
    }
    msSleep = millis(); // Sleep Timer reset

    // nach Inaktivität TH und Teensy in Sleep mode
  }
  else if ((millis() - msSleep) >= (SleepTime) && (!sysSleep)) // 600.000 ms = 10 Minuten
  {
    FastLED.setBrightness(0);
    debugln("TH_EN low und System schlaeft");
    sysSleep = true; // SystemSleep =true -> TH3122 System schläft, dadurch auch keine Neopixels mehr
    //delay(100);
    //digitalWrite(TH_EN, LOW);         // TH3122 enable pin low, TH wird disabled und TH-LDO abgeschaltet
    //Snooze.sleep( config_sleep );     // deepSleep ~20mA, sleep ~30mA, in hibernate IBUStrx does not woke up
  }
  // ############################ iBus message read ENDE #################################
}

// Variablen für jeden Status
bool doorFrontLeft = false;
bool doorFrontRight = false;
bool doorRearLeft = false;
bool doorRearRight = false;
bool ZVunlocked = false;
// bool ZVlocked = false;  Global definiert
bool interiorLight = false;

bool windowFrontLeft = false;
bool windowFrontRight = false;
bool windowRearLeft = false;
bool windowRearRight = false;
bool sunroof = false;
bool trunk = false;
bool hood = false;

// Statusabfrage z.B. 00 05 BF 7A 11 3A
void processStatusMessage(uint8_t byte1, uint8_t byte2)
{
  // Bitmasken dekodieren und in Variablen speichern
  // Byte 1
  doorFrontLeft = (byte1 & 0x01) ? 1 : 0;  // Bit 0
  doorFrontRight = (byte1 & 0x02) ? 1 : 0; // Bit 1
  doorRearLeft = (byte1 & 0x04) ? 1 : 0;   // Bit 2
  doorRearRight = (byte1 & 0x08) ? 1 : 0;  // Bit 3
  ZVunlocked = (byte1 & 0x10) ? 1 : 0;     // Bit 4
  ZVlocked = (byte1 & 0x20) ? 1 : 0;       // Bit 5
  interiorLight = (byte1 & 0x40) ? 1 : 0;  // Bit 6

  // Byte 2
  windowFrontLeft = (byte2 & 0x01) ? 1 : 0;  // Bit 0
  windowFrontRight = (byte2 & 0x02) ? 1 : 0; // Bit 1
  windowRearLeft = (byte2 & 0x04) ? 1 : 0;   // Bit 2
  windowRearRight = (byte2 & 0x08) ? 1 : 0;  // Bit 3
  sunroof = (byte2 & 0x10) ? 1 : 0;          // Bit 4
  trunk = (byte2 & 0x20) ? 1 : 0;            // Bit 5
  hood = (byte2 & 0x40) ? 1 : 0;             // Bit 6

  /*
  Beispiel: Filtern von Bit 0 mit doorFrontLeft = (byte1 & 0x01) ? 1 : 0;
  1. Bitmaske erstellen
  Die Bitmaske 0x01 (hexadezimal für 00000001 im Binärformat) hat nur das Bit 0 gesetzt und alle anderen Bits auf 0.
  Die Idee dahinter ist, dass beim AND-Vergleich nur das entsprechende Bit in byte1 (hier Bit 0) "durchgelassen" wird, alle anderen Bits in byte1 werden ausgefiltert, weil sie mit 0 multipliziert werden.
  2. Bitweises UND (AND) zur Extraktion
  Das &-Operator vergleicht jedes Bit von byte1 mit den Bits der Maske 0x01.
  Wenn Bit 0 in byte1 gesetzt ist (also 1), ergibt byte1 & 0x01 das Ergebnis 0x01, weil 1 & 1 = 1.
  Wenn Bit 0 in byte1 nicht gesetzt ist (also 0), ergibt byte1 & 0x01 das Ergebnis 0x00, weil 0 & 1 = 0.
  3. Ternäre Operation zur Zuweisung
  Die ternäre Operation (byte1 & 0x01) ? 1 : 0 wird verwendet, um das Ergebnis in doorFrontLeft entweder 1 oder 0 zu setzen:
  Wenn das Ergebnis von byte1 & 0x01 ungleich 0 ist (also Bit 0 ist gesetzt), wird doorFrontLeft auf 1 gesetzt.
  Wenn das Ergebnis 0 ist (also Bit 0 ist nicht gesetzt), wird doorFrontLeft auf 0 gesetzt.
  */

  // Variablenwerte ausgeben
  debug("doorFrontLeft: ");
  debugln(doorFrontLeft);

  debug("doorFrontRight: ");
  debugln(doorFrontRight);

  debug("doorRearLeft: ");
  debugln(doorRearLeft);

  debug("doorRearRight: ");
  debugln(doorRearRight);

  debug("ZVunlocked: ");
  debugln(ZVunlocked);

  debug("ZVlocked: ");
  debugln(ZVlocked);

  debug("interiorLight: ");
  debugln(interiorLight);

  debug("windowFrontLeft: ");
  debugln(windowFrontLeft);

  debug("windowFrontRight: ");
  debugln(windowFrontRight);

  debug("windowRearLeft: ");
  debugln(windowRearLeft);

  debug("windowRearRight: ");
  debugln(windowRearRight);

  debug("sunroof: ");
  debugln(sunroof);

  debug("trunk: ");
  debugln(trunk);

  debug("hood: ");
  debugln(hood);
}

// Lichtsensor BH1750 und Heimleuchten
void Daemmerung()
{
  if ((millis() - msTimer) >= 800) // 500ms Zeit abgelaufen?
  {
    msTimer = millis();
    // Lese den Wert vom LDR
    // int ldrValue = analogRead(LDR_PIN);

    float ldrValue = lightMeter.readLightLevel();

    // Berechne den Durchschnitt
    sum -= ldrValues[ldrindex];               // Subtrahiere den alten Wert
    ldrValues[ldrindex] = ldrValue;           // Speichere den neuen Wert
    sum += ldrValue;                          // Addiere den neuen Wert
    ldrindex = (ldrindex + 1) % NUM_READINGS; // Aktualisiere den ldrindex

    average = sum / NUM_READINGS; // Berechne den Durchschnitt

    // Entscheide anhand der Hysterese, ob das dunkel flag gesetzt wird
    if (average > UPPER_THRESHOLD && dunkel)
    {
      dunkel = false;
    }
    else if (average < LOWER_THRESHOLD && !dunkel)
    {
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

  if ((dunkel && !Ignition && doorFrontLeft) || (dunkel && heiml))
  {
    debugln("Heimleuchten gesendet!");
    // Ausgabe des Durchschnitts und des dunkel-flags
    debug("Average Lux: ");
    debug(average);
    debug(" - dunkel= ");
    debugln(dunkel);
    // dunkel = true;  // nur zum testen wenn kein LDR angeschlossen ist
    ibusTrx.write(Heimleuchten);
    // Ignition = true;
    doorFrontLeft = false;
    heiml = false;
  }
}

// Kühlmitteltemperatur im Bordmonitor anzeigen
void Coolant(uint8_t coolant)
{
  uint8_t coolantTemp[3];                     // Kühlmitteltemperatur zerlegt und in Hex abgelegt z.B. 128 => 31 32 38
  uint8_t zerlegen = coolant;                 // Hilfs Variable zum zerlegen des Decimalwertes
  // zerlegen von coolant
  for (int8_t i = 2; i >= 0; --i)
  {
    coolantTemp[i] = (zerlegen % 10) + 0x30;  // Die letzte Ziffer der Zahl +30 (in Hex 0x1E) und im Array speichern
    zerlegen /= 10;                           // Eine Ziffer entfernen
  }
  // wenn coolant nur zwei bzw. eine Ziffer hat füge Leerzeichen in HEX voran
  if (coolant < 100)
  {
    coolantTemp[0] = 0x20;
  }
   if (coolant < 10)
  {
    coolantTemp[1] = 0x20;
  }
  // Zeichenkette zusammenbauen: 80 0F E7 24 0E 00 + coolantTemp + B0 43 20 53 45 43 20 + cksum
  memcpy(BCcool, BCcoolbeginn, sizeof(BCcoolbeginn));                                        // BCcoolbeginn in BCcool speichern
  memcpy(BCcool + sizeof(BCcoolbeginn), coolantTemp, sizeof(coolantTemp));                   // hinzufügen von coolantTemp
  memcpy(BCcool + sizeof(BCcoolbeginn) + sizeof(coolantTemp), BCcoolend, sizeof(BCcoolend)); // hinzufügen von B0 43 20 53 45 43 20 (°C)
  BCcool[1] = sizeof(BCcool) - 1;                                                            // länge der gesammten Zeichenkette berechnen und eintragen
  // sende an den Bordmonitor im BC-Bereich anstelle von Timer 2
  ibusTrx.write(BCcool);          //z.b. Ergebnis: 80 0F E7 24 0E 00 20 39 32 B0 43 20 53 45 43 20 CF
}

// Wenn Interrup dann sende die iBus Codes
void ClearToSend()
{
  // debugln("................................................................... Interrupt Clear to send");
  ibusTrx.send(); // Nachricht senden, wenn Interrupt ausgelöst wird
}

// Automatischer Navigations Zoom je nach Geschwindigkeit
void updateNavZoom()
{
  static int previousNavZoomLevel = 0; // Speichert die vorherige Zoomstufe
  int navZoomLevel;
  uint8_t *NavZoom;

  // Definition der IBUS-Codes als uint8_t-Arrays
  static uint8_t NavZoom200m[] = {0xB0, 0x05, 0x7F, 0xAA, 0x10, 0x02};
  static uint8_t NavZoom500m[] = {0xB0, 0x05, 0x7F, 0xAA, 0x10, 0x04};
  static uint8_t NavZoom1km[] = {0xB0, 0x05, 0x7F, 0xAA, 0x10, 0x10};
  static uint8_t NavZoom5km[] = {0xB0, 0x05, 0x7F, 0xAA, 0x10, 0x12};

  // Bestimmen der Zoomstufe und des entsprechenden Ibus-Codes basierend auf der Geschwindigkeit
  if (speed < 56)
  {
    navZoomLevel = 1; // Zoom auf 200m von 0 - 60(digitale 55)km/H
    NavZoom = NavZoom200m;
  }
  else if (speed >= 56 && speed <= 84)
  {
    navZoomLevel = 2; // Zoom auf 500m von 61 - 90(digitale 85)km/H
    NavZoom = NavZoom500m;
  }
  else if (speed >= 85 && speed <= 114)
  {
    navZoomLevel = 3; // Zoom auf 1km von 91 - 120km/H
    NavZoom = NavZoom1km;
  }
  else if (speed > 114)
  {                   // Für Geschwindigkeiten ab 116 km/h
    navZoomLevel = 4; // Zoom auf 5km ab 121km/H
    NavZoom = NavZoom5km;
  }

  // Nur senden, wenn sich die Zoomstufe geändert hat
  if (navZoomLevel != previousNavZoomLevel)
  {
    previousNavZoomLevel = navZoomLevel;
    ibusTrx.write(NavZoom);
  }
}

// Beim Verriegeln Seitenspiegel anklappen
void SpiegelAnklappen()
{
  // Definition der IBUS-Codes für das Spiegelklappen als Byte-Arrays
  static uint8_t einklappenFahrer[] = {0x3F, 0x06, 0x00, 0x0C, 0x01, 0x31, 0x01, 0x04};
  static uint8_t einklappenBeifahrer[] = {0x3F, 0x06, 0x00, 0x0C, 0x02, 0x31, 0x01, 0x07};
  static uint8_t ausklappenFahrer[] = {0x3F, 0x06, 0x00, 0x0C, 0x01, 0x30, 0x01, 0x05};
  static uint8_t ausklappenBeifahrer[] = {0x3F, 0x06, 0x00, 0x0C, 0x02, 0x30, 0x01, 0x06};

  if (sideMirrorDone) // wenn mit dem Funkschlüssel auf oder zu geschlossen wurde
  {
    if (sideMirror == MIRROR_OPEN) // 1 = Aufgeschlossen also ausfahren
    {
      ibusTrx.write(ausklappenFahrer);
      ibusTrx.write(ausklappenBeifahrer);
      sideMirrorDone = false; // Spiegel ausfahren erledigt
    }
    else if (sideMirror == MIRROR_CLOSE)
    {
      ibusTrx.write(einklappenFahrer);
      ibusTrx.write(einklappenBeifahrer);
      sideMirrorDone = false;
    }
  }
}

// Blinkersperre zwischen zwei Blinksignalen aufheben
void BlinkerUnblock()
{
  blinkLockedLi = false; // Sperre für links aufheben
  blinkLockedRe = false; // Sperre für rechts aufheben
  debugln("Blinker block aufgehoben");
  /*
  Diese Lösung stellt sicher, dass nach drei Blinks eine Sperre aktiviert wird und erst dann
  aufgehoben wird, wenn über den Timer eine Pause von 1000 ms erreicht ist.
  */
}

// ###############################################################################################
// ########################### iBus Codes ########################################################
// die checksumme muss nicht mit angegeben werden
uint8_t cleanIKE[6] PROGMEM = {0x30, 0x05, 0x80, 0x1A, 0x30, 0x00};                                                                 // 30 05 80 1A 30 00 IKE Anzeige wird gelöscht, wichtig sonst bleibt sie immer an
uint8_t BlinkerRe[13] PROGMEM = {0x3F, 0x0C, 0xD0, 0x0C, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                     // 3F 0F D0 0C 00 00 40 00 00 00 00 00 00
uint8_t BlinkerLi[13] PROGMEM = {0x3F, 0x0c, 0xD0, 0x0C, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                     // 3F 0C D0 0C 00 00 80 00 00 00 00 00 00
uint8_t BlinkerAus[13] PROGMEM = {0x3F, 0x0C, 0xD0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                    // 3F 0F D0 0C 00 00 00 00 00 00 00 00 00
uint8_t LCMdimmReq[] PROGMEM = {0x3F, 0x03, 0xD0, 0x0B};                                                                            // Diagnoseanfrage ans LCM um Helligkeitswert für IKE zu finden
uint8_t LCMBlinkerAdd[2] PROGMEM = {0xFF, 0x00};                                                                                    // Anhängsel nach dem LCMBlinker zusammenbau
uint8_t BCcoolbeginn[6] PROGMEM = {0x80, 0x0E, 0xE7, 0x24, 0x0E, 0x00};                                                             // Kühlmitteltemperatur im Bordmonitor anzeigen Anfangs-Kette
uint8_t BCcoolend[7] PROGMEM = {0xB0, 0x43, 0x20, 0x53, 0x45, 0x43, 0x20};                                                          // Kühlmitteltemperatur im Bordmonitor anzeigen Schluß-Kette (_°C__)
uint8_t ZV_lock[] PROGMEM = {0x3F, 0x05, 0x00, 0x0C, 0x00, 0x0B};                                                                   // Zentralverriegelung öffnen / schließen 3F05000C000B(3D)
uint8_t Heimleuchten[] PROGMEM = {0x3F, 0x0F, 0xD0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x18, 0x44, 0x00, 0x00, 0x00, 0xE5, 0xFF, 0x00};  // Lichter für Heimleuchten: Bremslicht und Nebelleuchten: 3F0FD00C000000001844000000E5FF00
uint8_t Tankinhalt[5] PROGMEM = {0x3F, 0x04, 0x80, 0x0B, 0x0A};                                                                     // Tankinhalt abfragen:	3F 04 80 0B 0A (BA)
uint8_t SthzEIN[5] PROGMEM = {0x3B, 0x04, 0x80, 0x41, 0x12};                                                                        // Standheizung EIN: 3B 04 80 41 12 (EC)
uint8_t SthzAUS[5] PROGMEM = {0x3B, 0x04, 0x80, 0x41, 0x11};                                                                        // Standheizung AUS: 3B 04 80 41 11 (EC)

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

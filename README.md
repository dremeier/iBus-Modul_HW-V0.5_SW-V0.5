# BMW iBus Modul V0.5 – SARA-R5 LTE Telemetrie

Dieses Projekt ermöglicht die Anbindung eines BMW E38 an einen Cloud-basierten MQTT-Server über ein selbst entwickeltes iBus-Modul und ein SARA-R5 LTE-Modul. Damit können Fahrzeugdaten in Echtzeit überwacht und verschiedene Funktionen ferngesteuert werden, wie z.B. GPS-Tracking, Standheizung, Blinker, Spiegel und Zentralverriegelung.

## Hauptfunktionen
- **Sichere MQTT-Kommunikation:** Übertragung von Telemetrie- und Statusdaten zu ioBroker mit TLS-Verschlüsselung.
- **GPS-Daten:** Erfassen und Übermitteln von Position, Geschwindigkeit und Richtung.
- **iBus-Steuerung:** Senden und Empfangen von Nachrichten zur Steuerung von Fahrzeugfunktionen (z.B. Blinker, Spiegel, Standheizung, Zentralverriegelung, US-Tagfahrlicht).
- **Bluetooth-Integration:** Steuerung und Statusabfrage eines Bluetooth-Moduls.
- **NeoPixel-LEDs:** Drei LEDs zeigen den Systemstatus (Netzwerk, MQTT, GPS) an.
- **EEPROM-Speicherung:** Persistente Speicherung wichtiger Parameter (z.B. GPS-Abstand, Einstellungen).
- **Timer-Management:** Nicht-blockierende Timer für verschiedene Aufgaben (Polling, MQTT, Standheizung).

## Hardware
- **Plattform:** Eigenes iBus-Modul HW V0.5
- **Modem:** u-blox SARA-R5 LTE-Modul
- **Sensoren:** BH1750 Lichtsensor, INA219 Spannungs-/Stromsensor
- **LEDs:** NeoPixel

## Abhängigkeiten
- [SparkFun u-blox SARA-R5 Arduino Library](http://librarymanager/All#SparkFun_u-blox_SARA-R5_Arduino_Library)
- Modifizierte iBusTrx-Library (lib/ibustrx)
- ArduinoJson, EEPROM, Snooze, BH1750

## Projektstruktur
- `src/` – Hauptquellcode (z.B. main.cpp, IbusCodes.cpp/h, sara-lte.cpp/h, bluetooth.cpp/h, globals.cpp/h, math_functions.cpp/h)
- `lib/` – Bibliotheken (u.a. SparkFun SARA-R5, ibustrx)
- `include/` – Header für PlatformIO
- `platformio.ini` – PlatformIO-Projektkonfiguration
- `mqtt_secrets.h` – MQTT Zugangsdaten (nicht im Repository enthalten!)

## Inbetriebnahme & Nutzung
1. **Hardware aufbauen** (Schaltplan und Pinbelegung siehe globals.h).
2. **Projekt mit PlatformIO öffnen** und alle Abhängigkeiten installieren.
3. **MQTT Zugangsdaten** in `mqtt_secrets.h` eintragen.
4. **Firmware flashen** (PlatformIO: Upload).
5. **Modul im Fahrzeug anschließen** (iBus, Strom, LTE-Antenne, Sensoren, LEDs).
6. **ioBroker MQTT-Adapter konfigurieren** (Topics siehe Quellcode und Readme_iBus.txt).

## Wichtige Funktionen im Code
- `setup()` – Initialisiert Hardware, verbindet LTE, setzt MQTT auf, abonniert Topics.
- `loop()` – Pollt Sensoren, verarbeitet iBus-Nachrichten, aktualisiert Systemstatus.
- `iBusMessage()` – Zentrale Auswertung und Steuerung aller iBus-Kommandos.
- `publishMetricsData()` – Sendet Telemetrie als JSON via MQTT.
- `readMqttData()` – Verarbeitet eingehende MQTT-Kommandos (z.B. Standheizung, Spiegel, Blinker, US-Tagfahrlicht).

## Beispiel: US-Tagfahrlicht schalten
- MQTT-Topic: `BMW7er/Subscription/usLight` (Payload: `1`=ein, `0`=aus)
- Das Modul liest Block 08 des LCM, setzt Byte 21 und schreibt den Block zurück (siehe Readme_iBus.txt).

## Hinweise
- Die iBus-Kommandos und deren Antworten sind in `Readme_iBus.txt` dokumentiert.
- Die Hardware-Pinbelegung und Timer sind in `globals.h` und `main.cpp` beschrieben.
- Die Firmware ist für PlatformIO/Teensy ausgelegt.

## Autor & Lizenz
- **Autor:** AME
- **Version:** HW 0.5 / SW 0.5 (Stand: 2024)
- **Lizenz:** MIT

---
Weitere Details und Beispiele findest du in `Readme_iBus.txt` und in den Quellcode-Kommentaren.

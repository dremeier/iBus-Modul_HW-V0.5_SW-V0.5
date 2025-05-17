Jedes Netzwerk (K-Bus, I-Bus und D-Bus) überträgt Daten mit einer Baudrate von 9600 Bit pro Sekunde. Das Kommunikationsprotokoll hat 8 Daten-Bits, 1 Stop-Bit und gerade Parität (even).

Implementierte Codes (alle ohne CK):

Funkschlüssel:
00 04 BF 72 22              Schlüssel Cordula erkannt
00 04 BF 72 12              Tschüß Cordula

00 04 BF 72 27              Schlüssel Andre auf erkannt
00 04 BF 72 17              Tschüß Andre zu

00 04 BF 72 2B              3. Schlüssel auf
00 04 BF 72 0B              3. schlüssel zu

TODO: überprüfen
Schlüssel im Schloß , Motor aus, Tankinhalt // 44 05 bf 74 xx xx
44 05 bf 74 04 xx           Schlüssel wird ins Schloß gesteckt
44 05 bf 74 04 00           "gute Fahrt Cordula"
44 05 bf 74 04 05           Andre
44 05 bf 74 05 xx           Motor aus
44 05 bf 74 05 00           Schlüssel stellung 1 -> 0, Corula, bis bald Cordula
44 05 bf 74 05 05           Schlüssel stellung 1 -> 0, Andre
44 05 bf 74 05 04           Schlüssel stellung 1 -> 0, 3. Schlüssel
44 05 bf 74 04 04           Schlüssel stellung 0 -> 1, 3. Schlüssel
44 05 BF 74 00 FF           abgezogen, No_key_detected Vehicle_immobilised
BIT0, BIT1
0x00, 0xFF // No_key_detected Vehicle_immobilised
0x01, 0xFF // Immobilisation_deactivated Vehicle_immobilised
0x04, 0x04 // Valid_key_detected Key_4
0x05, 0x04 // Immobilisation_deactivated Valid_key_detected Key_4


Codes überprüfen in Real
80 04 BF 11 00     Zündung Aus
80 04 BF 11 01     Zündung Pos.1
80 04 BF 11 03     Zündung Pos.2
80 04 BF 11 07     Zündung Pos.3 Start

TODO: !Tankinhalt abfragen:	3F 04 80 0B 0A (BA) IKE (Antwort noch ermitteln!!!) keine Antwort

00 05 BF 7A 51          status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)

##########################################################################################################################
Blinker: Wenn Blinkerpfeil im IKE links: D0 07 BF 5B 20 00 04 00 (17) oder Blinkerpfeil im IKE rechts:  D0 07 BF 5B 40 00 04 00 (77)
     D0 07 BF 5B 40 00 04 00 77
                  |  |  |  |  |
                  |  |  |  |  Checksum
                  |  |  |Blinkstatus ist immer 04 , databytes[3]
                  |  |nil
                  |Blinkbyte kann sein: links-> 20, 21, 23, 27, 2B, 33, 3B, rechts-> 40, 41, 43, 47, 4B, 53, 5B


D0 07 BF 5B 01 00 00 00  ???????????? kommt immer automatisch auf dem Bus


 Anfrage Dimm-Wert des IKE Cluster vom LCM: 3F 03 D0 0B (E7)  
 Antwort:   (D0 xx 3F A0 |-> C1 C0 00 20 00 00 00 00 00 A0 00 00 88 14 84 E4 .....)
             D0 xx 3F A0     C1 C0 00 20 00 00 00 00 00 A0 00 00 88 14 84 E4 00 
             D0 23 3F A0     C1 C0 80 20 00 00 80 40 00 A3 00 00 00 00 00 10 FF 00 00 00 00 00 00 00 00 00 00 00 00 FD 4E FE 
             D0 23 3F A0     C1 C0 A0 20 00 03 AE 58 0E A2 00 00 00 00 00 24 FF 00 00 00 00 00 00 00 00 00 00 00 00 FD 4F FE 
             D0 23 3F A0     C1 C0 20 20 00 03 2E 18 0E A3 00 00 00 00 00 C1 FF 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FE 
                                                                           |-> Byte 16, message.b(16)

Blinker-Code zusammenbau:
3F 1E D0 0C 00 00 40 00 00 00 00 00 00 C1 C0 20 20 00 03 2E 18 0E A3 00 00 00 00 00 C1 FF 00 1A

Altes Modul HW2.0:
-> D0 07 BF 5B 40 00 04 00 77
<- 3F 03 D0 0B E7
-> D0 23 3F A0 C1 C0 20 20 00 03 2E 18 0E A3 00 00 00 00 00 C1 FF 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FE 35
<-                   3F 0F D0 0C 00 00 40 00 00 00 00 00 00 C1 FF 00 92

-> D0 23 3F A0 C1 C0 A0 20 00 03 AE 58 0E A2 00 00 00 00 00 24 FF 00 00 00 00 00 00 00 00 00 00 00 00 FD 4F FE
<-                   3F 0F D0 0C 00 00 40 00 00 00 00 00 00 24 FF 00 77

 #############################################################################################################################

############################ Funktion zum An-/Ausschalten für das US-Standlicht einbauen. Hierzu muss das Byte 21 in Block 08 des LCM codiert werden.

Byte 21 == 00 US-Tagfahrlicht aus
Byte 21 == 28 US-Tagfahrlicht ein

Auslesen des Blocks mit:
3F 04 D0 08 00 CS

Antwort:
D0 23 3F A0 ....... CS
also:
D0 23 3F A0 00 86 00 08 01 00 64 74 00 00 00 00 00 00 00 B0 00 55 40 90 00 00 0B 00 00 00 00 28 1F 00 00 00 (FA)
																		 |-> Byte 21, message.b(21)

Ohne US-Tagfahrlicht kommt sowas in der Art :
00 86 00 08 01 00 64 74 00 00 00 00 00 00 00 B0 00 55 40 90 00 00 0B 00 00 00 00 28 1F 00 00 00
                                                             | -> Byte 21

Schreiben des Blocks mit:
3F 23 D0 09 ....... CS

Für US-Tagfahrlicht muss dann dieser Block gesendet werden:
00 86 00 08 01 00 64 74 00 00 00 00 00 00 00 B0 00 55 40 90 28 00 0B 00 00 00 00 28 1F 00 00 00
                                                             |-> Byte 21
z.B.:
3F 23 D0 09 00 86 00 08 01 00 64 74 00 00 00 00 00 00 00 B0 00 55 40 90 28 00 0B 00 00 00 00 28 1F 00 00 00
######################################################################################################################


Geschwindigkeit, RPM, OutTemp, CoolantTemp
    80 05 BF 18 3F 18 (05),IKE --> GLO: Speed/RPM, Speed 126 km/h  [78 mph], 2,400 RPM
    80 05 BF 19 2F 5C (50),IKE --> GLO Temperature Outside 47C Coolant 92C


Navigations dreh-Knopf, push in:
F0 04 3B 48 05 82   -> ("Hallo BMW iBus")


### Prüfung Automatisch-Verriegeln:
Speed > 20KM ->>>   80 05 BF 18 3F 18
speed =0 ->>>       80 05 BF 18 00 18
speed und RPM =0 ->>> 80 05 BF 18 00 00
MotorOFF ->>>       80 05 BF 11 00 2A
80 05 BF 11 01 2B      Zündung Pos.1
ZVunlocked, Tür Vorne links auf ->>>       00 05 BF 7A 11 3A 
ZVunlocked, alle Türen zu       ->>>       00 05 BF 7A 10 3A 
ZVlocked, alle Türen zu         ->>>       00 05 BF 7A 20 3A

### Prüfung NaviZomm test:
speed =0  ->>>      80 05 BF 18 00 00   erwarte: B0 05 7F AA 10 02 72  NavZoom200m
speed =22 ->>>      80 05 BF 18 0B 09   erwarte: B0 05 7F AA 10 02 72  NavZoom200m
Speed =56 ->>>      80 05 BF 18 1C 15   erwarte: B0 05 7F AA 10 04 74  NavZoom500m
speed =70 ->>>      80 05 BF 18 23 18   erwarte: B0 05 7F AA 10 04 74  NavZoom500m
speed =90 ->>>      80 05 BF 18 2D 18   erwarte: B0 05 7F AA 10 10 60  NaviZomm1km
speed =120 ->>>     80 05 BF 18 3C 18   erwarte: B0 05 7F AA 10 12 62  NaviZomm5km


### prüfung US-Tagfahrlicht
Über ioBroker 1 oder 0 senden
-> Modul fragt den Block ab: 3F 04 D0 08 00
<- Antwort:                 D0 23 3F A0 00 86 00 08 01 00 64 74 00 00 00 00 00 00 00 B0 00 55 40 90 00 00 0B 00 00 00 00 28 1F 00 00 00
-> Modul ersetzt Byte 21:   3F 23 D0 09 00 86 00 08 01 00 64 74 00 00 00 00 00 00 00 B0 00 55 40 90 28 00 0B 00 00 00 00 28 1F 00 00 00 (7B)


## prüfung Getriebestatus:
80 0A BF 13 02 B0 00 02 00 00 5A -> Park
80 0A BF 13 02 D0 00 02 00 00 5A -> 3.Gang

### Neueberechnung der Consumption1
3B 04 80 41 04 10 xx

### Standheizung Status:
80 05 E7 2A 00 20 - ein
80 05 E7 2A 00 00 - aus


TODO: 
3. Wenn kein GSM-Empfang besteht und du darüber informiert werden möchtest, könntest du dies im Monitor unten rechts anzeigen lassen (ich hatte das bei mir mit dem GPS-Signal gemacht):
Code:
80 0F E7 24 02 00 4E 6F 20 47 53 4D 20 20 20 CS
Dieser Code sollte dort „No GSM“ anzeigen.
static uint8_t NoGSM[]= {0x80, 0x0F, 0xE7, 0x24, 0x02, 0x00, 0x4E, 0x6F, 0x20, 0x47, 0x53, 0x4D, 0x20, 0x20, 0x20};
ibusTrx.write(NoGSM);
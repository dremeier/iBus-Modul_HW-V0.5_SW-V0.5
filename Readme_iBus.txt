Implementierte Codes (alle ohne CK):

Funkschlüssel:
00 04 BF 72 22              Schlüssel Cordula erkannt
00 04 BF 72 26              Schlüssel Andre erkannt
00 04 BF 72 12              Tschüß Cordula
00 04 BF 72 16              Tschüß Andre

00 05 BF 7A 51              status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)


Schlüssel im Schloß , Motor aus, Tankinhalt // 44 05 bf 74 xx xx
44 05 bf 74 05 xx             Motor aus
44 05 bf 74 05 00             Schlüssel stellung 1 -> 0, Corula, bis bald Cordula
44 05 bf 74 05 05             Schlüssel stellung 1 -> 0, Andre

!Tankinhalt abfragen:	3F 04 80 0B 0A (BA)  (Antwort noch ermitteln!!!)

44 05 bf 74 04 xx       Schlüssel wird ins Schloß gesteckt
44 05 bf 74 04 00       "gute Fahrt Cordula"
44 05 bf 74 04 05       Andre

00 05 BF 7A 51          status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)


Blinker: Wenn Blinkerpfeil im IKE links: D0 07 BF 5B 20 00 04 00 17 oder Blinkerpfeil im IKE rechts:  D0 07 BF 5B 40 00 04 00 77
     D0 07 BF 5B 40 00 04 00 77
                  |  |  |  |  |
                  |  |  |  |  Checksum
                  |  |  |Blinkstatus ist immer 04 , databytes[3]
                  |  |nil
                  |Blinkbyte kann sein: links-> 20, 21, 23, 27, 2B, 33, 3B, rechts-> 40, 41, 43, 47, 4B, 53, 5B

 Anfrage Dimm-Wert des IKE Cluster vom LCM: 3F 03 D0 0B (E7)                   


Geschwindigkeit, RPM, OutTemp, CoolantTemp
    80 05 BF 18 3F 18 (05),IKE --> GLO: Speed/RPM, Speed 126 km/h  [78 mph], 2,400 RPM
    80 05 BF 19 2F 5C (50),IKE --> GLO Temperature Outside 47C Coolant 92C

80 05 BF 11 00 2A      Zündung Aus
80 05 BF 11 01 2B      Zündung Pos.1
80 05 BF 11 03 29      Zündung Pos.2

Navigations dreh-Knopf, push in:
F0 04 3B 48 05 82   -> ("Hallo BMW iBus")


### Prüfung Automatisch-Verriegeln:
Speed > 20KM ->>>   80 05 BF 18 3F 18
speed =0 ->>>       80 05 BF 18 00 18
MotorOFF ->>>       80 05 BF 11 00 2A

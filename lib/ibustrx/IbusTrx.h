/*
  a fork from IbusTrx (v2.4.0)
  Arduino library for sending and receiving messages over the BMW infotainment bus (IBUS).
  Author: A.Meier 12/2023 
  THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. WIP
*/
#ifndef IbusTrx_h
#define IbusTrx_h

#include "Arduino.h"
#include "IbusMessage.h"
#include "IbusNames.h"
#include <cppQueue.h>


class IbusTrx{
  public:
    void begin(HardwareSerial &userPort);
    void end();
    void write(uint8_t message[]);            // senden der Ibus Message in eine Warteschlange mit Checksumm calculation
    void writeTxt(const char txtmessage[]);   // senden ener Text Nachricht an das IKE
    void writefix(uint8_t message[], size_t arrayLength);          // direktes senden eines fixen arrays ohne Checksum calculation
    void send();                              // warte auf senSta low also Clear to send um zu senden
    void senStapin(uint8_t pin);              // übergeben des Pins aus der Main.cpp an die Lib
    bool available();                         // empfangen und prüfen einer Nachricht vom iBus
    bool transmitWaiting();
    uint8_t length();
    IbusMessage readMessage();
    
  private:
    HardwareSerial* serialPort;
    void clearBuffer();
    bool checkMessage();
    bool tx_msg_waiting = false;      // message waiting in transmit buffer
    bool rx_msg_waiting = false;      // message waiting in receive buffer
    uint8_t rx_buffer[0x40] = {0x00}; // receive bufer
    uint8_t tx_buffer[0x40] = {0x00}; // transmit buffer, hier wie die maximale Größe angegeben, 0x20 =32 bytes also 32 hex werte können maximal gesendet werden, 0xC1=193
    uint8_t rx_bytes = 0;             // number of bytes in receive buffer
    uint8_t tx_bytes = 0;             // number of bytes in transmit buffer
    uint32_t t_last_rx_byte = 0;      // timestamp of last byte received
    uint8_t senSta;                   // senSta = Clear to Send from the TH3122 Lin Transceiver 
};

void ClearToSend();

#endif

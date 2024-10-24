//MyINA219.cpp
//Code fuer Arduino und Attiny
//Author Retian
//Version 1.0

/*
Abfrage eines Stromsensors INA219

MyMINA219 Name(I2C-Adresse);

Beispiel siehe unter: 
https://arduino-projekte.webnode.at/meine-libraries/stromsensor-ina219/

Funktionen siehe unter:
https://arduino-projekte.webnode.at/meine-libraries/stromsensor-ina219/funktionen/

*/

//*************************************************************************
//*************************************************************************

#include "Arduino.h"
#include "MyINA219.h"

MyINA219::MyINA219(byte i2CAdd)
{
  _i2CAdd  = i2CAdd;
  Wire.begin();
}

//*************************************************************************
//INA219 vorhanden ?

bool MyINA219::isReady()
{
  Wire.beginTransmission(_i2CAdd);
  if (Wire.endTransmission()== 0) return true;
  else return false;
}

//*************************************************************************
//Initialisierung des INA219

void MyINA219::init()
{
  byte pgaBits; //PGA-Bits
  
  //Ermitteln der PGA-Bits im Configurationsregister
  configReg = read16(INA219_CONFIG_REG);
  pgaBits = (byte)((configReg & 0b0001100000000000) >> 11);

  //Ermitteln des maximalen Stromes iMax
  if (pgaBits == 0b11) iMax = 0.3199 / INA219_SHUNT;
  else if (pgaBits == 0b10) iMax = 0.1599 / INA219_SHUNT;
  else if (pgaBits == 0b01) iMax = 0.07999 / INA219_SHUNT;
  else if (pgaBits == 0b00) iMax = 0.03999 / INA219_SHUNT;

  //Berechnen von LSB_Min (lsbMin) und
  //dem Wert des Kalibrierungsregisters (calReg)
  lsbMin = iMax / 32767.0;
  calReg = (int)(0.04096 / (lsbMin * INA219_SHUNT));
  
  //Setzen des Kalibrierungsregisters
  write16(INA219_CALIBRATION_REG, calReg);
  //Serial.println("INIT");
}

//*************************************************************************
//Setzen des Shunt-Widerstandeswertes

void MyINA219::setShuntResistor(float shunt)
{
  INA219_SHUNT = shunt;
  init();
}

//*************************************************************************
//Setzen des Betriebsmodus

void MyINA219::setMode(byte mode)
{
  byte _mode = mode;

  configReg = read16(INA219_CONFIG_REG);
  if (bitRead(_mode, 2)) configReg |= (1 << INA219_MODE3);
  else configReg &= ~(1 << INA219_MODE3);
  if (bitRead(_mode, 1)) configReg |= (1 << INA219_MODE2);
  else configReg &= ~(1 << INA219_MODE2);
  if (bitRead(_mode, 0)) configReg |= (1 << INA219_MODE1);
  else configReg &= ~(1 << INA219_MODE1);
  write16(INA219_CONFIG_REG, configReg);
}

//*************************************************************************
//Setzen der Bit-Aufloesung oder Anzahl der Samples der Shunt-Spannung 

void MyINA219::setShuntADC(byte sadc)
{
  byte _sadc = sadc;

  configReg = read16(INA219_CONFIG_REG);
  if (bitRead(_sadc, 3)) configReg |= (1 << INA219_SADC4);
  else configReg &= ~(1 << INA219_SADC4);
  if (bitRead(_sadc, 2)) configReg |= (1 << INA219_SADC3);
  else configReg &= ~(1 << INA219_SADC3);
  if (bitRead(_sadc, 1)) configReg |= (1 << INA219_SADC2);
  else configReg &= ~(1 << INA219_SADC2);
  if (bitRead(_sadc, 0)) configReg |= (1 << INA219_SADC1);
  else configReg &= ~(1 << INA219_SADC1);
  write16(INA219_CONFIG_REG, configReg);
}

//*************************************************************************
//Setzen der Bit-Aufloesung oder Anzahl der Samples der Bus-Spannung 

void MyINA219::setBusADC(byte badc)
{
  byte _badc = badc;

  configReg = read16(INA219_CONFIG_REG);
  if (bitRead(_badc, 3)) configReg |= (1 << INA219_BADC4);
  else configReg &= ~(1 << INA219_BADC4);
  if (bitRead(_badc, 2)) configReg |= (1 << INA219_BADC3);
  else configReg &= ~(1 << INA219_BADC3);
  if (bitRead(_badc, 1)) configReg |= (1 << INA219_BADC2);
  else configReg &= ~(1 << INA219_BADC2);
  if (bitRead(_badc, 0)) configReg |= (1 << INA219_BADC1);
  else configReg &= ~(1 << INA219_BADC1);
  write16(INA219_CONFIG_REG, configReg);
}

//*************************************************************************
//Setzen des Messbereichs der Shunt-Spannung

void MyINA219::setGain(byte gain)
{
  byte _gain = gain;
  
  configReg = read16(INA219_CONFIG_REG);
  if (bitRead(_gain, 1)) configReg |= (1 << INA219_PG1);
  else configReg &= ~(1 << INA219_PG1);
  if (bitRead(_gain, 0)) configReg |= (1 << INA219_PG0);
  else configReg &= ~(1 << INA219_PG0);
  write16(INA219_CONFIG_REG, configReg);
  init();
}

//*************************************************************************
//Abfrage des Messbereichs der Shunt-Spannung

byte MyINA219::getGain()
{
  byte gain;
  byte val;
  
  configReg = read16(INA219_CONFIG_REG);
  gain = (configReg & 0b0001100000000000) >> 11;
  switch (gain) {
  case 0:
    val = 4;
    break;
  case 1:
    val = 8;
    break;
  case 2:
    val = 16;
    break;
  case 3:
    val = 32;
    break;
  }
  return val;
}

//*************************************************************************
//Setzen des Messbereichs der Bus-Spannung

void MyINA219::setRange(byte range)
{
  byte _range = range;
  
  configReg = read16(INA219_CONFIG_REG);
  if (_range) configReg |= (1 << INA219_BRNG);
  else configReg &= ~(1 << INA219_BRNG);
  write16(INA219_CONFIG_REG, configReg);
}

//*************************************************************************
//Abfrage des Messbereichs der Bus-Spannung

byte MyINA219::getRange()
{
  byte range;
  byte val;

  configReg = read16(INA219_CONFIG_REG);
  range = (configReg & 0b0010000000000000) >> 13;
  switch (range) {
  case 0:
    val = 16;
    break;
  case 1:
    val = 32;
    break;
  }
  return val;
}

//*************************************************************************
//Abfrage, ob die Wandlung beendet ist

bool MyINA219::conversionReady()
{
  bool cnvrBit;

  busVoltageRaw = read16(INA219_BUS_VOLTAGE_REG);
  cnvrBit = (byte)((busVoltageRaw & 0x02) >> 1);
  return cnvrBit;
}

//*************************************************************************
//Abfrage, des Strom-Overflow-Bits

bool MyINA219::currentOverflow()
{
  bool ovfBit;

  busVoltageRaw = read16(INA219_BUS_VOLTAGE_REG);
  ovfBit = (byte)(busVoltageRaw & 0x01);
  return ovfBit;
}

//*************************************************************************
//Abfrage, ob ein Bus-Spannungs-Overflow aufgetreten ist

bool MyINA219::voltageOverflow()
{
  bool ovfBit = false;
  float busVoltage;
  byte range;
  
  busVoltageRaw = read16(INA219_BUS_VOLTAGE_REG);
  busVoltage = calcBusVoltage();

  range = getRange();
  
  if(range == 16)
  {
    if (busVoltage >= 16.0) ovfBit = true;
  }
  else if (range == 32)
  {
    if (busVoltage >= 26.0) ovfBit = true;
  }
  return ovfBit;
}

//*************************************************************************
//Start einer Einzelmessung 

void MyINA219::startSingleMeas()
{
  byte mode = INA219_SHUNT_BUS_VOLTAGE_TRIG;

  startSingleMeas(mode);
}

void MyINA219::startSingleMeas(byte mode)
{
  byte _mode = mode;

  if (_mode == INA219_SHUNT_BUS_VOLTAGE_TRIG
     || mode == INA219_BUS_VOLTAGE_TRIG
     || mode == INA219_SHUNT_VOLTAGE_TRIG) setMode(_mode);
}

//*************************************************************************
//Starten der kontinuierlichen Messung

void MyINA219::startContinuousMeas()
{
  byte mode = INA219_SHUNT_BUS_VOLTAGE_CONT;
  startContinuousMeas(mode);
}

void MyINA219::startContinuousMeas(byte mode)
{
  byte _mode = mode;

  if (_mode == INA219_SHUNT_BUS_VOLTAGE_CONT
     || mode == INA219_BUS_VOLTAGE_CONT
     || mode == INA219_SHUNT_VOLTAGE_CONT) setMode(_mode);
}

//*************************************************************************
//Ausloesung eines Software-Reset des INA219

void MyINA219::reset()
{
  configReg = read16(INA219_CONFIG_REG);
  configReg |= 0x8000;
  write16(INA219_CONFIG_REG, configReg);
}

//*************************************************************************
//Lesen der Rohwerte

void MyINA219::readRawValues()
{
  shuntVoltageRaw = read16(INA219_SHUNT_VOLTAGE_REG);
  busVoltageRaw = read16(INA219_BUS_VOLTAGE_REG);
  currentRaw = read16(INA219_CURRENT_REG);
  powerRaw = read16(INA219_POWER_REG);
}

//*************************************************************************
//Berechnen von Shunt- und Bus-Spannung, Strom und Leistung

float MyINA219::calcShuntVoltage()
{
  return (float)(shuntVoltageRaw / 100.0);
}

float MyINA219::calcBusVoltage()
{
  return (float)((busVoltageRaw >> 3) * 4 / 1000.0);
}

float MyINA219::calcCurrent()
{
  return (float)(currentRaw * lsbMin * 1000.0);
}

float MyINA219::calcPower()
{
  return (float)(powerRaw * 20.0 * lsbMin * 1000.0);
}

//*************************************************************************
//Setzen/Ruecksetzen des Schlafmodus (power-down mode)

void MyINA219::setPowerDown(bool mode)
{
  bool _mode = mode;

  configReg = read16(INA219_CONFIG_REG);
  if (_mode == INA219_POWER_DOWN_ON)
  {
    oldMode = configReg & 0b111;
    setMode(INA219_POWER_DOWN);
  }
  else if (_mode == INA219_POWER_DOWN_OFF)
  {
    setMode(oldMode);
    delayMicroseconds(50); //40 µs lt. Datenblatt nach Power-Down!
  }
}

//*************************************************************************
//Setzen/Ruecksetzen der ADC-Aktivitaet

void MyINA219::setADCDown(bool mode)
{
  bool _mode = mode;

  configReg = read16(INA219_CONFIG_REG);
  if (_mode == INA219_ADC_DOWN_ON)
  {
    oldMode = configReg & 0b111;
    setMode(INA219_ADC_OFF);
  }
  else if (_mode == INA219_ADC_DOWN_OFF)
  {
    setMode(oldMode);
    delayMicroseconds(50);
  }
}

//*************************************************************************
//Ausgabe der INA219 Register am Seriellen Monitor
//Nicht fuer Attiny!

void MyINA219::printConfigRegister()
{
#if defined_ATtiny
#else
	
  unsigned int inaReg;
  
  Serial.print("Conf.Reg.: ");
  inaReg = read16(INA219_CONFIG_REG);
  Serial.println(inaReg, BIN);
#endif
}

void MyINA219::printAllRegister()
{
#if defined_ATtiny
#else

  unsigned int inaReg;

  Serial.print("Conf.Reg.   : ");
  inaReg = read16(INA219_CONFIG_REG);
  Serial.println(inaReg, BIN);
  Serial.print("Shunt_V.Reg.: ");
  inaReg = read16(INA219_SHUNT_VOLTAGE_REG);
  Serial.println(inaReg, BIN);
  Serial.print("BUS_V.Reg.  : ");
  inaReg = read16(INA219_BUS_VOLTAGE_REG);
  Serial.println(inaReg, BIN);
  Serial.print("PowerReg.   : ");
  inaReg = read16(INA219_POWER_REG);
  Serial.println(inaReg, BIN);
  Serial.print("CurrentReg. : ");
  inaReg = read16(INA219_CURRENT_REG);
  Serial.println(inaReg, BIN);
  Serial.print("Cal.Reg.    : ");
  inaReg = read16(INA219_CALIBRATION_REG);
  Serial.println(inaReg, BIN);
#endif
}

//*************************************************************************
//Schreiben eines 16 Bit Registers des INA219
//Interne Verwendung

void MyINA219::write16(byte reg, unsigned int val)
{
  byte _reg = reg;
  unsigned int _val = val;

  Wire.beginTransmission(_i2CAdd);
  Wire.write(_reg);
  Wire.write((byte)highByte(_val));
  Wire.write((byte)lowByte(_val));
  Wire.endTransmission();
}

//*************************************************************************
//Lesen eines 16 Bit Registers des INA219
//Interne Verwendung

unsigned int MyINA219::read16(byte reg)
{
  byte _reg = reg;
  byte hByte, lByte;

  Wire.beginTransmission(_i2CAdd);
  Wire.write(_reg);
  Wire.endTransmission();
  delay(1);
  Wire.requestFrom((int)_i2CAdd, 2);
  if (Wire.available())
  {
    hByte = Wire.read();
    lByte = Wire.read();
  }
  return ((int)(hByte << 8) + lByte);
}






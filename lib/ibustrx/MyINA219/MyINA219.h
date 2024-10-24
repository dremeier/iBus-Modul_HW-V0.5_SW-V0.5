//MyINA219.h

#ifndef MyINA219_h
#define MyINA219_h

#define defined_ATtiny (defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny84__))

#if defined_ATtiny
  #include "TinyWireM.h"
  #define Wire TinyWireM
#else
  #include <Wire.h>
  #include <SPI.h>
#endif

//INA219 Register
#define INA219_CONFIG_REG        0x00
#define INA219_SHUNT_VOLTAGE_REG 0x01
#define INA219_BUS_VOLTAGE_REG   0x02
#define INA219_POWER_REG         0x03
#define INA219_CURRENT_REG       0x04
#define INA219_CALIBRATION_REG   0x05

//Bit-Nummer und -Bezeichnung INA219_CONFIGURATION_REG
#define INA219_MODE1 0x00 //Default-Wert = 1
#define INA219_MODE2 0x01 //Default-Wert = 1
#define INA219_MODE3 0x02 //Default-Wert = 1
#define INA219_SADC1 0x03 //Default-Wert = 1
#define INA219_SADC2 0x04 //Default-Wert = 1
#define INA219_SADC3 0x05 //Default-Wert = 0
#define INA219_SADC4 0x06 //Default-Wert = 0
#define INA219_BADC1 0x07 //Default-Wert = 1
#define INA219_BADC2 0x08 //Default-Wert = 1
#define INA219_BADC3 0x09 //Default-Wert = 0
#define INA219_BADC4 0x0A //Default-Wert = 0
#define INA219_PG0   0x0B //Default-Wert = 1
#define INA219_PG1   0x0C //Default-Wert = 1
#define INA219_BRNG  0x0D //Default-Wert = 1
#define INA219_RST   0x0F //Default-Wert = 0

//Bit-Nummer und -Bezeichnung INA219_BUS_VOLTAGE_REG
#define INA219_OVF 0x00  //Overflow Flag
#define INA219_CNVR 0x01 //Conversion Ready Flaf

//INA219_CONFIGURATION_REG Einstellungen
#define INA219_CALIBRATION_REG_DEFAULT 0b0011100110011111
//INA219 Modus-Einstellungen
#define INA219_POWER_DOWN             0b000
#define INA219_SHUNT_VOLTAGE_TRIG     0b001
#define INA219_BUS_VOLTAGE_TRIG       0b010
#define INA219_SHUNT_BUS_VOLTAGE_TRIG 0b011
#define INA219_ADC_OFF                0b100
#define INA219_SHUNT_VOLTAGE_CONT     0b101
#define INA219_BUS_VOLTAGE_CONT       0b110
#define INA219_SHUNT_BUS_VOLTAGE_CONT 0b111

//INA219 ADC-Einstellungen
#define INA219_ADC_9_BIT       0b0000
#define INA219_ADC_10_BIT      0b0001
#define INA219_ADC_11_BIT      0b0010
#define INA219_ADC_12_BIT      0b0011
#define INA219_ADC_2_SAMPLES   0b1001
#define INA219_ADC_4_SAMPLES   0b1010
#define INA219_ADC_8_SAMPLES   0b1011
#define INA219_ADC_16_SAMPLES  0b1100
#define INA219_ADC_32_SAMPLES  0b1101
#define INA219_ADC_64_SAMPLES  0b1110
#define INA219_ADC_128_SAMPLES 0b1111
//INA219 PGA (Programmable Gain Amplifier) Shunt Voltage Einstellungen
#define INA219_GAIN_40   0b00 //PGA = 1;  max. Shuntspannung +/- 40mV
#define INA219_GAIN_80   0b01 //PGA = /2; max. Shuntspannung +/- 80mV
#define INA219_GAIN_160  0b10 //PGA = /4; max. Shuntspannung +/- 160mV
#define INA219_GAIN_320  0b11 //PGA = /8; max. Shuntspannung +/- 320mV
//INA219 FSR (Full Scale Range) Bus Voltage Einstellungen
#define INA219_RANGE_16 0b0 //FSR = 16V
#define INA219_RANGE_32 0b1 //FSR = 32V

#define INA219_ADC_DOWN_OFF   0
#define INA219_ADC_DOWN_ON    1
#define INA219_POWER_DOWN_OFF 0
#define INA219_POWER_DOWN_ON  1

#include "Arduino.h"

class MyINA219
{
  public:
    MyINA219(byte);
    bool isReady(void);
    void init(void);
    void setMode(byte);
    bool conversionReady(void);
    bool currentOverflow(void);
    bool voltageOverflow(void);
    void startSingleMeas(void);
    void startSingleMeas(byte);
    void startContinuousMeas(void);
    void startContinuousMeas(byte);
    void setShuntResistor(float);
    void setShuntADC(byte);
    void setBusADC(byte);
    void setGain(byte);
    byte getGain(void);
    void setRange(byte);
    byte getRange(void);
    void reset(void);
    float calcShuntVoltage(void);
    float calcBusVoltage(void);
    float calcCurrent(void);
    float calcPower(void);
    void readRawValues(void);
    void printConfigRegister(void);
    void printAllRegister(void);
    void setPowerDown(bool);
    void setADCDown(bool);
        
  private:
    void write16(byte, unsigned int);
    unsigned int read16(byte);
      
    byte _i2CAdd;
    float INA219_SHUNT = 0.1; //Ohm
    unsigned int configReg, calReg;
    unsigned int busVoltageRaw; 
    int shuntVoltageRaw, currentRaw, powerRaw;
    float lsbMin, iMax;
    byte oldMode = 0b111;
};

#endif


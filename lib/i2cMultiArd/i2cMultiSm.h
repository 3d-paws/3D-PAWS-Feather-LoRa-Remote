/*
 * vcs3i2c.h
 *
 */

#ifndef SVMULTI_H_
#define SVMULTI_H_

#include <Arduino.h>
#include <Wire.h>

#define  REG_READ_START    0x01
#define  REG_GET_DATA      0x09

#define  REG_READ_E25     0x02
#define  REG_READ_EC      0x03
#define  REG_READ_TEMP    0x04
#define  REG_READ_VWC     0x05

#define  REG_CALIBRATE_AIR    0x06
#define  REG_CALIBRATE_WATER  0x07
#define  REG_CALIBRATE_EC  0x10

#define  REG_RES     0x0B

#define REG_SET_I2C_ADDR    0x08

#define MULTI_ZONES 5
#define MULTI_TEMPS 2

  typedef struct{
	  int16_t temp[MULTI_TEMPS];
	  int16_t dp[MULTI_ZONES];
  } vals_t;

  typedef struct{
	  float temp[MULTI_TEMPS];
	  float dp[MULTI_ZONES];
	  float vwc[MULTI_ZONES];
  } soil_ret_t;

class SVMULTI
{
public:
  SVMULTI();
  int init(int address, TwoWire *the_wire);
  int init(int address);
  int newAddress(byte newAddr);
  int calibrationAir();
  int calibrationWater();
  int newReading();
  void getData(soil_ret_t *data);
  void getRaw(vals_t *vals);
private:
  TwoWire *_wire;
  uint16_t addr;
  int getState();
  int16_t getVal(byte reg);
  int setReg8(byte reg, byte val);
  int setReg(byte reg);
  bool i2cdelay(int size);

};

#endif /* VCS3I2C_H_ */


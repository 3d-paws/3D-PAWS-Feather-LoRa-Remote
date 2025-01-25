/*
 * vcs3i2c.cpp
 *
 */
#include "i2cMultiSm.h"


SVMULTI::SVMULTI(){
  addr=101;
}

int SVMULTI::init(int address, TwoWire *the_wire){
  _wire = the_wire;
  addr = address;
  return 0;
}

int SVMULTI::init(int address){
  _wire = &Wire;
  _wire->begin();
  addr = address;
  return 0;
}


bool SVMULTI::i2cdelay(int size){
  int i=0;
  for (;_wire->available() < size && i<=size;i++) {
	  delay(2);
  }
  if(i>=size){
	  return false;
  }else{
	  return true;
  }
}

int SVMULTI::getState(){ //-1:no data, 0:err, 1:ok
  _wire->requestFrom(addr, (uint8_t)1);
   if(i2cdelay(1)){
    return _wire->read();
  }else{
    return -1;
  }
}

int16_t SVMULTI::getVal(byte reg){

  _wire->beginTransmission(addr); // transmit to device
  _wire->write(reg);              // sends one byte
  _wire->endTransmission();    // stop transmitting
 
  _wire->requestFrom(addr, (uint8_t)2);
  int16_t ret=0;
  if(i2cdelay(2)){
	byte *pointer = (byte *)&ret;
    pointer[0] = _wire->read();
    pointer[1] = _wire->read();
  }
  return ret;
}


int SVMULTI::setReg8(byte reg, byte val){
  _wire->beginTransmission(addr); // transmit to device
  _wire->write(reg);              // sends one byte
  _wire->write(val);              // sends one byte
  _wire->endTransmission();    // stop transmitting
  return getState();
}

int SVMULTI::setReg(byte reg){
  _wire->beginTransmission(addr); // transmit to device
  _wire->write(reg);              // sends one byte
  _wire->endTransmission();    // stop transmitting
  delay(2);
  return getState();
}



int SVMULTI::calibrationAir(){
  return setReg(REG_CALIBRATE_AIR);
}

int SVMULTI::calibrationWater(){
  return setReg(REG_CALIBRATE_WATER);
}
int SVMULTI::newAddress(byte newAddr){
  if(setReg8(REG_SET_I2C_ADDR, newAddr)){
    addr = newAddr;
  }
  else{
    return 0;
  }
  return 1;
}

int SVMULTI::newReading(){
  _wire->beginTransmission(addr); // transmit to device
  _wire->write(REG_READ_START);              // sends one byte
  _wire->endTransmission();    // stop transmitting
  delay(300);
  return getState();
}

void SVMULTI::getData(soil_ret_t *data){
  vals_t vals;
  getRaw(&vals);
  for(int a=0;a<MULTI_ZONES;a++){
		  data->dp[a] = vals.dp[a]/100.0;
		  data->vwc[a] = (4.3e-6 * pow(data->dp[a],3) - 5.5e-4*pow(data->dp[a],2) + 2.92e-2*data->dp[a] - 5.3e-2)* 100.0;
	}
	for(int a=0;a<MULTI_TEMPS;a++){
		  data->temp[a] = vals.temp[a]/100.0;
	}

}


void SVMULTI::getRaw(vals_t *vals){
  _wire->beginTransmission(addr); // transmit to device
  _wire->write(REG_GET_DATA);              // sends one byte
  _wire->endTransmission();    // stop transmitting
  _wire->requestFrom(addr, (uint8_t)14);
   byte *p1 = (byte*)vals;
  if(i2cdelay(14)){
    // Wire.readBytes((byte*)&vals, 14);
	  for(int i = 0; i<14; i++){
		  p1[i] = _wire->read();
	  }
  }
}

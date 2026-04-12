/*
 * ======================================================================================================================
 *  dsmux.cpp - Dallas One Wire Mux Functions for DS2482S-800 8 Channel I2C to 1-Wire Bus Adapter
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/obs.h"
#include "include/output.h"
#include "include/dsmux.h"
#include "include/main.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
 Adafruit_DS248x ds248x;

 bool DSMUX_exists = false;
 bool dsmux_sensor_exists[DS248X_CHANNELS];

 /*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * dsmux_readTemperature() - 
 *=======================================================================================================================
 */
float dsmux_readTemperature(uint8_t channel) {
  // Select the channel on the DS2482-800
  if (!ds248x.selectChannel(channel)) {
    // Handle error if channel selection fails
    Output("DSMUX:Select CH Err");
    return NAN; // Return 'Not a Number' to indicate an error
  }

  // Start temperature conversion
  ds248x.OneWireReset();
  ds248x.OneWireWriteByte(DS18B20_CMD_SKIP_ROM); // Skip ROM command
  ds248x.OneWireWriteByte(DS18B20_CMD_CONVERT_T); // Convert T command
  delay(750); // Wait for conversion (750ms for maximum precision)

  // Read scratchpad
  ds248x.OneWireReset();
  ds248x.OneWireWriteByte(DS18B20_CMD_SKIP_ROM); // Skip ROM command
  ds248x.OneWireWriteByte(DS18B20_CMD_READ_SCRATCHPAD); // Read Scratchpad command

  uint8_t data[9];
  for (int i = 0; i < 9; i++) {
    ds248x.OneWireReadByte(&data[i]);
  }

  // Calculate temperature
  int16_t raw = (data[1] << 8) | data[0];
  float celsius = (float)raw / 16.0;

  return (celsius);
}

/* 
 *=======================================================================================================================
 * dsmux_get_sensor_address() - 
 *=======================================================================================================================
 */
bool dsmux_get_sensor_address(int channel, uint8_t *addr) {
  // default to 0s
  for (int i = 0; i < 8; i++) {
    addr[i] = 0;
  }

  // select 1-Wire channel (0–7)
  if (!ds248x.selectChannel((uint8_t)channel)) {
    return (false);
  }

  // reset 1-Wire bus on that channel
  if (!ds248x.OneWireReset()) {
    return (false);
  }

  // reset search state
  if (!ds248x.OneWireSearchReset()) {
    return (false);
  }

  // search for first (and only) device on this channel
  if (!ds248x.OneWireSearch(addr)) {
    // no device found, addr already zeroed
    return (false);
  }

  // got one device address in addr[0..7]
  return (true);
}

/* 
 *=======================================================================================================================
 * dsmux_obs_do() - do obs for dallas temperature sensors
 *=======================================================================================================================
 */
void dsmux_obs_do(int &sidx) {
  if (DSMUX_exists) {
    for (int channel=0; channel<DS248X_CHANNELS; channel++) {
      if (dsmux_sensor_exists[channel]) {
        float t = dsmux_readTemperature(channel);
        t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

        sprintf (Buffer32Bytes, "dst%d", channel);
        strcpy (obs.sensor[sidx].id, Buffer32Bytes);
        obs.sensor[sidx].type = F_OBS;
        obs.sensor[sidx].f_obs = (float) t;
        obs.sensor[sidx++].inuse = true;
      }
    }
  }
}

/* 
 *=======================================================================================================================
 * dsmux_initialize() - detect ds mux if found look for sensors
 *=======================================================================================================================
 */
void dsmux_initialize() {
  Output("DSMUX:INIT");

  if (ds248x.begin(&Wire, DSMUX_ADDRESS)) {
    uint8_t addr[8];

    Output ("DSMUX Channel Scan");
    DSMUX_exists = true;
    int count=0;

    for (int channel=0; channel<DS248X_CHANNELS; channel++) {
      dsmux_sensor_exists[channel] = dsmux_get_sensor_address(channel, addr);

      if (dsmux_sensor_exists[channel]) {
        float t = dsmux_readTemperature(channel);

        sprintf (Buffer32Bytes, "  dst-%d=%d.%02d %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
          channel, (int)t, (int)(t*100)%100,
          addr[0],addr[1],addr[2],addr[3], addr[4],addr[5],addr[6],addr[7]);
        Output(Buffer32Bytes);  
        count++;
      }
    }
    sprintf (Buffer32Bytes, "DSMUX %d Found", count);
    Output(Buffer32Bytes);
  } 
  else {
    Output ("DSMUX NF");
    DSMUX_exists = false;
  }
}
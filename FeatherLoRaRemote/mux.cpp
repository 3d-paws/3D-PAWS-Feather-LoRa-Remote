/*
 * ======================================================================================================================
 *  mux.cpp - PCA9548 I2C MUX
 * ======================================================================================================================
 */
#include <Arduino.h>
#include <Wire.h>
#include <i2cArduino.h>
#include "include/qc.h"
#include "include/obs.h"
#include "include/sensors.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/mux.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
const char *sensor_state[] = {"OFFLINE", "ONLINE"};
bool MUX_exists = false;
MULTIPLEXER_STR mux[MUX_CHANNELS];
MULTIPLEXER_STR *mc;
CH_SENSOR *chs;
const char *sensor_type[] = {"UNKN", "bmp", "bme", "b38", "b39", "htu", "sht", "mcp", "hdc", "lps", "hih", "tlw", "tsm", "si"};

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * mux_deselect_all() - set mux channel
 *=======================================================================================================================
 */
void mux_deselect_all() {
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(0);
  Wire.endTransmission();  
}


/* 
 *=======================================================================================================================
 * mux_channel_set() - set mux channel
 *=======================================================================================================================
 */
void mux_channel_set(uint8_t channel) {
  if (channel >= MUX_CHANNELS) return;
/*
  sprintf (Buffer32Bytes, "MUX:CHANNEL:%d SET", channel);
  Output (Buffer32Bytes);
*/
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();  
}
  
/* 
 *=======================================================================================================================
 * mux_obs_do() - do obs for mux devices
 *=======================================================================================================================
 */
void mux_obs_do(int &sidx) {
  if (MUX_exists) {
    Output("MUX:OBSDO");
 
    for (int c=0; c<MUX_CHANNELS; c++) {
      if (mux[c].inuse) {
        mux_channel_set(c);
        for (int s = 0; s < MAX_CHANNEL_SENSORS; s++) {

          // Tinovi Soil Moisture
          if (mux[c].sensor[s].type == m_tsm) {
            tsm.newReading();
            delay(100);
            
            sprintf (obs.sensor[sidx].id, "tsme25-%d", mux[c].sensor[s].id);
            obs.sensor[sidx].type = F_OBS;
            obs.sensor[sidx].f_obs = tsm.getE25();
            obs.sensor[sidx++].inuse = true;

            sprintf (obs.sensor[sidx].id, "tsmec-%d", mux[c].sensor[s].id);
            obs.sensor[sidx].type = F_OBS;
            obs.sensor[sidx].f_obs = tsm.getEC();
            obs.sensor[sidx++].inuse = true;

            sprintf (obs.sensor[sidx].id, "tsmvwc-%d", mux[c].sensor[s].id);
            obs.sensor[sidx].type = F_OBS;
            obs.sensor[sidx].f_obs = tsm.getVWC();
            obs.sensor[sidx++].inuse = true;
            
            float t = tsm.getTemp();
            t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
            
            sprintf (obs.sensor[sidx].id, "tsmt-%d", mux[c].sensor[s].id);
            obs.sensor[sidx].type = F_OBS;
            obs.sensor[sidx].f_obs = t;
            obs.sensor[sidx++].inuse = true;
          } // Tinovi Soil Moisture
        } // for
      } // in use
    } // for channels
    mux_deselect_all(); // When done with MUX set to channel 0
  } // MUX_exists
  else {
    // No MUX so check main i2c bus for Sensor
    if (TSM_exists) {
      tsm.newReading();
      delay(100);

      sprintf (obs.sensor[sidx].id, "tsme25");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = tsm.getE25();
      obs.sensor[sidx++].inuse = true;

      sprintf (obs.sensor[sidx].id, "tsmec");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = tsm.getEC();
      obs.sensor[sidx++].inuse = true;

      sprintf (obs.sensor[sidx].id, "tsmvwc");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = tsm.getVWC();
      obs.sensor[sidx++].inuse = true;
            
      float t = tsm.getTemp();
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
            
      sprintf (obs.sensor[sidx].id, "tsmt");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = t;
      obs.sensor[sidx++].inuse = true;        
    } // TSM
  } // No MUX
}

/* 
 *=======================================================================================================================
 * mux_scan() - detect connected sensors
 *=======================================================================================================================
 */
void mux_scan() {
  if (MUX_exists) {
    Output("MUX:SCAN");
    for (int c=0; c<MUX_CHANNELS; c++) {
      mux_channel_set(c);
      int s = 0;

      // Test for Tinovi Soil Moisture sensor
      int tsm_id = 0;

      if (I2C_Device_Exist(TSM_ADDRESS)) {
        tsm.init(TSM_ADDRESS);
        mux[c].inuse = true;
        // Tinovi Soil Moisture
        mux[c].sensor[s].state = ONLINE;
        mux[c].sensor[s].type = m_tsm;
        mux[c].sensor[s].id = ++tsm_id; 
        mux[c].sensor[s].address = TSM_ADDRESS;

        sprintf (Buffer32Bytes, "  CH-%d.%d TSM OK", c, s);
        Output (Buffer32Bytes);
        s++;
      }
      else {         
        sprintf (Buffer32Bytes, "  CH-%d TSM NF", c);
        Output (Buffer32Bytes);
      }

      // Test for next sensor type
      
    }
    mux_deselect_all();
  }
}

/* 
 *=======================================================================================================================
 * mux_initialize() - detect mux if found look for sensors
 *=======================================================================================================================
 */
void mux_initialize() {
  Output("MUX:INIT");

  Wire.beginTransmission(MUX_ADDR);
  if (Wire.endTransmission() == 0) {
    Output ("MUX OK");
    MUX_exists = true;

    for (int c=0; c<MUX_CHANNELS; c++) {
      mc = &mux[c];
      mc->inuse = false;
      for (int s=0; s<MAX_CHANNEL_SENSORS; s++) {
        chs = &mc->sensor[s];
        chs->state = OFFLINE;
        chs->type = UNKN;
        chs->address = 0x00;
        chs->id = 0;
      }
    }
    mux_scan(); // Scan channels 0-7 for i2c sensors.
  } 
  else {
    Output ("MUX NF");
    MUX_exists = false;
  }
}

/*
 * ======================================================================================================================
 *  obs.cpp - Observation Handeling
 *  
 *  NCSLR,[3],[10],{"at":"2025-09-25T16:53:58","id":XXX,"devid":"330eff6367815b7d93bfbcec","mtype":"OBS",}
 *                 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
 *                          1         2         3         4         5         6         7         8         9
 *  
 *  LoRa Header and Tail NCSLR,[len 3],[len10], == 12 Bytes
 *  OBS HEADER + {} = 97
 *  Overhead = 110
 *  LoRa Payload 222 bytes when using typical settings like Spreading Factor 7 and 125 kHz bandwidth
 *  RadioHead RF95 library is typically using Spreading Factor = 7.
 * ======================================================================================================================
 */
//#include <Arduino.h>
//#include <SD.h>
//#include <RTClib.h>
//#include <Adafruit_EEPROM_I2C.h>

#include "include/qc.h"
#include "include/ssbits.h"
#include "include/feather.h"
#include "include/mux.h"
#include "include/dsmux.h"
#include "include/sensors.h"
#include "include/sensors_i2c_44_47.h"
#include "include/eeprom.h"
#include "include/wrda.h"
#include "include/cf.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/lora.h"
#include "include/support.h"
#include "include/gps.h"
#include "include/time.h"
#include "include/main.h"
#include "include/obs.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
OBSERVATION_STR obs;
float bmx_1_pressure = 0.0;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * OBS_Clear() - Set OBS to not in use
 * ======================================================================================================================
 */
void OBS_Clear() {
  obs.inuse =false;
  for (int s=0; s<MAX_SENSORS; s++) {
    obs.sensor[s].inuse = false;
  }
}

/*
 * ======================================================================================================================
 * OBS_Send() - From obs structure build a JSON and send 1 or more LoRa packets as needed
 * ======================================================================================================================
 */
void OBS_Send() {
  char header[128];
  char sensors[128];
  char sensor[16];
  char loramsg[256];
  char obslog[1024];   // Holds JSON observations to write to log
   
  Output("OBS_SEND()");
    
  if (obs.inuse) {     // Sanity check set by OBS_Take()
    memset(header, 0, sizeof(header));
    memset(sensors, 0, sizeof(sensors));
    memset(obslog, 0, sizeof(obslog));
    memset(loramsg, 0, sizeof(loramsg));

    // Observation will be in JSON format
    // OBS_Take() has already obtained the time

    sprintf (header, "\"at\":\"%s\",\"id\":%d,\"devid\":\"%s\",\"mtype\":\"OBS\"", 
      timestamp, cf_lora_unitid, DeviceID);

    sprintf (obslog, "{%s", header);
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        // sprintf (Buffer32Bytes, "PROCESSOBS=%d", s);
        // Output(Buffer32Bytes);   
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (sensor, ",\"%s\":%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (sensor, ",\"%s\":%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (sensor, ",\"%s\":%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output ("WhyAmIHere?");
            break;
        }
        
        // Add the sensor to the obs log that we will later save to SD card
        sprintf (obslog+strlen(obslog), "%s", sensor);
        
        // Will this Sensor observation fit into sensors
        // OBS_SPACE is 112 bytes. This is how we do not overflow the 256 byte loramsg structure, sensors structure is 128 bytes
        if ( (strlen(sensor) + strlen(sensors)) <= OBS_SPACE) {
          // Add the sensor to the sensors
          sprintf (sensors+strlen(sensors), "%s", sensor);
        }
        else {       
          // Put the parts together and send
          Output("OBS_SEND:SENDING");
          sprintf (loramsg, "{%s%s}", header, sensors);
          SendLoRaMessage(loramsg, "LR");
          delay(500); // Its Recommended before sending another message
          Output("OBS_SEND:SENT");

          // Clear sensors and add the one that would not fit
          memset(sensors, 0, sizeof(sensors));
          memset(loramsg, 0, sizeof(loramsg));
          sprintf (sensors, "%s", sensor);
        }
      }
      else {
        // sprintf (Buffer32Bytes, "NOT INUSE=%d", s);
        // Output(Buffer32Bytes);          
      }
    } // for

    // Send remainding obs if any
    if (strlen(sensors)) {
      Output("OBS_SEND:SENDING-LAST");
      sprintf (loramsg, "{%s%s}", header, sensors);
      SendLoRaMessage(loramsg, "LR");
    }
    
    // Close off the observation and save to SD card
    sprintf (obslog+strlen(obslog), "}"); 
    SD_LogObservation(obslog);
    OBS_Clear(); 
    
    Output("OBS_SEND:OK");
  }
  else {
    // Output("OBS_SEND:EMPTY");
  }
}

/*
 * ======================================================================================================================
 * OBS_Take() - Take Observations - Should be called once a minute - fill data structure
 * ======================================================================================================================
 */
void OBS_Take() {
  int sidx = 0;
  float rg1 = 0.0;
  float rg2 = 0.0;
  unsigned long rg1ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  unsigned long rg2ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  float ws = 0.0;
  int wd = 0;
  float mcp3_temp = 0.0;  // globe temperature

  float heat_index = 0.0;
  float wetbulb_temp = 0.0;

  rtc_timestamp(); // Set now and timestamp struture with current time
  sprintf (Buffer32Bytes, "OBS_TAKE(%s)", timestamp);
  Output (Buffer32Bytes);
  
  // Safty Check for Vaild Time
  if (!RTC_valid) {
    Output ("OBS_Take: TM Invalid");
    return;
  }
  
  OBS_Clear(); // Just do it again as a safty check

  obs.inuse = true;
  
  // now = rtc.now(); // not needed.
  obs.ts = now.unixtime();

  strcpy (obs.sensor[sidx].id, "bv");
  obs.sensor[sidx].type = F_OBS;
  obs.sensor[sidx].f_obs = vbat_get();
  obs.sensor[sidx++].inuse = true;

  strcpy (obs.sensor[sidx].id, "hth");
  obs.sensor[sidx].type = I_OBS;
  obs.sensor[sidx].f_obs = SystemStatusBits;
  obs.sensor[sidx++].inuse = true;

  if (gps_exists) {
    strcpy (obs.sensor[sidx].id, "gon");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].f_obs = (gps_on) ? 1 : 0;
    obs.sensor[sidx++].inuse = true;
  }

  // Rain Gauge 1 - Each tip is 0.2mm of rain
  if (cf_rg1_enable) {
    rg1 = raingauge1_sample();
  }

  // Rain Gauge 2 - Each tip is 0.2mm of rain
  if (cf_op1 == OP1_STATE_RAIN) {
    rg2 = raingauge2_sample();
  }

  if (RainEnabled()) {
    if (eeprom_exists && eeprom_valid) {
      EEPROM_UpdateRainTotals(rg1, rg2);
    }
  }

  // Rain Gauge 1
  if (cf_rg1_enable) {
    strcpy (obs.sensor[sidx].id, "rg1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = rg1;
    obs.sensor[sidx++].inuse = true;

    if (eeprom_exists && eeprom_valid) {
      strcpy (obs.sensor[sidx].id, "rgt1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = eeprom.rgt1;
      obs.sensor[sidx++].inuse = true;

      strcpy (obs.sensor[sidx].id, "rgp1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = eeprom.rgp1;
      obs.sensor[sidx++].inuse = true;
    }
  }

  // Rain Gauge 2
  if (cf_op1 == OP1_STATE_RAIN) {
    strcpy (obs.sensor[sidx].id, "rg2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = rg2;
    obs.sensor[sidx++].inuse = true;

    if (eeprom_exists && eeprom_valid) {
      strcpy (obs.sensor[sidx].id, "rgt2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = eeprom.rgt2;
      obs.sensor[sidx++].inuse = true;

      strcpy (obs.sensor[sidx].id, "rgp2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = eeprom.rgp2;
      obs.sensor[sidx++].inuse = true;
    }
  }

  if (cf_op1 == OP1_STATE_RAW) {
    // OP1 Raw
    strcpy (obs.sensor[sidx].id, "op1r");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = Pin_ReadAvg(OP1_PIN);
    obs.sensor[sidx++].inuse = true;
  } 

  if ((cf_op1 == OP1_STATE_DIST_5M) || (cf_op1 == OP1_STATE_DIST_10M)) {
    float ds_median, ds_median_raw;
    
    ds_median = ds_median_raw = DS_Median();
    if (cf_ds_baseline > 0) {
      ds_median = cf_ds_baseline - ds_median_raw;
    }

    strcpy (obs.sensor[sidx].id, "ds");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ds_median;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "dsr");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ds_median_raw;
    obs.sensor[sidx++].inuse = true;
  }

  if (cf_op2 == OP2_STATE_RAW) {
    // OP2 Raw
    strcpy (obs.sensor[sidx].id, "op2r");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = Pin_ReadAvg(OP2_PIN);
    obs.sensor[sidx++].inuse = true;
  }

  if (cf_op2 == OP2_STATE_VOLTAIC) {
    // OP2 Voltaic Battery Voltage
    float vbv = VoltaicVoltage(OP2_PIN);
    strcpy (obs.sensor[sidx].id, "vbv");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = vbv;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "vpc");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = VoltaicPercent(vbv);
    obs.sensor[sidx++].inuse = true;
  }

  if (cf_op3 == OP3_STATE_RAW) {
    // OP3 Raw
    strcpy (obs.sensor[sidx].id, "op3r");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = Pin_ReadAvg(OP3_PIN);
    obs.sensor[sidx++].inuse = true;
  }

  if (cf_op4 == OP4_STATE_RAW) {
    // OP3 Raw
    strcpy (obs.sensor[sidx].id, "op4r");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = Pin_ReadAvg(OP4_PIN);
    obs.sensor[sidx++].inuse = true;
  }

  if (!cf_nowind) {
    // Wind Speed
    ws = Wind_SpeedAverage();
    ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
    strcpy (obs.sensor[sidx].id, "ws");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ws;
    obs.sensor[sidx++].inuse = true;

    // Wind Direction
    wd = Wind_DirectionVector();
    wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
    strcpy (obs.sensor[sidx].id, "wd");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = Wind_DirectionVector();
    obs.sensor[sidx++].inuse = true;

    // Wind Gust
    ws = Wind_Gust();
    ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
    strcpy (obs.sensor[sidx].id, "wg");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ws;
    obs.sensor[sidx++].inuse = true;

    // Wind Gust Direction (Global)
    wd = Wind_GustDirection();
    wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
    strcpy (obs.sensor[sidx].id, "wgd");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = wd;
    obs.sensor[sidx++].inuse = true;
  }
 
  if (BMX_1_exists) {
    float p,t,h;
    bmx1_read(p, t, h);
    
    // BMX1 Preasure
    strcpy (obs.sensor[sidx].id, "bp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = p;
    obs.sensor[sidx++].inuse = true;

    // BMX1 Temperature
    strcpy (obs.sensor[sidx].id, "bt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // BMX1 Humidity
    if (BMX_1_type == BMX_TYPE_BME280) {
      strcpy (obs.sensor[sidx].id, "bh1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = h;
      obs.sensor[sidx++].inuse = true;
    }

    bmx_1_pressure = p; // Used later for mslp calc
  }
  
  if (BMX_2_exists) {
    float p,t,h;
    bmx2_read(p, t, h);

    // BMX2 Preasure
    strcpy (obs.sensor[sidx].id, "bp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = p;
    obs.sensor[sidx++].inuse = true;

    // BMX2 Temperature
    strcpy (obs.sensor[sidx].id, "bt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // BMX2 Humidity
    if (BMX_2_type == BMX_TYPE_BME280) {
      strcpy (obs.sensor[sidx].id, "bh2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = h;
      obs.sensor[sidx++].inuse = true;
    }
  }

  // Do Sensor observations for SHT31, SHT45, BMP581, HDC302x
  sensor_i2c_44_47_obs_do(sidx);      
  
  if (HTU21DF_exists) {
    float t = 0.0;
    float h = 0.0;
    
    // HTU Humidity
    strcpy (obs.sensor[sidx].id, "hh1");
    obs.sensor[sidx].type = F_OBS;
    h = htu.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;

    // HTU Temperature
    strcpy (obs.sensor[sidx].id, "ht1");
    obs.sensor[sidx].type = F_OBS;
    t = htu.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (HIH8_exists) {
    float t = 0.0;
    float h = 0.0;
    bool status = hih8_getTempHumid(&t, &h);
    if (!status) {
      t = -999.99;
      h = 0.0;
    }
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

    // HIH8 Temperature
    strcpy (obs.sensor[sidx].id, "ht2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // HIH8 Humidity
    strcpy (obs.sensor[sidx].id, "hh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }
  
  if (SI1145_exists) {
    float si_vis = uv.readVisible();
    float si_ir = uv.readIR();
    float si_uv = uv.readUV()/100.0;

    // Additional code to force sensor online if we are getting 0.0s back.
    if ( ((si_vis+si_ir+si_uv) == 0.0) && ((si_last_vis+si_last_ir+si_last_uv) != 0.0) ) {
      // Let Reset The SI1145 and try again
      Output ("SI RESET");
      if (uv.begin()) {
        SI1145_exists = true;
        Output ("SI ONLINE");

        si_vis = uv.readVisible();
        si_ir = uv.readIR();
        si_uv = uv.readUV()/100.0;
      }
      else {
        SI1145_exists = false;
        Output ("SI OFFLINE");
      }
    }

    // Save current readings for next loop around compare
    si_last_vis = si_vis;
    si_last_ir = si_ir;
    si_last_uv = si_uv;

    // QC Checks
    si_vis = (isnan(si_vis) || (si_vis < QC_MIN_VI)  || (si_vis > QC_MAX_VI)) ? QC_ERR_VI  : si_vis;
    si_ir  = (isnan(si_ir)  || (si_ir  < QC_MIN_IR)  || (si_ir  > QC_MAX_IR)) ? QC_ERR_IR  : si_ir;
    si_uv  = (isnan(si_uv)  || (si_uv  < QC_MIN_UV)  || (si_uv  > QC_MAX_UV)) ? QC_ERR_UV  : si_uv;

    // SI Visible
    strcpy (obs.sensor[sidx].id, "sv1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = si_vis;
    obs.sensor[sidx++].inuse = true;

    // SI IR
    strcpy (obs.sensor[sidx].id, "si1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = si_ir;
    obs.sensor[sidx++].inuse = true;

    // SI UV
    strcpy (obs.sensor[sidx].id, "su1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = si_uv;
    obs.sensor[sidx++].inuse = true;
  }
    
  if (MCP_1_exists) {
    float t = 0.0;
   
    // MCP1 Temperature
    strcpy (obs.sensor[sidx].id, "mt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp1.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_2_exists) {
    float t = 0.0;
    
    // MCP2 Temperature
    strcpy (obs.sensor[sidx].id, "mt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp2.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_3_exists) {
    float t = 0.0;

    // MCP3 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp3.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    mcp3_temp = t; // globe temperature
  }

  if (MCP_4_exists) {
    float t = 0.0;

    // MCP4 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp4.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }
  
  if (PM25AQI_exists) {
    // Atmospheric Environmental PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e10;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e25;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_e100;
    obs.sensor[sidx++].inuse = true;

    // Clear readings
    pm25aqi_clear();
  }
  
  // Heat Index Temperature
  if (HI_exists) {
    heat_index = hi_calculate(sht1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "hi");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) heat_index;
    obs.sensor[sidx++].inuse = true;    
  }
  
  // Wet Bulb Temperature
  if (WBT_exists) {
    wetbulb_temp = wbt_calculate(sht1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "wbt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wetbulb_temp;
    obs.sensor[sidx++].inuse = true;    
  }

  // Wet Bulb Globe Temperature
  if (WBGT_exists) {
    float wbgt = 0.0;
    if (MCP_3_exists) {
      wbgt = wbgt_using_wbt(sht1_temp, mcp3_temp, wetbulb_temp); // TempAir, TempGlobe, TempWetBulb
    }
    else {
      wbgt = wbgt_using_hi(heat_index);
    }
    strcpy (obs.sensor[sidx].id, "wbgt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbgt;
    obs.sensor[sidx++].inuse = true;    
  }

  if (MSLP_exists) {
    float mslp = (float) mslp_calculate(sht1_temp, sht1_humid, bmx_1_pressure, cf_elevation);
    strcpy (obs.sensor[sidx].id, "mslp");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) mslp;
    obs.sensor[sidx++].inuse = true;  
  }

  // Tinovi Leaf Wetness
  if (TLW_exists) {
    tlw.newReading();
    delay(100);
    float w = tlw.getWet();
    float t = tlw.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs.sensor[sidx].id, "tlww");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) w;
    obs.sensor[sidx++].inuse = true; 

    strcpy (obs.sensor[sidx].id, "tlwt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }

  // Tinovi Soil Moisture
  if (TSM_exists) {
    tsm.newReading();
    delay(100);
    float e25 = tsm.getE25();
    float ec = tsm.getEC();
    float vwc = tsm.getVWC();
    float t = tsm.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs.sensor[sidx].id, "tsme25");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) e25;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tsmec");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) ec;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tsmvwc");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) vwc;
    obs.sensor[sidx++].inuse = true; 

    strcpy (obs.sensor[sidx].id, "tsmt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }

  // Tinovi Soil Moisture
  mux_obs_do(sidx);

  // Dallas Sensors Temperature on mux
  dsmux_obs_do(sidx);
  
  Output("OBS_TAKE(DONE)");
}

/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing
 * ======================================================================================================================
 */
void OBS_Do() {
  Output("OBS_DO()");
  
  Do_WRDA_Samples();    // Do Wind, Distance and Air Quality 1 minute of 1 second samples
  
  OBS_Take();          // Take an observation
  OBS_Send();          // From obs structure build JSON and send
}

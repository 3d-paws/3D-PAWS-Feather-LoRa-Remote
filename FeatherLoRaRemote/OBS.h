/*
 * ======================================================================================================================
 *  OBS.h - Observation Handeling
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

void mux_obs_do(int &sidx); // Prototype this function to aviod compile function unknown issue.

#define OBSERVATION_INTERVAL 60   // Seconds
#define MAX_SENSORS          64
#define LORA_PAYLOAD         222
#define OBS_HEADER           110
#define OBS_SPACE            112


typedef enum {
  F_OBS, 
  I_OBS, 
  U_OBS
} OBS_TYPE;

typedef struct {
  char          id[12];
  int           type;
  float         f_obs;
  int           i_obs;
  unsigned long u_obs;
  bool          inuse;
} SENSOR;

typedef struct {
  bool            inuse;                // Set to true when an observation is stored here         
  time_t          ts;                   // TimeStamp
  SENSOR          sensor[MAX_SENSORS];
} OBSERVATION_STR;

OBSERVATION_STR obs;

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

    sprintf (obslog, "%s", header);
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
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
        
        sprintf (obslog+strlen(obslog), "%s", sensor);
        
        // Will this Sensor observation fit into sensors
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

          // Clear sensors and add the one that would not fit
          memset(sensors, 0, sizeof(sensors));
          memset(loramsg, 0, sizeof(loramsg));
          sprintf (sensors+strlen(sensors), "%s", sensor);
        }
      }
    } // for

    // Send remainding obs if any
    if (strlen(sensors)) {
      Output("OBS_SEND:SENDING-LAST");
      sprintf (loramsg, "{%s%s}", header, sensors);
      SendLoRaMessage(loramsg, "LR");
    }
    
    sprintf (obslog+strlen(obslog), "}"); 
    SD_LogObservation(obslog);
    OBS_Clear(); 
    
    Output("OBS_SEND:OK");
  }
  else {
    Output("OBS_SEND:EMPTY");
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
  float sht1_humid = 0.0;
  float sht1_temp = 0.0;
  float heat_index = 0.0;
  float wetbulb_temp = 0.0;

  Output("OBS_TAKE()");
  
  // Safty Check for Vaild Time
  if (!RTC_valid) {
    Output ("OBS_Take: TM Invalid");
    return;
  }
  
  OBS_Clear(); // Just do it again as a safty check

  obs.inuse = true;
  rtc_timestamp(); // Set now and timestamp struture with current time
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

  // Rain Gauge 1 - Each tip is 0.2mm of rain
  if (cf_rg1_enable) {
    rg1ds = (millis()-raingauge1_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rg1 = raingauge1_interrupt_count * 0.2;
    raingauge1_interrupt_count = 0;
    raingauge1_interrupt_stime = millis();
    raingauge1_interrupt_ltime = 0; // used to debounce the tip
    // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
    rg1 = (isnan(rg1) || (rg1 < QC_MIN_RG) || (rg1 > (((float)rg1ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg1;
  }

  // Rain Gauge 2 - Each tip is 0.2mm of rain
  if (cf_rg2_enable) {
    rg2ds = (millis()-raingauge2_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rg2 = raingauge2_interrupt_count * 0.2;
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = millis();
    raingauge2_interrupt_ltime = 0; // used to debounce the tip
    // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
    rg2 = (isnan(rg2) || (rg2 < QC_MIN_RG) || (rg2 > (((float)rg2ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg2;
  }

  if (cf_rg1_enable || cf_rg2_enable) {
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
  if (cf_rg2_enable) {
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

  if (cf_ds_enable) {
    float dg_median, dg_median_raw;
    
    dg_median = dg_median_raw = DistanceGauge_Median();
    if (cf_ds_baseline > 0) {
      dg_median = cf_ds_baseline - dg_median_raw;
    }

    strcpy (obs.sensor[sidx].id, "ds");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = dg_median;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "dsr");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = dg_median_raw;
    obs.sensor[sidx++].inuse = true;
  }

  if (AS5600_exists) {
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

    Wind_ClearSampleCount(); // Clear Counter, Counter maintain how many samples since last obs sent
  }
  
  //
  // Add I2C Sensors
  //
  if (BMX_1_exists) {
    float p = 0.0;
    float t = 0.0;
    float h = 0.0;

    if (BMX_1_chip_id == BMP280_CHIP_ID) {
      p = bmp1.readPressure()/100.0F;       // bp1 hPa
      t = bmp1.readTemperature();           // bt1
    }
    else if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
      if (BMX_1_type == BMX_TYPE_BME280) {
        p = bme1.readPressure()/100.0F;     // bp1 hPa
        t = bme1.readTemperature();         // bt1
        h = bme1.readHumidity();            // bh1 
      }
      if (BMX_1_type == BMX_TYPE_BMP390) {
        p = bm31.readPressure()/100.0F;     // bp1 hPa
        t = bm31.readTemperature();         // bt1 
      }    
    }
    else { // BMP388
      p = bm31.readPressure()/100.0F;       // bp1 hPa
      t = bm31.readTemperature();           // bt1
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    
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
  }
  
  if (BMX_2_exists) {
    float p = 0.0;
    float t = 0.0;
    float h = 0.0;

    if (BMX_2_chip_id == BMP280_CHIP_ID) {
      p = bmp2.readPressure()/100.0F;       // bp2 hPa
      t = bmp2.readTemperature();           // bt2
    }
    else if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
      if (BMX_2_type == BMX_TYPE_BME280) {
        p = bme2.readPressure()/100.0F;     // bp2 hPa
        t = bme2.readTemperature();         // bt2
        h = bme2.readHumidity();            // bh2 
      }
      if (BMX_2_type == BMX_TYPE_BMP390) {
        p = bm32.readPressure()/100.0F;     // bp2 hPa
        t = bm32.readTemperature();         // bt2       
      }
    }
    else { // BMP388
      p = bm32.readPressure()/100.0F;       // bp2 hPa
      t = bm32.readTemperature();           // bt2
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

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
  
  if (SHT_1_exists) {                                                                               
    float t = 0.0;
    float h = 0.0;

    // SHT1 Temperature
    strcpy (obs.sensor[sidx].id, "st1");
    obs.sensor[sidx].type = F_OBS;
    t = sht1.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    sht1_temp = t; // save for derived observations
    
    // SHT1 Humidity   
    strcpy (obs.sensor[sidx].id, "sh1");
    obs.sensor[sidx].type = F_OBS;
    h = sht1.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;

    sht1_humid = h; // save for derived observations
  }

  if (SHT_2_exists) {
    float t = 0.0;
    float h = 0.0;

    // SHT2 Temperature
    strcpy (obs.sensor[sidx].id, "st2");
    obs.sensor[sidx].type = F_OBS;
    t = sht2.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // SHT2 Humidity   
    strcpy (obs.sensor[sidx].id, "sh2");
    obs.sensor[sidx].type = F_OBS;
    h = sht2.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
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
        SystemStatusBits &= ~SSB_SI1145; // Turn Off Bit

        si_vis = uv.readVisible();
        si_ir = uv.readIR();
        si_uv = uv.readUV()/100.0;
      }
      else {
        SI1145_exists = false;
        Output ("SI OFFLINE");
        SystemStatusBits |= SSB_SI1145;  // Turn On Bit    
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
  
  if (VEML7700_exists) {
    float lux = veml.readLux(VEML_LUX_AUTO);
    lux = (isnan(lux) || (lux < QC_MIN_LX)  || (lux > QC_MAX_LX))  ? QC_ERR_LX  : lux;

    // VEML7700 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "lx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
    obs.sensor[sidx++].inuse = true;
  }
  
  if (PM25AQI_exists) {
    // Standard Particle PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s10;
    obs.sensor[sidx++].inuse = true;

    // Standard Particle PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s25;
    obs.sensor[sidx++].inuse = true;

    // Standard Particle PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1s100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.max_s100;
    obs.sensor[sidx++].inuse = true;

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

  // Tinovi Multi Level Soil Moisture
  if (TMSM_exists) {
    soil_ret_t multi;
    float t;

    tmsm.newReading();
    delay(100);
    tmsm.getData(&multi);

    strcpy (obs.sensor[sidx].id, "tmsms1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[0];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[1];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms3");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[2];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms4");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[3];
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "tmsms5");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) multi.vwc[4];
    obs.sensor[sidx++].inuse = true;
    
    t = multi.temp[0];
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    strcpy (obs.sensor[sidx].id, "tmsmt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    t = multi.temp[1];
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    strcpy (obs.sensor[sidx].id, "tmsmt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }

  // Tinovi Soil Moisture
  mux_obs_do(sidx);
  
  Output("OBS_TAKE(DONE)");
}

/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing
 * ======================================================================================================================
 */
void OBS_Do() {
  Output("OBS_DO()");
  
  I2C_Check_Sensors(); // Make sure Sensors are online

  Do_WRDA_Samples();    // Do Wind, Distance and Air Quality 1 minute of 1 second samples
  
  OBS_Take();          // Take an observation
  OBS_Send();          // From obs structure build JSON and send

}

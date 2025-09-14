/*
 * ======================================================================================================================
 *  OBS.h - Observation Handeling
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * OBS_Do() - Collect Observations, Build message, Send to logging site
 * ======================================================================================================================
 */
void OBS_Do (bool log_obs) {
  char obsbuf[256];
  float bmx1_pressure = 0.0;
  float bmx1_temp = 0.0;
  float bmx1_humid = 0.0;
  float bmx2_pressure = 0.0;
  float bmx2_temp = 0.0;
  float bmx2_humid = 0.0;
  float sht1_temp = 0.0;
  float sht1_humid = 0.0;
  float sht2_temp = 0.0;
  float sht2_humid = 0.0;
  float mcp1_temp = 0.0;
  float mcp2_temp = 0.0;
  float batt = 0.0;
  float rain = 0.0;
  float ds_median, ds_median_raw;
  unsigned long rgds;    // rain gauge delta seconds, seconds since last rain gauge observation logged
  bool SoilProbesExist = false;

  // Safty Check for Vaild Time
  if (!RTC_valid) {
    Output ("OBS_Do: Time NV");
    return;
  }

  Output ("OBS_Do()");

  if (!cf_rg_disable) {
    // Rain Gauge
    rgds = (millis()-rainguage_interrupt_stime)/1000;
    rain = rainguage_interrupt_count * 0.2;
    
    /*
    float maxrain = rain > ((rgds / 60) * QC_MAX_RG);
    sprintf (Buffer32Bytes, "RAIN [%d][%d.%d]RGDS[%d.%d]MR[%d.%d]",
      rainguage_interrupt_count, (int)rain, (int)(rain*100)%100,
      (int)rgds, (int)(rgds*100)%100,
      (int)maxrain, (int)(maxrain*100)%100);
    Output(Buffer32Bytes);
    */
    
    rain = ( isnan(rain) || (rain < QC_MIN_RG) || (rain > (((float)rgds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rain;

    rainguage_interrupt_count = 0;
    rainguage_interrupt_stime = millis();
    rainguage_interrupt_ltime = 0; // used to debounce the tip
  }

  if (ds_found[0] || ds_found[1]) {
    SoilProbesExist = true;
  }



  // Read Dallas Temp and Soil Moisture Probes
  if (SoilProbesExist) {
    DoSoilReadings();
    for (int probe=0; probe<NPROBES; probe++) {
      if (ds_found[probe]) {       
        sprintf (Buffer32Bytes, "S %d M%d T%d.%02d", 
          probe+1,  sm_reading[probe],
          (int)ds_reading[probe], (int)(ds_reading[probe]*100)%100);
        Output (Buffer32Bytes);
      }
    }
  }



  if (cf_ds_enable) {
    ds_median = ds_median_raw = DS_Median();
    if (cf_ds_baseline > 0) {
      ds_median = cf_ds_baseline - ds_median_raw;
    }
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
    bmx1_pressure = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    bmx1_temp     = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    bmx1_humid    = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
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
        p = bm32.readPressure()/100.0F;       // bp2 hPa
        t = bm32.readTemperature();           // bt2
      }      
    }
    else { // BMP388
      p = bm32.readPressure()/100.0F;       // bp2 hPa
      t = bm32.readTemperature();           // bt2
    }
    bmx2_pressure = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    bmx2_temp     = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    bmx2_humid    = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
  }

  if (SHT_1_exists) {                                                                               
    // SHT1 Temperature
    sht1_temp = sht1.readTemperature();
    sht1_temp = (isnan(sht1_temp) || (sht1_temp < QC_MIN_T)  || (sht1_temp > QC_MAX_T))  ? QC_ERR_T  : sht1_temp;
    
    // SHT1 Humidity   
    sht1_humid = sht1.readHumidity();
    sht1_humid = (isnan(sht1_humid) || (sht1_humid < QC_MIN_RH) || (sht1_humid > QC_MAX_RH)) ? QC_ERR_RH : sht1_humid;
  }

  if (SHT_2_exists) {
    // SHT2 Temperature
    sht2_temp = sht2.readTemperature();
    sht2_temp = (isnan(sht2_temp) || (sht2_temp < QC_MIN_T)  || (sht2_temp > QC_MAX_T))  ? QC_ERR_T  : sht2_temp;
    
    // SHT2 Humidity   
    sht2_humid = sht2.readHumidity();
    sht2_humid = (isnan(sht2_humid) || (sht2_humid < QC_MIN_RH) || (sht2_humid > QC_MAX_RH)) ? QC_ERR_RH : sht2_humid;
  }

  if (MCP_1_exists) {
    // MCP1 Temperature
    mcp1_temp = mcp1.readTempC();
    mcp1_temp = (isnan(mcp1_temp) || (mcp1_temp < QC_MIN_T)  || (mcp1_temp > QC_MAX_T))  ? QC_ERR_T  : mcp1_temp;
  }

  if (MCP_2_exists) {
    // MCP2 Temperature
    mcp2_temp = mcp2.readTempC();
    mcp2_temp = (isnan(mcp2_temp) || (mcp2_temp < QC_MIN_T)  || (mcp2_temp > QC_MAX_T))  ? QC_ERR_T  : mcp2_temp;
  }
  
  batt = vbat_get();

  // Set the time for this observation
  rtc_timestamp();
  if (log_obs) {
    Output(timestamp);
  }

  // Build JSON log entry by hand  
  // {"at":"2025-08-12T17:05:18","id":72,"devid":"330eff6367815b7d93bfbcec","rg":0.00,"sg":700.00,"sgr":700.00,"sm1":1,"st1":22.50,"bp1":849.0065,"bt1":23.29,"bh1":0.00,"bv":3.76,"hth":0}

  sprintf (obsbuf, "{\"at\":\"%s\",\"id\":%d,\"devid\":\"%s\",\"type\":\"OBS\",", 
    timestamp, cf_lora_unitid, DeviceID);

  if (!cf_rg_disable) {
    
    // sprintf (obsbuf+strlen(obsbuf), "\"rg\":%d.%02d,\"rgs\":%d,", 
    //   (int)rain, (int)((rain < 0 ? -rain : rain)*100)%100, rgds);

    sprintf (obsbuf+strlen(obsbuf), "\"rg\":%d.%02d,", 
      (int)rain, (int)((rain < 0 ? -rain : rain) * 100)%100 );

      //(int)rain, (int)(rain*100)%100);
  }
  
  if (cf_ds_enable) {
    sprintf (obsbuf+strlen(obsbuf), "\"sg\":%d.%02d,\"sgr\":%d.%02d,", 
      (int)ds_median, (int)(ds_median*100)%100,
      (int)ds_median_raw, (int)(ds_median_raw*100)%100);
  }
    
  if (SoilProbesExist) {
    for (int probe=0; probe<NPROBES; probe++) {
      if (ds_found[probe]) {
        sprintf (obsbuf+strlen(obsbuf), "\"sm%d\":%d,\"st%d\":%d.%02d,",  
          probe+1, sm_reading[probe],
          probe+1, (int)ds_reading[probe], abs((int)((ds_reading[probe]<0 ? -ds_reading[probe] : ds_reading[probe])*100)%100)); 
      }
    }
  }

  if (BMX_1_exists) {
    sprintf (obsbuf+strlen(obsbuf), "\"bp1\":%u.%04d,\"bt1\":%d.%02d,\"bh1\":%d.%02d,",
      (int)bmx1_pressure, (int)((bmx1_pressure < 0 ? -bmx1_pressure : bmx1_pressure)*100)%100,
      (int)bmx1_temp, abs((int)((bmx1_temp < 0 ? -bmx1_temp : bmx1_temp)*100)%100),
      (int)bmx1_humid, (int)((bmx1_humid < 0 ? -bmx1_humid : bmx1_humid)*100)%100);
  }
  if (BMX_2_exists) {
    sprintf (obsbuf+strlen(obsbuf), "\"bp2\":%u.%04d,\"bt2\":%d.%02d,\"bh2\":%d.%02d,",
      (int)bmx2_pressure, (int)((bmx2_pressure < 0 ? -bmx2_pressure : bmx2_pressure)*100)%100,
      (int)bmx2_temp, abs((int)((bmx2_temp < 0 ? -bmx2_temp : bmx2_temp)*100)%100),
      (int)bmx2_humid, (int)((bmx2_humid < 0 ? -bmx2_humid : bmx2_humid)*100)%100);
  }
  
  if (SHT_1_exists) { 
     sprintf (obsbuf+strlen(obsbuf), "\"st1\":%d.%02d,\"sh1\":%d.%02d,",
      (int)sht1_temp, abs((int)((sht1_temp < 0 ? -sht1_temp : sht1_temp)*100)%100),
      (int)sht1_humid, (int)((sht1_humid < 0 ? -sht1_humid : sht1_humid)*100)%100);   
  }
  if (SHT_2_exists) { 
    sprintf (obsbuf+strlen(obsbuf), "\"st2\":%d.%02d,\"sh2\":%d.%02d,",
      (int)sht2_temp, abs((int)((sht2_temp < 0 ? -sht2_temp : sht2_temp)*100)%100),
      (int)sht2_humid, (int)((sht2_humid < 0 ? -sht2_humid : sht2_humid)*100)%100);   
  }
  
  if (MCP_1_exists) {
    sprintf (obsbuf+strlen(obsbuf), "\"mt1\":%d.%02d,",
      (int)mcp1_temp, abs((int)((mcp1_temp < 0 ? -mcp1_temp : mcp1_temp)*100)%100));    
  }
  if (MCP_2_exists) {
    sprintf (obsbuf+strlen(obsbuf), "\"mt2\":%d.%02d,",
      (int)mcp1_temp, abs((int)((mcp2_temp < 0 ? -mcp2_temp : mcp2_temp)*100)%100));        
  }

  if (TLW_exists) {
    tlw.newReading();
    delay(100);
    float w = tlw.getWet();
    float t = tlw.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    sprintf (obsbuf+strlen(obsbuf), "\"tlww\":%d.%02d,\"tlwt\":%d.%02d,",
      (int)w, abs((int)((w < 0 ? -w : w)*100)%100), 
      (int)t, abs((int)((t < 0 ? -t : t)*100)%100));
  }

  if (TSM_exists) {
    tsm.newReading();
    delay(100);
    float e25 = tsm.getE25();
    float ec = tsm.getEC();
    float vwc = tsm.getVWC();
    float t = tsm.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    sprintf (obsbuf+strlen(obsbuf), "\"tsme25\":%d.%02d,\"tsmec\":%d.%02d,\"tsmvwc\":%d.%02d,\"tsmt\":%d.%02d,",
      (int)e25, (int)((e25 < 0 ? -e25 : e25)*100)%100, 
      (int)ec, (int)((ec < 0 ? -ec : ec)*100)%100,
      (int)vwc, (int)((vwc < 0 ? -vwc : vwc)*100)%100,
      (int)t, abs((int)((t < 0 ? -t : t)*100)%100));
  }

  if (TMSM_exists) {
    soil_ret_t multi;
    float t1,t2;

    tmsm.newReading();
    delay(100);
    tmsm.getData(&multi);
    t1 = multi.temp[0];
    t1 = (isnan(t1) || (t1 < QC_MIN_T)  || (t1 > QC_MAX_T))  ? QC_ERR_T  : t1;
    t2 = multi.temp[0];
    t2 = (isnan(t2) || (t2 < QC_MIN_T)  || (t2 > QC_MAX_T))  ? QC_ERR_T  : t2;  
      
    sprintf (obsbuf+strlen(obsbuf), "\"tmsms1\":%d.%02d,\"tmsms2\":%d.%02d,\"tmsms3\":%d.%02d,\"tmsms4\":%d.%02d,\"tmsmt1\":%d.%02d,\"tmsmt2\":%d.%02d,",
      (int)multi.vwc[0], (int)((multi.vwc[0] < 0 ? -multi.vwc[0] : multi.vwc[0])*100)%100,
      (int)multi.vwc[1], (int)((multi.vwc[1] < 0 ? -multi.vwc[1] : multi.vwc[1])*100)%100,
      (int)multi.vwc[2], (int)((multi.vwc[2] < 0 ? -multi.vwc[2] : multi.vwc[2])*100)%100,
      (int)multi.vwc[3], (int)((multi.vwc[3] < 0 ? -multi.vwc[3] : multi.vwc[3])*100)%100,
      (int)t1, abs((int)((t1 < 0 ? -t1 : t1)*100)%100),
      (int)t2, abs((int)((t2 < 0 ? -t2 : t2)*100)%100));
  } 
   
  sprintf (obsbuf+strlen(obsbuf), "\"bv\":%d.%02d,\"hth\":%d}", 
    (int)batt, (int)(batt*100)%100, SystemStatusBits);

  // Log Observation to SD Card
  if (log_obs) {
    SD_LogObservation(obsbuf);
  }

  SendOBSMessage(obsbuf);
}

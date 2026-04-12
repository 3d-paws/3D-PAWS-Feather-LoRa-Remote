/*
 * ======================================================================================================================
 * statmon.cpp - Station Monitor - When Jumper Set Main Loop runs StationMonitor()
 * ======================================================================================================================
 */
#include "include/ssbits.h"
#include "include/feather.h"
#include "include/mux.h"
#include "include/sensors.h"
#include "include/sensors_i2c_44_47.h"
#include "include/wrda.h"
#include "include/cf.h"
#include "include/output.h"
#include "include/support.h"
#include "include/time.h"
#include "include/main.h"
#include "include/statmon.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */


/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 * StationMonitor() - On OLED display station information
 * ======================================================================================================================
 */
void StationMonitor() {
  static int cycle = 0;
  static int count = 0;
  static int b = 0;
  static int p = 0;  // use to loop through each probe 0,1,2
  int r, c, len;
  
  OLED_ClearDisplayBuffer();
  
  float batt = vbat_get();

  // =================================================================
  // Line 0 of OLED   [Timestamp]
  // =================================================================
  rtc_timestamp();
  len = (strlen (timestamp) > 21) ? 21 : strlen (timestamp);
  for (c=0; c<=len; c++) oled_lines [0][c] = *(timestamp+c);
  Serial_write (timestamp);

  // =================================================================
  // Line 1 of OLED
  // =================================================================
  if ((cf_op1 == OP1_STATE_DIST_5M) || (cf_op1 == OP1_STATE_DIST_10M)) {
    int sg  = (int) analogRead(DISTANCE_GAUGE_PIN) * dg_resolution_adjust;  // cf_ds_enable = 0,5,10
    sprintf (msgbuf, "SG:%d %s", sg, (cf_op1 == OP1_STATE_DIST_10M) ? "10m" : "5m"); 
  }
  else {
    sprintf (msgbuf, "SG:NF");
  }
  len = (strlen (msgbuf) > 21) ? 21 : strlen (msgbuf);
  for (c=0; c<=len; c++) oled_lines [1][c] = *(msgbuf+c);
  Serial_write (msgbuf);
  
  // =================================================================
  // Line 2 of OLED
  // =================================================================
  if (cf_rg1_enable) {
    sprintf (msgbuf, "RG1:%d", (int) raingauge1_interrupt_count);
  }
  else {
    sprintf (msgbuf, "RG1:NF");
  }
  
  if (cf_op1 == OP1_STATE_RAIN) {
    sprintf (msgbuf+strlen(msgbuf), " RG2:%d", (int) raingauge2_interrupt_count);
  }
  else {
    sprintf (msgbuf+strlen(msgbuf), " RG2:NF");
  }
  len = (strlen (msgbuf) > 21) ? 21 : strlen (msgbuf);
  for (c=0; c<=len; c++) oled_lines [2][c] = *(msgbuf+c);
  Serial_write (msgbuf);

  // =================================================================
  // Line 3 of OLED Cycle between multiple sensors
  // =================================================================
  if (cycle == 0) {
    if (BMX_1_exists) {
      float p,t,h;
      bmx1_read(p, t, h);
      sprintf (msgbuf, "B1 %.2f %.2f %.2f", p,t,h);
    }
    else {
      sprintf (msgbuf, "B1 NF");
    }
  }
  
  if (cycle == 1) {
    if (BMX_2_exists) {
      float p,t,h;
      bmx2_read(p, t, h);
      sprintf (msgbuf, "B2 %.2f %.2f %.2f", p,t,h);
    }
    else {
      sprintf (msgbuf, "B2 NF");
    }
  }

  if (cycle == 2) {
    memset(msgbuf, 0, sizeof(msgbuf));
      
    if (MCP_1_exists) {
      float mcp_temp = mcp1.readTempC();   
      sprintf (msgbuf, "MCP1 T%.2f", mcp_temp);
    }
    else {
      sprintf (msgbuf, "MCP1 NF");
    }
  }

  if (cycle == 3) {
    if (MCP_2_exists) {
      float mcp_temp = mcp2.readTempC();   
      sprintf (msgbuf, "MCP2 T%.2f", mcp_temp);
    }
    else {
      sprintf (msgbuf, "MCP2 NF");
    }
  }

  if (cycle == 4) {
   sensor_i2c_44_47_statmon(0, Buffer32Bytes);
   sprintf (msgbuf, "%s", Buffer32Bytes);
  }
  if (cycle == 5) {
   sensor_i2c_44_47_statmon(1, Buffer32Bytes);
   sprintf (msgbuf, "%s", Buffer32Bytes);
  }
  if (cycle == 6) {
   sensor_i2c_44_47_statmon(2, Buffer32Bytes);
   sprintf (msgbuf, "%s", Buffer32Bytes);
  }
  if (cycle == 7) {
   sensor_i2c_44_47_statmon(3, Buffer32Bytes);
   sprintf (msgbuf, "%s", Buffer32Bytes);
  }
  if (cycle == 8) {
    sprintf (msgbuf, "BATT:%.2f HTH:%04X", 
    batt, SystemStatusBits); 
  }

  if (cycle == 9) {   
    if (HTU21DF_exists) {
      float htu_humid = htu.readHumidity();
      float htu_temp = htu.readTemperature();

      sprintf (msgbuf, "HTU H:%.2f T:%.2f", htu_humid, htu_temp);
    }
    else {
      sprintf (msgbuf, "HTU NF"); 
    } 
  }

  if (cycle == 10) {
    if (LPS_1_exists) {
      float t,p;
      t = lps1.readTemperature();
      p = lps1.readPressure();
      sprintf (msgbuf, "LPS1 P:%.2f T:%.2f", p, t);
    }
    else {
      sprintf (msgbuf, "LPS1 NF");
    }
  }

  if (cycle == 11) {
    if (LPS_2_exists) {
      float t,p;
      t = lps2.readTemperature();
      p = lps2.readPressure();
      sprintf (msgbuf, "LPS2 P:%.2f T:%.2f", p, t);
    }
    else {
      sprintf (msgbuf, "LPS2 NF");
    }
  }

  if (cycle == 12) {   
    if (HIH8_exists) {
      float t = 0.0;
      float h = 0.0;
      bool status = hih8_getTempHumid(&t, &h);
      if (!status) {
        t = -999.99;
        h = 0.0;
      }
      sprintf (msgbuf, "HIH8 T%.2f H%.2f", t, h);
    }
    else {
      sprintf (msgbuf, "HIH8 NF");
    }
  }

  if (cycle == 13) {
    if (SI1145_exists) {
      float si_vis = uv.readVisible();
      float si_ir = uv.readIR();
      float si_uv = uv.readUV()/100.0;
      sprintf (msgbuf, "SI V%.2f I%.2f U%.2f", si_vis, si_ir, si_uv);
    }
    else {
      sprintf (msgbuf, "SI NF");
    }
  }

  if (cycle == 14) {
    if (BLX_exists) {
      float lux = blx_takereading ();
      sprintf (msgbuf, "BLX %.2f", lux);
    }
    else {
      sprintf (msgbuf, "BLX NF");
    }
  }

  if (cycle == 15) {
    if (PM25AQI_exists) {
      PM25_AQI_Data aqid;
      pmaq.read(&aqid);
      sprintf (msgbuf, "PM 10:%ld 25:%ld 100:%ld", 
        aqid.pm10_env,
        aqid.pm25_env,
        aqid.pm100_env);
    }
    else {
      sprintf (msgbuf, "PM NF");
    }
  }

  len = (strlen (msgbuf) > 21) ? 21 : strlen (msgbuf);
  for (c=0; c<=len; c++) oled_lines [3][c] = *(msgbuf+c);
  Serial_writeln (msgbuf);

  // Give the use some time to read line 3 before changing
  if (count++ >= 5) {
    cycle = ++cycle % 16; // << +1
    count = 0;
  }
  
  OLED_update();
}

  // Tinovi Mositure Sensor
  // tlw
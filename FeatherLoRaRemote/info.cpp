/*
 * ======================================================================================================================
 *  info.cpp - Station Information Functions
 * ======================================================================================================================
 */
//#include <Arduino.h>
//#include <SD.h>
//#include <RTClib.h>

#include "include/ssbits.h"
#include "include/feather.h"
#include "include/eeprom.h"
#include "include/gps.h"
#include "include/mux.h"
#include "include/dsmux.h"
#include "include/cf.h"
#include "include/sensors_i2c_44_47.h"
#include "include/sensors.h"
#include "include/wrda.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/lora.h"
#include "include/support.h"
#include "include/time.h"
#include "include/main.h"
#include "include/info.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures 
 * =======================================================================================================================
 */
char SD_INFO_FILE[] = "INFO.TXT";       // Store INFO information in this file. Every INFO call will overwrite content

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */
 
/*
 * ======================================================================================================================
 * INFO_Do() - Get and Send System Information
 * 
 * Message Type 1 - Information
 *   NCS    Length (N) and Checksum (CS)
 *   IF,    INFO Particle Message Type
 *   INT,   Station ID
 *   INT,   Transmit Counter
 *   JSON   Msg Battery and Status
 * =======================================================================================================================
 */
void INFO_Do()
{
  char header[128];
  char rest[128];
  char loramsg[256];
  char fullmsg[1024];   // Holds JSON observations to write to INFO.TXT
  const char *sensorcomma = "";
  const char *comma = "";
  int msgLength;
  unsigned short checksum;
  float batt;
  bool oktosend = false; 

  
  memset(header, 0, sizeof(header));
  memset(rest, 0, sizeof(rest));
  memset(loramsg, 0, sizeof(loramsg));
  memset(fullmsg, 0, sizeof(fullmsg));

  rtc_timestamp();
  
  // BUILD HEADER ======================================================================================

  sprintf (header, "\"at\":\"%s\",\"id\":%d,\"devid\":\"%s\",\"mtype\":\"IF\"",
    timestamp, cf_lora_unitid, DeviceID);

  sprintf (fullmsg, "{%s", header);


  // BUILD BASE INFO ===================================================================================

  // Battery Voltage and System Status
  batt = vbat_get();

  sprintf (rest, ",\"ver\":\"%s\",\"bv\":%.2f,\"hth\":%d,",
    versioninfo, batt, SystemStatusBits);

  sprintf (rest+strlen(rest), "\"obsi\":\"%dm\",\"obsti\":\"%dm\",\"t2nt\":\"%ds\",",
    cf_obs_period, cf_obs_period, seconds_to_next_obs());

  // Station Elevation
  sprintf (rest+strlen(rest), "\"elev\":%d,", cf_elevation);

  // Rain total rollover offset
  sprintf(rest+strlen(rest), "\"rtro\":\"%d:%02d\",", cf_rtro_hour, cf_rtro_minute);

  // LoRa
  if (LORA_exists) {
    sprintf (rest+strlen(rest), "\"lora\":\"%d,%d,%dMHz,OK\"", cf_lora_unitid, cf_lora_txpower, cf_lora_freq);  
  }
  else {
    sprintf (rest+strlen(rest), "\"lora\":\"%d,%d,%dMHz,NF\"", cf_lora_unitid, cf_lora_txpower, cf_lora_freq);
  }

  //================================
  // Put the parts together and send
  //================================
  
  // Grow our full message
  sprintf (fullmsg+strlen(fullmsg), "%s", rest);
  
  Output("IFDO:SENDING");
  sprintf (loramsg, "{%s%s}", header, rest);
  SendLoRaMessage(loramsg, "IF");
  delay(500); // Its Recommended before sending another message

  // SEND DEVS ======================================================================================
  
  // Clear buffers
  memset(loramsg, 0, sizeof(loramsg));
  memset(rest, 0, sizeof(rest));

  sprintf (rest, ",\"devs\":\"");

  comma = "";
  if (eeprom_exists) {
    sprintf (rest+strlen(rest), "%seeprom", comma);
    comma=",";    
  }
  if (MUX_exists) {
    sprintf (rest+strlen(rest), "%smux", comma);
    comma=",";    
  }
  if (DSMUX_exists) {
    sprintf (rest+strlen(rest), "%sdsmux", comma);
    comma=",";    
  }
  if (SD_exists) {
    sprintf (rest+strlen(rest), "%ssd", comma);
    comma=",";    
  }
  if (gps_exists) {
    sprintf (rest+strlen(rest), "%sgps", comma);
    comma=","; 
  }
  // End of Discovered Devices List
  sprintf (rest+strlen(rest), "\"");

  if (gps_exists) {
    // add detailed gps information
    if (gps_valid) {
      sprintf (rest+strlen(rest), ",\"gps\":{\"lat\":%f,\"lon\":%f,\"alt\":%f,\"sat\":%d,\"hdop\":%f,\"on\":%d}",
        gps_lat, gps_lon, gps_altm, gps_sat, gps_hdop, (gps_on)?1:0);
    }
  }

  //================================
  // Put the parts together and send
  //================================
  
  // Grow our full message
  sprintf (fullmsg+strlen(fullmsg), "%s", rest);

  Output("IFDO:SENDING");
  sprintf (loramsg, "{%s%s}", header, rest);
  SendLoRaMessage(loramsg, "IF");
  delay(500); // Its Recommended before sending another message

  // SEND SENSORS PART1 ======================================================================================
  
  // Clear buffers
  memset(loramsg, 0, sizeof(loramsg));
  memset(rest, 0, sizeof(rest));

  // SENSORS
  comma="";
  if (BMX_1_exists) {
    sprintf (rest+strlen(rest), "%sBMX1(%s)", comma, bmxtype[BMX_1_type]);
    comma=",";
  }
  if (BMX_2_exists) {
    sprintf (rest+strlen(rest), "%sBMX2(%s)", comma, bmxtype[BMX_2_type]);
    comma=",";
  }
  if (MCP_1_exists) {
    sprintf (rest+strlen(rest), "%sMCP1", comma);
    comma=",";
  }
  if (MCP_2_exists) {
    sprintf (rest+strlen(rest), "%sMCP2", comma);
    comma=",";
  }
  if (MCP_3_exists) {
    sprintf (rest+strlen(rest), "%sMCP3/gt1", comma);
    comma=",";
  }
  if (MCP_4_exists) {
    sprintf (rest+strlen(rest), "%sMCP4/gt2", comma);
    comma=",";
  }

  // Add 0x44-)x47 sensors to the list
  sensor_i2c_44_47_info(rest, 128, comma);

  if (LPS_1_exists) {
    sprintf (rest+strlen(rest), "%sLPS1", comma);
    comma=",";
  }
  if (LPS_2_exists) {
    sprintf (rest+strlen(rest), "%sLPS2", comma);
    comma=",";
  }
  if (HIH8_exists) {
    sprintf (rest+strlen(rest), "%sHIH8", comma);
    comma=",";
  }
  if (SI1145_exists) {
    sprintf (rest+strlen(rest), "%sSI", comma);
    comma=",";
  }
  if (BLX_exists) {
    sprintf (rest+strlen(rest), "%sBLX", comma);
    comma=",";
  }
  if (AS5600_exists) {
    sprintf (rest+strlen(rest), "%sAS5600", comma);
    comma=",";
    sprintf (rest+strlen(rest), "%sWS(%s)", comma, pinNames[ANEMOMETER_IRQ_PIN]);
  }

  //================================
  // Put the parts together and send
  //================================
  if (strlen(rest)) {
    // Grow our full message
    sprintf (fullmsg+strlen(fullmsg), "\"sensors\":\"%s", rest);
    if (strlen(comma) > 0) {
      sensorcomma=",";
    }
  
    Output("IFDO:SEND SENSORS");
    sprintf (loramsg, "{%s,\"sensors\":\"%s\"}", header, rest);
    SendLoRaMessage(loramsg, "IF");
    delay(500); // Its Recommended before sending another message

    // Clear buffers
    memset(loramsg, 0, sizeof(loramsg));
    memset(rest, 0, sizeof(rest));
  }

  // SEND SENSORS PART2 ======================================================================================
  
  // SENSORS  
  comma="";
  if (TLW_exists) {
    sprintf (rest+strlen(rest), "%sTLW", comma);
    comma=",";
  } 
  if (TSM_exists) {
    sprintf (rest+strlen(rest), "%sTSM", comma);
    comma=",";
  }
  if (HI_exists) {
    sprintf (rest+strlen(rest), "%sHI", comma);
    comma=",";
  }
  if (WBT_exists) {
    sprintf (rest+strlen(rest), "%sWBT", comma);
    comma=",";
  }
  if (WBGT_exists) {
    if (MCP_3_exists) {
      sprintf (rest+strlen(rest), "%sWBGT W/GLOBE", comma);
    }
    else {
      sprintf (rest+strlen(rest), "%sWBGT WO/GLOBE", comma);
    }
    comma=",";
  }
  if (PM25AQI_exists) {
    sprintf (rest+strlen(rest), "%sPM25AQ(%s)", comma, pinNames[PM25AQI_PIN]);
    comma=",";
  }
  if (cf_rg1_enable) {
    sprintf (rest+strlen(rest), "%sRG1(%s)", comma, pinNames[RAINGAUGE1_IRQ_PIN]); 
    comma=",";
  }
  if (cf_op1 == OP1_STATE_RAW) {
    sprintf (rest+strlen(rest), "%sOP1R(%s)", comma, pinNames[OP1_PIN]);
    comma=",";
  } 
  if (cf_op1 == OP1_STATE_RAIN) {
    sprintf (rest+strlen(rest), "%sRG2(%s)", comma, pinNames[RAINGAUGE2_IRQ_PIN]);
    comma=",";
  } 
  if (cf_op1 == OP1_STATE_DIST_5M) {
    sprintf (rest+strlen(rest), "%s5MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], cf_ds_baseline);
    comma=",";
  } 
  if (cf_op1 == OP1_STATE_DIST_10M) {
    sprintf (rest+strlen(rest), "%s10MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], cf_ds_baseline);
    comma=",";
  }
  if (cf_op2 == OP2_STATE_RAW) {
    sprintf (rest+strlen(rest), "%sOP2R(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  }
  if (cf_op2 == OP2_STATE_VOLTAIC) {
    sprintf (rest+strlen(rest), "%sVBV(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  }
  if (cf_op3 == OP3_STATE_RAW) {
    sprintf (rest+strlen(rest), "%sOP3R(%s)", comma, pinNames[OP3_PIN]);
    comma=",";
  }
  if (cf_op4 == OP4_STATE_RAW) {
    sprintf (rest+strlen(rest), "%sOP4R(%s)", comma, pinNames[OP4_PIN]);
    comma=",";
  }

  //================================
  // Put the parts together and send
  //================================

  if (strlen(rest)) {
    // Grow our full message
    sprintf (fullmsg+strlen(fullmsg), "%s%s", sensorcomma, rest);
    if (strlen(comma) > 0) {
      sensorcomma=",";
    }
  
    Output("IFDO:SEND SENSORS");
    sprintf (loramsg, "{%s,\"sensors\":\"%s\"}", header, rest);
    SendLoRaMessage(loramsg, "IF");
    delay(500); // Its Recommended before sending another message

    // Clear buffers
    memset(loramsg, 0, sizeof(loramsg));
    memset(rest, 0, sizeof(rest));
  }

  // SEND MUX SENSORS ======================================================================================
  
  // MUX SENSORS  
  comma="";
  
  if (MUX_exists) {
    for (int c=0; c<MUX_CHANNELS; c++) {
      if (mux[c].inuse) {
        for (int s = 0; s < MAX_CHANNEL_SENSORS; s++) {
          if (mux[c].sensor[s].type == m_tsm) {
            sprintf (rest+strlen(rest), "%sTSM%d(%d.%d)", comma, mux[c].sensor[s].id, c, s);
            comma=",";
          }
        }
      }
    }
    if (strlen(rest)) {
      Output("IFDO:SEND MUX SENSORS");
      // Grow our full message
      sprintf (fullmsg+strlen(fullmsg), "%s%s\"", sensorcomma, rest);
      Serial_writeln(fullmsg); 
  
      sprintf (loramsg, "{%s,\"sensors\":\"%s\"}", header, rest);
      SendLoRaMessage(loramsg, "IF");
    }
  }

  //================================
  // Put the parts together and send
  //================================
  
  // Adding closing }
  sprintf (fullmsg+strlen(fullmsg), "\"}");
  Serial_writeln(fullmsg); 

  // Update INFO.TXT file
  if (SD_exists) {
    LoRaDisableSPI(); // Disable LoRA SPI0 Chip Select

    File fp = SD.open(SD_INFO_FILE, FILE_WRITE | O_TRUNC); 
    if (fp) {
      fp.println(fullmsg);
      fp.close();
      SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
      Output ("INFO->SD OK");
      // Output ("INFO Logged to SD");
    }
    else {
      SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
      Output ("SD:Open(Info)ERR");
    }
  }
}

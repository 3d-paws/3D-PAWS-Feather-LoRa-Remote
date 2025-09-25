/*
 * ======================================================================================================================
 *  INFO.h - Report Information about the station at boot
 * ======================================================================================================================
 */

char SD_INFO_FILE[] = "INFO.TXT";       // Store INFO information in this file. Every INFO call will overwrite content
 
// Prototyping functions to aviod compile function unknown issue.
int seconds_to_next_obs();
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

  memset(header, 0, sizeof(header));
  memset(rest, 0, sizeof(rest));
  memset(loramsg, 0, sizeof(loramsg));
  memset(fullmsg, 0, sizeof(fullmsg));

  rtc_timestamp();
  
  // Battery Voltage and System Status
  batt = vbat_get();

  sprintf (header, "\"at\":\"%s\",\"id\":%d,\"devid\":\"%s\",\"mtype\":\"IF\"",
    timestamp, cf_lora_unitid, DeviceID);

  sprintf (fullmsg, "{%s", header);

  sprintf (rest, ",\"ver\":\"%s\",\"bv\":%d.%02d,\"hth\":%d,",
    VERSION_INFO, (int)batt, (int)(batt*100)%100, SystemStatusBits);

  sprintf (rest+strlen(rest), "\"obsi\":\"%dm\",\"obsti\":\"%dm\",\"t2nt\":\"%ds\",",
    cf_obs_period, cf_obs_period, seconds_to_next_obs());

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
  
  // Clear buffers
  memset(loramsg, 0, sizeof(loramsg));
  memset(rest, 0, sizeof(rest));

  comma="";
  sprintf (rest+strlen(rest), "\"devs\":\"");
  if (eeprom_exists) {
    sprintf (rest+strlen(rest), "%seeprom", comma);
    comma=",";    
  }
  if (MUX_exists) {
    sprintf (rest+strlen(rest), "%smux", comma);
    comma=",";    
  }
  if (SD_exists) {
    sprintf (rest+strlen(rest), "%ssd", comma);
    comma=",";    
  }
  // End of Discovered Devices List
  sprintf (rest+strlen(rest), "\""); 

  //================================
  // Put the parts together and send
  //================================
  
  // Grow our full message
  sprintf (fullmsg+strlen(fullmsg), "%s,\"sensors\":\"", rest);

  Output("IFDO:SENDING");
  sprintf (loramsg, "{%s%s}", header, rest);
  SendLoRaMessage(loramsg, "IF");
  delay(500); // Its Recommended before sending another message
  
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
  if (SHT_1_exists) {
    sprintf (rest+strlen(rest), "%sSHT1", comma);
    comma=",";
  }
  if (SHT_2_exists) {
    sprintf (rest+strlen(rest), "%sSHT2", comma);
    comma=",";
  }
  if (HDC_1_exists) {
    sprintf (rest+strlen(rest), "%sHDC1", comma);
    comma=",";
  }
  if (HDC_2_exists) {
    sprintf (rest+strlen(rest), "%sHDC2", comma);
    comma=",";
  }
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
  if (VEML7700_exists) {
    sprintf (rest+strlen(rest), "%sVEML", comma);
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
    sprintf (fullmsg+strlen(fullmsg), "\"%s", rest);
    sensorcomma=",";
  
    Output("IFDO:SEND SENSORS");
    sprintf (loramsg, "{%s,\"sensors\":\"%s\"}", header, rest);
    SendLoRaMessage(loramsg, "IF");
    delay(500); // Its Recommended before sending another message

    // Clear buffers
    memset(loramsg, 0, sizeof(loramsg));
    memset(rest, 0, sizeof(rest));
  }
  
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
  if (TMSM_exists) {
    sprintf (rest+strlen(rest), "%sTMSM", comma);
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
  if (cf_rg2_enable) {
    sprintf (rest+strlen(rest), "%sRG2(%s)", comma, pinNames[RAINGAUGE2_IRQ_PIN]);
    comma=",";
  } 
  if (cf_ds_enable) {
    sprintf (rest+strlen(rest), "%sDIST %s(%s)", comma, 
     (cf_ds_enable==5) ? "5M" : "10M", pinNames[DISTANCE_GAUGE_PIN]);
    comma=",";
  } 
  
  for (int probe=0; probe<NPROBES; probe++) {
    if (ds_found[probe]) {
      sprintf (rest+strlen(rest), "%sSM%d(%s),ST%d(%s)",  
        comma, probe, pinNames[sm_pn[probe]], probe, pinNames[st_pn[probe]]);
        comma=",";   
    }
  }

  //================================
  // Put the parts together and send
  //================================

  if (strlen(rest)) {
    // Grow our full message
    sprintf (fullmsg+strlen(fullmsg), "%s%s", sensorcomma, rest);
    sensorcomma = ",";
  
    Output("IFDO:SEND SENSORS");
    sprintf (loramsg, "{%s,\"sensors\":\"%s\"}", header, rest);
    SendLoRaMessage(loramsg, "IF");
    delay(500); // Its Recommended before sending another message

    // Clear buffers
    memset(loramsg, 0, sizeof(loramsg));
    memset(rest, 0, sizeof(rest));
  }
  
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
    Output("IFDO:SEND MUX SENSORS");
    sprintf (loramsg, "{%s,\"sensors\":\"%s\"}", header, rest);
    SendLoRaMessage(loramsg, "IF");
  }

  //================================
  // Put the parts together and send
  //================================
  
  // Grow our full message
  sprintf (fullmsg+strlen(fullmsg), "%s%s\"}", sensorcomma, rest);
  Serial_writeln(fullmsg); 

  // Update INFO.TXT file
  if (SD_exists) {
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

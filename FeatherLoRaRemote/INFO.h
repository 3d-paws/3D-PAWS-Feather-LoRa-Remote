/*
 * ======================================================================================================================
 *  INFO.h - Report Information about the station at boot
 * ======================================================================================================================
 */

 
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
  const char *comma = "";
  int msgLength;
  unsigned short checksum;
  float batt; 

  rtc_timestamp();
  
  // Battery Voltage and System Status
  batt = vbat_get();
  
  strcpy (msgbuf, "NCS");
  // Message type,
  sprintf (msgbuf+strlen(msgbuf), "IF,"); // IF Particle Message Type (INFO)
  
  // Station ID
  sprintf (msgbuf+strlen(msgbuf), "%d,", cf_lora_unitid);    // Must be unique if multiple are transmitting
  
  // Transmit Counter
  sprintf (msgbuf+strlen(msgbuf), "%d,", SendMsgCount++);

  sprintf (msgbuf+strlen(msgbuf), "{");

    sprintf (msgbuf+strlen(msgbuf), "{\"at\":\"%s\",\"id\":%d,\"devid\":\"%s\",",
      timestamp, cf_lora_unitid, DeviceID);

    sprintf (msgbuf+strlen(msgbuf), "\"ver\":\"%s\",\"bv\":%d.%02d,\"hth\":%d,",
      VERSION_INFO, (int)batt, (int)(batt*100)%100, SystemStatusBits);

    sprintf (msgbuf+strlen(msgbuf), "\"obsi\":\"%dm\",\"obsti\":\"%dm\",\"t2nt\":\"%ds\",",
      cf_obs_period, cf_obs_period, seconds_to_next_obs());

    sprintf (msgbuf+strlen(msgbuf), "\"sensors\":\"");
      if (BMX_1_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sBMX1(%s)", comma, bmxtype[BMX_1_type]);
        comma=",";
      }
      if (BMX_2_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sBMX2(%s)", comma, bmxtype[BMX_2_type]);
        comma=",";
      }
      if (MCP_1_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sMCP1", comma);
        comma=",";
      }
      if (MCP_2_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sMCP2", comma);
        comma=",";
      }
      if (SHT_1_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sSHT1", comma);
        comma=",";
      }
      if (SHT_2_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sSHT2", comma);
        comma=",";
      }
      if (TLW_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sTLW", comma);
        comma=",";
      }
      if (TSM_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sTSM", comma);
        comma=",";
      }
      if (TMSM_exists) {
        sprintf (msgbuf+strlen(msgbuf), "%sTMSM", comma);
        comma=",";
      } 
      if (!cf_rg_disable) {
        sprintf (msgbuf+strlen(msgbuf), "%sRG(%s)", comma, pinNames[RAIN_GAUGE_PIN]);
        comma=",";
      } 
      if (cf_ds_enable) {
        sprintf (msgbuf+strlen(msgbuf), "%sDS(%s)", comma, pinNames[DS_PIN]);
        comma=",";
      } 
           
      for (int probe=0; probe<NPROBES; probe++) {
        if (ds_found[probe]) {
          sprintf (msgbuf+strlen(msgbuf), "%sSM%d(%s),ST%d(%s)",  
            comma, probe, pinNames[sm_pn[probe]], probe, pinNames[st_pn[probe]]);
          comma=",";   
        }
      }
    sprintf (msgbuf+strlen(msgbuf), "\""); 
    
  sprintf (msgbuf+strlen(msgbuf), "}");

  
  msgLength = strlen(msgbuf);
  // Compute checksum
  checksum=0;
  for(int i=3;i<msgLength;i++) {
    checksum += msgbuf[i];
  }
  
  if (SerialConsoleEnabled) {
    Output(msgbuf);
  }

  msgbuf[0] = msgLength;
  msgbuf[1] = checksum >> 8;
  msgbuf[2] = checksum % 256;
    
  SendAESLoraWanMsg (128, msgbuf, msgLength);
}

/*
 * ======================================================================================================================
 * sdcard.cpp - SD Card
 * ======================================================================================================================
 */
#include <Arduino.h>
#include <RTClib.h>
#include <SD.h>

#include "include/ssbits.h"
#include "include/mux.h"
#include "include/eeprom.h"
#include "include/output.h"
#include "include/lora.h"
#include "include/time.h"
#include "include/sdcard.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
File SD_fp;
char SD_obsdir[] = "/OBS";  // Store our obs in this directory. At Power on, it is created if does not exist
bool SD_exists = false;     // Set to true if SD card found at boot
char SD_crt_file[] = "CRT.TXT";             // if file exists clear rain totals and delete file

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * SD_initialize()
 *=======================================================================================================================
 */
void SD_initialize() {
  if (!SD.begin(SD_ChipSelect)) {
    Output ("SD:NF");
    SystemStatusBits |= SSB_SD;
    delay (5000);
  }
  else {
    SD_exists = true;
    if (!SD.exists(SD_obsdir)) {
      if (SD.mkdir(SD_obsdir)) {
        Output ("SD:MKDIR OBS OK");
        Output ("SD:Online");
        SD_exists = true;
      }
      else {
        Output ("SD:MKDIR OBS ERR");
        Output ("SD:Offline");
        SystemStatusBits |= SSB_SD;  // Turn On Bit     
      } 
    }
    else {
      Output ("SD:Online");
      Output ("SD:OBS DIR Exists");
      SD_exists = true;
    }
  }
}

/* 
 *=======================================================================================================================
 * SD_LogObservation()
 *=======================================================================================================================
 */
void SD_LogObservation(char *observations) {
  char SD_logfile[24];
  File fp;

  if (!SD_exists) {
    return;
  }

  if (!RTC_valid) {
    return;
  }

  LoRaDisableSPI(); // Disable LoRA SPI0 Chip Select

  // Note: "now" is global and is set when ever timestampe() is called. Value last read from RTC.
  sprintf (SD_logfile, "%s/%4d%02d%02d.log", SD_obsdir, now.year(), now.month(), now.day());
  // Output (SD_logfile);
  
  fp = SD.open(SD_logfile, FILE_WRITE); 
  if (fp) {
    fp.println(observations);
    fp.close();
    SystemStatusBits &= ~SSB_SD;  // Turn Off Bit
    Output ("OBS Logged to SD");
  }
  else {
    SystemStatusBits |= SSB_SD;  // Turn On Bit - Note this will be reported on next observation
    Output ("OBS Open Log Err");
    // At thins point we could set SD_exists to false and/or set a status bit to report it
    // SD_initialize();  // Reports SD NOT Found. Library bug with SD
  }
}

/* 
 *=======================================================================================================================
 * SD_ClearRainTotals() -- If CRT.TXT exists on SD card clear rain totals - Checked at Boot
 *=======================================================================================================================
 */
void SD_ClearRainTotals() {
  if (RTC_valid && SD_exists) {
    if (SD.exists(SD_crt_file)) {
      if (SD.remove (SD_crt_file)) {
        Output ("CRT:OK-CLR");
        now = rtc.now();
        EEPROM_ClearRainTotals(now.unixtime());
        EEPROM_Dump();
      }
      else {
        Output ("CRT:ERR-RM");
      }
    }
    else {
      Output ("CRT:OK-NF");
    }
  }
  else {
    Output ("CRT:ERR-CLK");
  }
}

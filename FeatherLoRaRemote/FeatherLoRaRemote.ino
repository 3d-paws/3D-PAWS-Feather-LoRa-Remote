#define COPYRIGHT "Copyright [2025] [University Corporation for Atmospheric Research]"
/*
 *======================================================================================================================
 * FeatherLoRaRemote - 3D-PAWS-Feather-LoRaRemote
 *   Board Type : Adafruit Feather M0  #if defined(ADAFRUIT_FEATHER_M0)
 *   Description: Uses LoRa to report to Particle Full Stations for relay of observations 
 *   Author: Robert Bubon
 *   Date:   2025-01-15 RJB Renamed RS_LoRa_MO code base to LoRaRemoteM0
 *                          Reworked LoRa transmission format. Now sending JSON
 *                          Added INFO message at power on
 *                          Added cf_rg_disable to the config file for enable disable rain gauge option
 *                          Added support for Distance sensor on pin A4
 *           2025-01-23 RJB Added ds_basline to config file, If positive, distance = baseline - ds_median
 *                          Added support for observation intervals 5,6,10,15,20,30
 *                          Added support for Tinovi moisture sensors (Leaf, Soil, Multi Level Soil)                          
 *           2025-02-24 RJB Changes to Config File handing. Safty checks added.
 *           2025-03-09 RJB Replaced a direct Serial output in SDC.h SD_findKey() with Output() 
 *           2025-04-24 RJB Corrected bugs in negative temperature reporting
 *           2025-08-12 RJB Bug - Double incrementing SendMsgCount. Fixed in OBS.h
 *           2025-08-17 RJB added to INFO and OBS messages \"type\":\"IF\"  \"type\":\"OBS\"
 *           2025-09-11 RJB In OBS fixed casting bug on rain collection. Added (float)
 *                          (rain > (((float) rgds / 60) * QC_MAX_RG))
 *                          Fixed sending -999.-90 QC error values FIX (rain < 0 ? -rain : rain) added
 *           2025-09-16 RJB Added MUX support for Multiple Tinovi Capacitive Soil Moisture & Temperature sensors                         
 *                          Only wait for serial consile connection for 30 not 60 seconds. When jumper set
 *                          Only stay in station monitor for 5 minues not 30.
 *                          Adding Wind. If we detect the AS5600 Wind direction we will then wake up
 *                          take one minute of samples for wind, transmit and go back to sleep.
 *                          Added air quality, average of 10 1s samples. Must be on mux channel 7 with no other sensors.
 *           2025-09-20 RJB Added SDU support - Modified 
 *                          ~/Library/Arduino15/packages/adafruit/hardware/samd/1.7.16/libraries/SDU/src/SDU.cpp
 *                            #elif defined(ADAFRUIT_FEATHER_M0)
 *                            #include "boot/adafeatherM0.h"
 *                            Added adafeatherM0.h
 *           2025-09-23 RJB Fixed checksum error on LoRa INFO messages
 *                          Added the chunking of the observers into multiple lora packets
 *           2025-10-05 RJB Code clean up, True .h files with corresponding.cpp files.          
 *                          Added mux_deselect_all() and changed all mux_channel_set(0) to this
 *                          Added void LoRaDisableSPI() used in CF.cpp
 *                          Added LoRaSleep() use in loop()
 *           
 * Time Format: 2022:09:19:19:10:00  YYYY:MM:DD:HR:MN:SS
 * 
 * ======================================================================================================================
 * Normal loop operation
 * 
 * If a tip occurs we wake up from low power sleep and increment the tip count in the interrupt handler.
 * Upon wake up the code starts executing after the function "LowPower.sleep(GoToSleepTime);" in the loop() function.
 * The loop() function determines if the full sleep (15min) has transpired.
 *   If so,  an observation is made and transmitted. After, another 15min low sleep period occurs.
 *   If not, it calculates the amount of time left in the 15 minute sleep period.
 *      If this amount of time is 2s or less before next observation,it delays and avoids the sleep.
 *      Otherwise it enters low power sleep for the calculated amount of time.
 * As a result of the above, rain and all other observations have a granularity of 15 minutes
 * ======================================================================================================================
 *                    
 * SEE https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module
 * SEE https://learn.adafruit.com/assets/46254 for Pinouts      
 * SEE https://www.microchip.com/wwwproducts/en/ATsamd21g18
 * SEE https://learn.adafruit.com/adafruit-adalogger-featherwing/pinouts
 * SEE https://www.microchip.com/wwwproducts/en/MCP73831 - Battery Charger
 * 
 * Antenna Options - Wire 3in long for 915Mhz
 * https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/antenna-options
 * https://www.changpuak.ch/electronics/bi_quad_antenna_designer.php
 * 
 * Arduino Low Power
 * SEE: https://www.arduino.cc/en/Reference/ArduinoLowPower
 * SEE https://github.com/arduino-libraries/ArduinoLowPower
 *  
 * Arduino/libraries/AES-master/AES_config.h  Modified to fix below problem.
 *     Arduino/libraries/AES-master/AES_config.h:43:18: fatal error: pgmspace.h: No such file or directory
 *     #include <pgmspace.h>
 *                ^~~~~~~~~~~~
 * diff AES_config.h.org AES_config.h
 *     40c40
 *     <     #if (defined(__AVR__))
 *     ---
 *     >     #if (true || defined(__AVR__))
 *     41a42
 *     >  #define printf_P printf
 *
 * Library of Adafruit_BMP280_Library has been modified to enable Forced Mode
 *
 * ======================================================================================================================
 * Pin Definitions
 * 
 * Board Label   Arduino  Info & Usage                   Grove Shield Connector   
 * ======================================================================================================================
 * RST
 * 3V            3v3 Power
 * ARef
 * GND
 * A0            A0       Soil Moisture Sensor 1         Grove A0
 * A1            A1       Dallas Sensor 1wire 1          Grove A0
 * A2            A2       Interrupt For Anemometer       Grove A2
 * A3            A3       Interrupt For Rain Gauge 1     Grove A2
 * A4            A4       Interrupt For Rain Gauge 2     Grove A4
 * A5            A5       Distance Sensor                Grove A4
 * SCK           SCK      SPI0 Clock - SD/LoRa           Not on Grove
 * MOS           MOSI     Used by SD Card/LoRa           Not on Grove
 * MIS           MISO     Used by SDCard/LoRa            Not on Grove
 * RX0           D0                                      Grove UART
 * TX1           D1                                      Grove UART 
 * io1           DIO1     to D6 if LoRa WAN              Not on Grove (Particle Pin D9)
   
 * BAT           VBAT Power
 * En            Control - Connect to ground to disable the 3.3v regulator
 * USB           VBUS Power
 * 13            D13      LED                            Not on Grove 
 * 12            D12      Serial Console Enable          Not on Grove
 * 11            D11      Enable Sensors 2n2222/2N3904   Not on Grove
 * 10            D10      Used by SD Card as CS          Grove D4  (Particle Pin D5)
 * 9             D9/A7    Voltage Battery Pin            Grove D4  (Particle Pin D4)
 * 6             D6       to DIO1 if LoRa WAN            Grove D2  (Particle Pin D3)
 * 5             D5       Air Quality SET Pin            Grove D2  (Particle Pin D2)
 * SCL           D3       i2c Clock                      Grove I2C_1
 * SDA           D2       i2c Data                       Grove I2C_1
 * 
 * Not exposed on headers
 * D8 = LoRa NSS aka Chip Select CS
 * D4 = LoRa Reset
 * D3 = LoRa DIO
 * ======================================================================================================================
 * Normal main loop operation
 * 
 * If a tip occurs we wake up increment the tip count in the interrupt handler.
 * Upon wake up the code starts executing after the function "LowPower.sleep(GoToSleepTime);" in the main loop.
 * The main loop determines if the full sleep period (5,6,10,15,20,30 min) has transpired.
 *   If so,  an observation is made and transmitted. After another 15min sleep period occurs.
 *   If not, It calculates the amount of time left in the 15 minute sleep period.
 *      If This amount of time is 2s or less before next observation,it delays and avoids the sleep.
 *      Otherwise it enters ultra low power sleep for the calculated amount of time.
 * So rain observations have a granularity of 15 minutes
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * Add SDUBoot Support for UPDATE.BIN on SD card
 * ======================================================================================================================
 */
#include "boot/SDU.h"

/* 
 *=======================================================================================================================
 * Includes
 *=======================================================================================================================
 */
#include <ArduinoLowPower.h>
#include <SD.h>
#include <RTClib.h>

#include "include/ssbits.h"
#include "include/mux.h"
#include "include/qc.h"
#include "include/eeprom.h"
#include "include/obs.h"
#include "include/wrda.h"
#include "include/cf.h"
#include "include/sdcard.h"
#include "include/info.h"
#include "include/sensors.h"
#include "include/output.h"
#include "include/lora.h"
#include "include/statmon.h"
#include "include/smt.h"
#include "include/support.h"
#include "include/time.h"
#include "include/main.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures 
 * =======================================================================================================================
 */
bool JustPoweredOn = true;   // Used to clear SystemStatusBits set during power on device discovery
char msgbuf[MAX_MSGBUF_SIZE];// Buffer used all over the code
char *msgp;                  // Pointer used all over the code
char Buffer32Bytes[32];      // Buffer used all over the code
int countdown = 300;         // Exit calibration mode when reaches 0 - protects against burnt out pin or forgotten jumper
unsigned long nextinfo=0;    // Time of Next INFO transmit


void sleepinterrupt() {
  Output("I");
}

/*
 * =======================================================================================================================
 * setup()
 * =======================================================================================================================
 */
void setup() 
{
  // Put initialization like pinMode and begin functions here.
  pinMode (LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode (PM25AQI_PIN, OUTPUT); 
  digitalWrite(PM25AQI_PIN, HIGH);

  Output_Initialize();
  delay(2000); // Prevents usb driver crash on startup

  Serial_write(COPYRIGHT);
  Output (VERSION_INFO);

  GetDeviceID();
  sprintf (msgbuf, "DevID:%s", DeviceID);
  Output (msgbuf);
  
  delay (2000);      // Pause so user can see version if not waiting for serial

  // https://forums.adafruit.com/viewtopic.php?f=57&t=174492&p=850337&hilit=RFM95+adalogger#p850337
  // Normally, well-behaved libraries for SPI devices would take care to set CS high when inactive. 
  // But since the SD library does not initialize the radio, pin 8 is left floating.
  // When I need to use the radio again, should I set pin 8 to low?
  // No, the driver will handle it for you. You just have to make sure it is high when not in use.
  
  LoRaDisableSPI();

  // Initialize SD card if we have one.
  SD_initialize();

  if (SD_exists && SD.exists(CF_NAME)) {
    SD_ReadConfigFile();
  }
  else {
    sprintf(msgbuf, "CF:NO %s", CF_NAME); Output (msgbuf);
  } 

  // Read RTC and set system clock if RTC clock valid
  rtc_initialize();

  if (RTC_valid) {
    Output("RTC: Valid");
  }
  else {
    Output("RTC: Not Valid");
  }

  rtc_timestamp();
  sprintf (msgbuf, "%s", timestamp);
  Output(msgbuf);
  delay (2000);

  EEPROM_initialize();

  obs_interval_initialize();
  
  if (cf_rg1_enable) {
    // Optipolar Hall Effect Sensor SS451A - Rain1 Gauge
    raingauge1_interrupt_count = 0;
    raingauge1_interrupt_stime = millis();
    raingauge1_interrupt_ltime = 0;  // used to debounce the tip
    LowPower.attachInterruptWakeup(RAINGAUGE1_IRQ_PIN, raingauge1_interrupt_handler, FALLING);
    Output ("RG1:ENABLED");
  }
  else {
    Output ("RG1:NOT ENABLED");
  }

  // Optipolar Hall Effect Sensor SS451A - Rain2 Gauge
  if (cf_rg2_enable) {
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = millis();
    raingauge2_interrupt_ltime = 0;  // used to debounce the tip
    LowPower.attachInterruptWakeup(RAINGAUGE2_IRQ_PIN, raingauge2_interrupt_handler, FALLING);
    Output ("RG2:ENABLED");
  }
  else {
    Output ("RG2:NOT ENABLED");
  }

  //==================================================
  // Soil Moisture and Temperature Sensors
  //==================================================
  smt_initialize();
 
  //==================================================
  // Scan Mux Channels (0-6) for i2c Devices
  //==================================================
  mux_initialize();

  //==================================================
  // Scan Mux channel 7 for Air Quality
  // When AQ is powered down it holds onto the I2C bus
  //==================================================
  if (MUX_exists) {
    mux_channel_set(MUX_AQ_CHANNEL);
    pm25aqi_initialize(); 
    mux_deselect_all();   
  }
  
  if (!MUX_exists) {
    tsm_initialize(); // Check main bus
  }
  
  bmx_initialize();
  htu21d_initialize();  // This sensor has same i2c address as AS5600L
  mcp9808_initialize();
  sht_initialize();
  hih8_initialize();
  si1145_initialize();
  vlx_initialize();
  blx_initialize();
  as5600_initialize();
  hdc_initialize();
  lps_initialize();

  // Tinovi Mositure Sensors
  tlw_initialize();
  tmsm_initialize();

  // Derived Observations
  wbt_initialize();
  hi_initialize();
  wbgt_initialize();

  lora_initialize();

  // Set a time to force the first observation
  wakeuptime = now.unixtime();
}

/*
 * =======================================================================================================================
 * loop()
 * =======================================================================================================================
 */
void loop()
{
  static time_t sleep_time = 0;
  time_t time_asleep;
  int GoToSleepTime;
  
  // RTC not set, Get Time for User
  if (!RTC_valid) {
    static bool first = true;

    delay (1000);
      
    if (first) {
      if (digitalRead(SCE_PIN) != LOW) {
        Serial.begin(9600);
        SerialConsoleEnabled = true;
      }  
    
      Output("SET RTC ENTER:");
      Output("YYYY:MM:DD:HH:MM:SS");
      first = false;
    }
    
    if (rtc_readserial()) { // check for serial input, validate for rtc, set rtc, report result
      Output("!!!!!!!!!!!!!!!!!!!");
      Output("!!! Press Reset !!!");
      Output("!!!!!!!!!!!!!!!!!!!");

      while (true) {
        delay (1000);
      }
    }
  }

  //Calibration mode, You can also reset the RTC here
  else if (countdown && digitalRead(SCE_PIN) == LOW) { 
    StationMonitor();
    
    // check for input sting, validate for rtc, set rtc, report result
    if (Serial.available() > 0) {
      rtc_readserial(); // check for serial input, validate for rtc, set rtc, report result
    }
    
    countdown--;
    delay (1000);
  }

  // Normal Operation
  else {
    // Delayed initialization. We need a valid clock before we can validate the EEPROM
    if (eeprom_exists && !eeprom_valid) {
      EEPROM_Validate();
      EEPROM_Dump();
      SD_ClearRainTotals(); 
    }
    
    now = rtc.now(); 
        
    // Every 24 hours send INFO
    if (now.unixtime() > nextinfo) {      // Upon power on this will be true
      INFO_Do();
      nextinfo = now.unixtime() + (3600 * 24);      
    }

    if (now.unixtime() >= wakeuptime) {   // Upon power on this will be true
      I2C_Check_Sensors();
      OBS_Do();
      
      // Shutoff System Status Bits related to initialization after we have logged first observation
      JPO_ClearBits();
    }

    unsigned long stno = seconds_to_next_obs();
      
    if (stno <= 2) {
      // Avoid going to sleep if there is 2s or less time until we need to do an observation
      // This is really here to address going into low power move for a fraction of a second.
      Output("Delay - Not Sleep");
      delay (GoToSleepTime);
    }
    else {  

      sprintf (Buffer32Bytes, "Sleep for %us", stno);
      Output (Buffer32Bytes);  

      LoRaSleep();
    
      OLED_sleepDisplay();

      wakeuptime = stno + now.unixtime(); // "now" was updated in the seconds_to_next_obs() function
      LowPower.sleep(stno*1000); // uses milliseconds
 
      OLED_wakeDisplay();   // May need to toggle the Display reset pin.
      delay(2000);
      OLED_ClearDisplayBuffer(); 
      Output("Wakeup");
    }
  }

}

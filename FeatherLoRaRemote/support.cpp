/*
 * ======================================================================================================================
 *  support.cpp - Support Functions
 * ======================================================================================================================
 */
#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

#include "include/cf.h"
#include "include/output.h"
#include "include/sensors.h"
#include "include/wrda.h"
#include "include/time.h"
#include "include/main.h"
#include "include/support.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
int  LED_PIN = LED_BUILTIN;  // Built in LED
const char* pinNames[] = {
  "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
  "D8", "D9", "D10", "D11", "D12", "D13",
  "A0", "A1", "A2", "A3", "A4", "A5"
};
char DeviceID[25];           // A generated ID based on board's 128-bit serial number converted down to 96bits

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 *=======================================================================================================================
 * vbat_get() -- return battery voltage
 *=======================================================================================================================
 */
float vbat_get() {
  float v = analogRead(VBATPIN);
  v *= 2;    // we divided by 2, so multiply back
  v *= 3.3;  // Multiply by 3.3V, our reference voltage
  v /= 1024; // convert to voltage
  return (v);
}

/* 
 *=======================================================================================================================
 * I2C_Device_Exist - does i2c device exist at address
 * 
 *  The i2c_scanner uses the return value of the Write.endTransmisstion to see 
 *  if a device did acknowledge to the address.
 *=======================================================================================================================
 */
bool I2C_Device_Exist(byte address) {
  byte error;

  Wire.begin();                     // Connect to I2C as Master (no addess is passed to signal being a slave)

  Wire.beginTransmission(address);  // Begin a transmission to the I2C slave device with the given address. 
                                    // Subsequently, queue bytes for transmission with the write() function 
                                    // and transmit them by calling endTransmission(). 

  error = Wire.endTransmission();   // Ends a transmission to a slave device that was begun by beginTransmission() 
                                    // and transmits the bytes that were queued by write()
                                    // By default, endTransmission() sends a stop message after transmission, 
                                    // releasing the I2C bus.

  // endTransmission() returns a byte, which indicates the status of the transmission
  //  0:success
  //  1:data too long to fit in transmit buffer
  //  2:received NACK on transmit of address
  //  3:received NACK on transmit of data
  //  4:other error 

  // Partice Library Return values
  // SEE https://docs.particle.io/cards/firmware/wire-i2c/endtransmission/
  // 0: success
  // 1: busy timeout upon entering endTransmission()
  // 2: START bit generation timeout
  // 3: end of address transmission timeout
  // 4: data byte transfer timeout
  // 5: data byte transfer succeeded, busy timeout immediately after
  // 6: timeout waiting for peripheral to clear stop bit

  if (error == 0) {
    return (true);
  }
  else {
    // sprintf (msgbuf, "I2CERR: %d", error);
    // Output (msgbuf);
    return (false);
  }
}

/*
 * ======================================================================================================================
 * Blink() - Count, delay between, delay at end
 * ======================================================================================================================
 */
void Blink(int count, int between) {
  int c;

  for (c=0; c<count; c++) {
    digitalWrite(LED_PIN, HIGH);
    delay(between);
    digitalWrite(LED_PIN, LOW);
    delay(between);
  }
}

/*
 * ======================================================================================================================
 * FadeOn() - https://www.dfrobot.com/blog-596.html
 * ======================================================================================================================
 */
void FadeOn(unsigned int time,int increament) {
  for (byte value = 0 ; value < 255; value+=increament) {
  analogWrite(LED_PIN, value);
  delay(time/(255/5));
  }
}

/*
 * ======================================================================================================================
 * FadeOff() - 
 * ======================================================================================================================
 */
void FadeOff(unsigned int time,int decreament){
  for (byte value = 255; value >0; value-=decreament) {
  analogWrite(LED_PIN, value);
  delay(time/(255/5));
  }
}

/*
 *======================================================================================================================
 * myswap()
 *======================================================================================================================
 */
void myswap(unsigned int *p, unsigned int *q) {
  int t;
  
  t=*p;
  *p=*q;
  *q=t;
}

/*
 *======================================================================================================================
 * mysort()
 *======================================================================================================================
 */
void mysort(unsigned int a[], int n)
{
  unsigned int i,j,temp;

  for(i = 0;i < n-1;i++) {
    for(j = 0;j < n-i-1;j++) {
      if(a[j] > a[j+1])
        myswap(&a[j],&a[j+1]);
    }
  }
}

/*
 * =======================================================================================================================
 * isnumeric() - check if string contains all digits
 * =======================================================================================================================
 */
bool isnumeric(char *s) {
  for (int i=0; i< strlen(s); i++) {
    if (!isdigit(*(s+i)) ) {
      return(false);
    }
  }
  return(true);
}


/*
 * ======================================================================================================================
 * GetDeviceID() - 
 * 
 * This function reads the 128-bit (16-byte) unique serial number from the 
 * SAMD21 microcontroller's flash memory, compresses it to 12 bytes, and 
 * converts it to a 24-character hexadecimal string.
 * 
 * The compression is done by XORing the first 12 bytes with the last 12 bytes 
 * of the original 16-byte identifier, ensuring all 128 bits contribute to 
 * the final result.
 * 
 * SAM D21/DA1 Family Datas Sheet
 * 10.3.3 Serial Number
 * Each device has a unique 128-bit serial number which is a concatenation of four 32-bit words contained at the
 * following addresses:
 * Word 0: 0x0080A00C
 * Word 1: 0x0080A040
 * Word 2: 0x0080A044
 * Word 3: 0x0080A048
 * The uniqueness of the serial number is guaranteed only when using all 128 bits.
 * ======================================================================================================================
 */
void GetDeviceID() {
  // Pointer to the unique 128-bit serial number in flash
  uint32_t *uniqueID = (uint32_t *)0x0080A00C;
  
  // Temporary buffer to store 16 bytes (128 bits)
  uint8_t fullId[16];
  
  // Extract all 16 bytes from the 128-bit serial number
  for (int i = 0; i < 4; i++) {
    uint32_t val = uniqueID[i];
    fullId[i*4] = (val >> 24) & 0xFF;
    fullId[i*4 + 1] = (val >> 16) & 0xFF;
    fullId[i*4 + 2] = (val >> 8) & 0xFF;
    fullId[i*4 + 3] = val & 0xFF;
  }

  // Compress 16 bytes to 12 bytes using XOR
  uint8_t compressedId[12];
  for (int i = 0; i < 12; i++) {
    compressedId[i] = fullId[i] ^ fullId[i + 4];
  }

  memset (DeviceID, 0, 25);
  for (int i = 0; i < 12; i++) {
    sprintf (DeviceID+strlen(DeviceID), "%02x", compressedId[i]);
  }
  DeviceID[24] = '\0'; // Ensure null-termination
}

/* 
 *=======================================================================================================================
 * obs_interval_initialize() - observation interval 5,6,10,15,20,30
 *=======================================================================================================================
 */
void obs_interval_initialize() {
  if ((cf_obs_period != 5) && 
      (cf_obs_period != 6) && 
      (cf_obs_period != 10) &&
      (cf_obs_period != 15) &&
      (cf_obs_period != 20) &&
      (cf_obs_period != 30)) {
    sprintf (Buffer32Bytes, "OBS Interval:%dm Now:15m", cf_obs_period);
    Output(Buffer32Bytes);
    cf_obs_period = 15; 
  }
  else {
    sprintf (Buffer32Bytes, "OBS Interval:%dm", cf_obs_period);
    Output(Buffer32Bytes);    
  }
}

/* 
 *=======================================================================================================================
 * seconds_to_next_obs() - This will return seconds to next observation
 *=======================================================================================================================
 */
int seconds_to_next_obs() {
  now = rtc.now(); //get the current date-time
  int wd_sampletime = (cf_ds_enable || AS5600_exists | PM25AQI_exists) ? 60 : 0; // Lets start next obs 1 minute early if we have wind or distance

  // seconds remain until the next period boundary
  int stno = ( (cf_obs_period*60) - (now.unixtime() % (cf_obs_period*60)) ); // The mod operation gives us seconds passed last observation period 

  if (stno > wd_sampletime ) {
    stno = stno - wd_sampletime; // We want to start the observastion early to take wind, distance, air samples.
  }
  return (stno);
}

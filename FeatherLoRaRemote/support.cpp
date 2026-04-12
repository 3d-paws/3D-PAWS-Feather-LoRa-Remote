/*
 * ======================================================================================================================
 *  support.cpp - Support Functions
 * ======================================================================================================================
 */
#include "include/cf.h"
#include "include/feather.h"
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

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * I2C_Device_Exist - does i2c device exist at address
 * 
 *  The i2c_scanner uses the return value of the Write.endTransmisstion to see 
 *  if a device did acknowledge to the address.
 *=======================================================================================================================
 */
bool I2C_Device_Exist(uint8_t address) {
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

  // Lets start next obs 1 minute early if we have wind distance or air
  int wd_sampletime = (!cf_nowind || PM25AQI_exists || (cf_op1 == OP1_STATE_DIST_5M) || (cf_op1 == OP1_STATE_DIST_10M)) ? 60 : 0; 

  // seconds remain until the next period boundary
  int stno = ( (cf_obs_period*60) - (now.unixtime() % (cf_obs_period*60)) ); // The mod operation gives us seconds passed last observation period 

  if (stno > wd_sampletime ) {
    stno = stno - wd_sampletime; // We want to start the observastion early to take wind, distance, air samples.
  }
  return (stno);
}

/*
 * ======================================================================================================================
 *  wrda.cpp - Wind Rain Distance Air Functions
 * ======================================================================================================================
 */
#include <Arduino.h>
#include <Wire.h>
#include <ArduinoLowPower.h>

#include "include/qc.h"
#include "include/ssbits.h"
#include "include/mux.h"
#include "include/sensors.h"
#include "include/cf.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/wrda.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 *  Wind
 * ======================================================================================================================
 */
WIND_STR wind;

/*
 * ======================================================================================================================
 *  Wind Direction - AS5600 Sensor
 * ======================================================================================================================
 */
bool      AS5600_exists     = false;
int       AS5600_ADR        = 0x36;
const int AS5600_raw_ang_hi = 0x0c;
const int AS5600_raw_ang_lo = 0x0d;

/*
 * ======================================================================================================================
 *  Wind Speed Calibration
 * ======================================================================================================================
 */
float ws_calibration = 2.64;       // From wind tunnel testing
float ws_radius = 0.079;           // In meters

/*
 * ======================================================================================================================
 *  Optipolar Hall Effect Sensor SS451A - Interrupt 1 - Anemometer
 * ======================================================================================================================
 */
volatile unsigned int anemometer_interrupt_count;
unsigned long anemometer_interrupt_stime;

/*
 * ======================================================================================================================
 *  Rain Gauge 1 - Optipolar Hall Effect Sensor SS451A
 * ======================================================================================================================
 */
volatile unsigned int raingauge1_interrupt_count=0;
unsigned long raingauge1_interrupt_stime; // Send Time
volatile unsigned long raingauge1_interrupt_ltime; // Last Time
unsigned long raingauge1_interrupt_toi;   // Time of Interrupt

/*
 * ======================================================================================================================
 *  Rain Gauge 2 - Optipolar Hall Effect Sensor SS451A
 * ======================================================================================================================
 */
volatile unsigned int raingauge2_interrupt_count=0;
unsigned long raingauge2_interrupt_stime; // Send Time
volatile unsigned long raingauge2_interrupt_ltime; // Last Time
unsigned long raingauge2_interrupt_toi;   // Time of Interrupt


/*
 * =======================================================================================================================
 *  Distance Gauge
 * =======================================================================================================================
 */
/*
 * Distance Sensors
 * The 5-meter sensors (MB7360, MB7369, MB7380, and MB7389) use a scale factor of (Vcc/5120) per 1-mm.
 * Particle 12bit resolution (0-4095),  Sensor has a resolution of 0 - 5119mm,  Each unit of the 0-4095 resolution is 1.25mm
 * Feather has 10bit resolution (0-1023), Sensor has a resolution of 0 - 5119mm, Each unit of the 0-1023 resolution is 5mm
 *
 * The 10-meter sensors (MB7363, MB7366, MB7383, and MB7386) use a scale factor of (Vcc/10240) per 1-mm.
 * Particle 12bit resolution (0-4095), Sensor has a resolution of 0 - 10239mm, Each unit of the 0-4095 resolution is 2.5mm
 * Feather has 10bit resolution (0-1023), Sensor has a resolution of 0 - 10239mm, Each unit of the 0-1023 resolution is 10mm
 *
 * The distance sensor will report as type sg  for Snow, Stream, or Surge gauge deployments.
 * A Median value based on 60 samples 250ms apart is obtain. Then subtracted from ds_baseline for the observation.
 */
unsigned int dg_bucket = 0;
unsigned int dg_resolution_adjust = 5; // Default is 5m sensor
unsigned int dg_buckets[DG_BUCKETS];

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  anemometer_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void anemometer_interrupt_handler()
{
  anemometer_interrupt_count++;
}

/*
 * ======================================================================================================================
 *  raingauge1_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void raingauge1_interrupt_handler()
{
  unsigned long now = millis();
  if ((now - raingauge1_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    raingauge1_interrupt_ltime = now;
    raingauge1_interrupt_count++;
  }
}

/*
 * ======================================================================================================================
 *  raingauge1_sample() - return rain amount since last sample
 * ======================================================================================================================
 */
float raingauge1_sample() {
  unsigned long time_ms, rg1ds, count;
  float rg1;

  noInterrupts();
  time_ms = millis();
  rg1ds = (time_ms - raingauge1_interrupt_stime) / 1000;
  count = raingauge1_interrupt_count;
  raingauge1_interrupt_count = 0;
  raingauge1_interrupt_stime = time_ms;
  raingauge1_interrupt_ltime = 0;
  interrupts();

  rg1 = count * 0.2f;
  rg1 = (isnan(rg1) || (rg1 < QC_MIN_RG) || (rg1 > (((float)rg1ds / 60.0f) * QC_MAX_RG))) ? QC_ERR_RG : rg1;

  return rg1;
}

/*
 * ======================================================================================================================
 *  raingauge2_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void raingauge2_interrupt_handler()
{
  unsigned long now = millis();
  if ((now - raingauge2_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    raingauge2_interrupt_ltime = now;
    raingauge2_interrupt_count++;
  }
}

/*
 * ======================================================================================================================
 *  raingauge2_sample() - return rain amount since last sample
 * ======================================================================================================================
 */
float raingauge2_sample() {
  unsigned long time_ms, rg2ds, count;
  float rg2;

  noInterrupts();
  time_ms = millis();
  rg2ds = (time_ms - raingauge2_interrupt_stime) / 1000;
  count = raingauge2_interrupt_count;
  raingauge2_interrupt_count = 0;
  raingauge2_interrupt_stime = time_ms;
  raingauge2_interrupt_ltime = 0;
  interrupts();

  rg2 = count * 0.2f;
  rg2 = (isnan(rg2) || (rg2 < QC_MIN_RG) || (rg2 > (((float)rg2ds / 60.0f) * QC_MAX_RG))) ? QC_ERR_RG : rg2;

  return rg2;
}

/* 
 * =======================================================================================================================
 * RainEnabled()
 * =======================================================================================================================
 */
bool RainEnabled() {
  if (cf_rg1_enable || (cf_op1 == OP1_STATE_RAIN)) {
    return (true);
  }
  else {
    return (false);
  }
}

/* 
 *=======================================================================================================================
 * Wind_SampleSpeed() - Return a wind speed based on interrupts and duration wind
 * 
 * Optipolar Hall Effect Sensor SS451A - Anemometer
 * speed  = (( (signals/2) * (2 * pi * radius) ) / time) * calibration_factor
 * speed in m/s =  (   ( (interrupts/2) * (2 * 3.14156 * 0.079) )  / (time_period in ms / 1000)  )  * 2.64
 *=======================================================================================================================
 */
float Wind_SampleSpeed() {
  unsigned long time_ms, delta_ms, count;
  float wind_speed;

  noInterrupts();
  count = anemometer_interrupt_count;
  anemometer_interrupt_count = 0;
  time_ms = millis();
  anemometer_interrupt_stime = time_ms;
  interrupts();

  // Unsigned subtraction naturally wraps on rollover, so if time_ms has rolled past zero and anemometer_interrupt_stime 
  // is still the old large value, the subtraction still produces the correct elapsed time.
  delta_ms = time_ms - anemometer_interrupt_stime;

  if (count && delta_ms > 0) {
    wind_speed = ((count * 3.14156f * ws_radius) / ((float)delta_ms / 1000.0f)) * ws_calibration;
  } else {
    wind_speed = 0.0f;
  }

  return wind_speed;
}

/* 
 *=======================================================================================================================
 * Wind_SampleDirection() -- Talk i2c to the AS5600 sensor and get direction
 *=======================================================================================================================
 */
int Wind_SampleDirection() {
  int degree;
  
  // Read Raw Angle Low Byte
  Wire.beginTransmission(AS5600_ADR);
  Wire.write(AS5600_raw_ang_lo);
  if (Wire.endTransmission()) {
    if (AS5600_exists) {
      Output ("WD Offline_L");
    }
    AS5600_exists = false;
  }
  else if (Wire.requestFrom(AS5600_ADR, 1)) {
    int AS5600_lo_raw = Wire.read();
  
    // Read Raw Angle High Byte
    Wire.beginTransmission(AS5600_ADR);
    Wire.write(AS5600_raw_ang_hi);
    if (Wire.endTransmission()) {
      if (AS5600_exists) {
        Output ("WD Offline_H");
      }
      AS5600_exists = false;
    }
    else if (Wire.requestFrom(AS5600_ADR, 1)) {
      word AS5600_hi_raw = Wire.read();

      if (!AS5600_exists) {
        Output ("WD Online");
      }
      AS5600_exists = true;           // We made it 
      
      AS5600_hi_raw = AS5600_hi_raw << 8; //shift raw angle hi 8 left
      AS5600_hi_raw = AS5600_hi_raw | AS5600_lo_raw; //AND high and low raw angle value

      // Do data integ chec
      degree = (int) AS5600_hi_raw * 0.0879;
      if ((degree >=0) && (degree <= 360)) {
        return (degree);
      }
      else {
        return (-1);
      }
    }
  }
  return (-1); // Not the best value to return 
}

/* 
 *=======================================================================================================================
 * Wind_DirectionVector()
 *=======================================================================================================================
 */
int Wind_DirectionVector() {
  double NS_vector_sum = 0.0;
  double EW_vector_sum = 0.0;
  double r;
  float s;
  int d, i, rtod;
  bool ws_zero = true;

  for (i=0; i<WIND_READINGS; i++) {
    d = wind.bucket[i].direction;

    // if at any time 1 of the 60 wind direction readings is -1
    // then the sensor was offline and we need to invalidate or data
    // until it is clean with out any -1's
    if (d == -1) {
      return (-1);
    }
    
    s = wind.bucket[i].speed;

    // Flag we have wind speed
    if (s > 0) {
      ws_zero = false;  
    }
    r = (d * 71) / 4068.0;
    
    // North South Direction 
    NS_vector_sum += cos(r) * s;
    EW_vector_sum += sin(r) * s;
  }
  rtod = (atan2(EW_vector_sum, NS_vector_sum)*4068.0)/71.0;
  if (rtod<0) {
    rtod = 360 + rtod;
  }

  // If all the winds speeds are 0 then we return current wind direction or 0 on failure of that.
  if (ws_zero) {
    return (Wind_SampleDirection()); // Can return -1
  }
  else {
    return (rtod);
  }
}

/* 
 *=======================================================================================================================
 * Wind_SpeedAverage()
 *=======================================================================================================================
 */
float Wind_SpeedAverage() {
  float wind_speed = 0.0;
  for (int i=0; i<WIND_READINGS; i++) {
    // sum wind speeds for later average
    wind_speed += wind.bucket[i].speed;
  }
  return( wind_speed / (float) WIND_READINGS);
}

/* 
 *=======================================================================================================================
 * Wind_Gust()
 *=======================================================================================================================
 */
float Wind_Gust() {
  return(wind.gust);
}

/* 
 *=======================================================================================================================
 * Wind_GustDirection()
 *=======================================================================================================================
 */
int Wind_GustDirection() {
  return(wind.gust_direction);
}

/* 
 *=======================================================================================================================
 * Wind_GustUpdate()
 *   Wind Gust = Highest 3 consecutive samples from the 60 samples. The 3 samples are then averaged.
 *   Wind Gust Direction = Average of the 3 Vectors from the Wind Gust samples.
 * 
 *   Note: To handle the case of 2 or more gusts at the same speed but different directions
 *          Sstart with oldest reading and work forward to report most recent.
 * 
 *   Algorithm: 
 *     Start with oldest reading.
 *     Sum this reading with next 2.
 *     If greater than last, update last 
 * 
 *=======================================================================================================================
 */
void Wind_GustUpdate() {
  int bucket = wind.bucket_idx; // Start at next bucket to fill (aka oldest reading)
  float ws_sum = 0.0;
  int ws_bucket = bucket;
  float sum;

  for (int i=0; i<(WIND_READINGS-2); i++) {  // subtract 2 because we are looking ahead at the next 2 buckets
    // sum wind speeds 
    sum = wind.bucket[bucket].speed +
          wind.bucket[(bucket+1) % WIND_READINGS].speed +
          wind.bucket[(bucket+2) % WIND_READINGS].speed;
    if (sum >= ws_sum) {
      ws_sum = sum;
      ws_bucket = bucket;
    }
    bucket = (bucket+1) % WIND_READINGS;
  }
  wind.gust = ws_sum/3;
  
  // Determine Gust Direction 
  double NS_vector_sum = 0.0;
  double EW_vector_sum = 0.0;
  double r;
  float s;
  int d, i, rtod;
  bool ws_zero = true;

  bucket = ws_bucket;
  for (i=0; i<3; i++) {
    d = wind.bucket[bucket].direction;

    // if at any time any wind direction readings is -1
    // then the sensor was offline and we need to invalidate or data
    // until it is clean with out any -1's
    if (d == -1) {
      ws_zero = true;
      break;
    }
    
    s = wind.bucket[bucket].speed;

    // Flag we have wind speed
    if (s > 0) {
      ws_zero = false;  
    }
    r = (d * 71) / 4068.0;
    
    // North South Direction 
    NS_vector_sum += cos(r) * s;
    EW_vector_sum += sin(r) * s;

    bucket = (bucket+1) % WIND_READINGS;
  }

  rtod = (atan2(EW_vector_sum, NS_vector_sum)*4068.0)/71.0;
  if (rtod<0) {
    rtod = 360 + rtod;
  }

  // If all the winds speeds are 0 or we have a -1 direction then set -1 for direction.
  if (ws_zero) {
    wind.gust_direction = -1;
  }
  else {
    wind.gust_direction = rtod;
  }
}

/*
 * ======================================================================================================================
 * Wind_TakeReading() - Wind direction and speed, measure every second             
 * ======================================================================================================================
 */
void Wind_TakeReading() {
  wind.bucket[wind.bucket_idx].direction = (int) Wind_SampleDirection();
  wind.bucket[wind.bucket_idx].speed = Wind_SampleSpeed();
  wind.bucket_idx = (wind.bucket_idx+1) % WIND_READINGS; // Advance bucket index for next reading
}

/* 
 *=======================================================================================================================
 * as5600_initialize() - wind direction sensor I2C 0x36
 *=======================================================================================================================
 */
void as5600_initialize() {
  Output("AS5600:INIT");
  Wire.beginTransmission(AS5600_ADR);
  if (Wire.endTransmission()) {
    msgp = (char *) "WD:NF";
    AS5600_exists = false;
  }
  else {
    msgp = (char *) "WD:OK";
    AS5600_exists = true;
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * Pin_ReadAvg()
 *=======================================================================================================================
 */
float Pin_ReadAvg(int pin) {
  int numReadings = 5;
  int totalValue = 0;
  for (int i = 0; i < numReadings; i++) {
    totalValue += analogRead(pin);
    delay(10);  // Short delay between readings
  }
  return(totalValue / numReadings);
}

/* 
 *=======================================================================================================================
 * VoltaicVoltage() - Breakout the Voltaic Cell Voltage from the UCB-C 
 * 
 * Analog A0-A6	0-3.6V	ADC input range
 * 
 * Info: https://blog.voltaicsystems.com/reading-charge-level-of-voltaic-usb-battery-packs/ for  D+ 
 * Info: https://blog.voltaicsystems.com/updated-usb-c-pd-and-always-on-for-v25-v50-v75-batteries/ for SBU
 * 
 * Newer V25/V50/V75 firmware (post-2023) moved this cell voltage signal to SBU pins (A8/B8) on USB-C for better USB 
 * compliance. Older units used D+. Which conflicted with data transfer. 
 * 
 * Using both A8/B8 SBU pins ensures compatibility regardless of USB-C cable orientation (flipped or not)
 * 
 * Voltaic’s docs say the pack reports half the cell voltage.
 * The expected voltage range on Voltaic V25's monitor pins (D+ on older firmware, 
 *   SBU1 (A8) and SBU2 (B8) on newer) is 1.6V to 2.1V
 *=======================================================================================================================
 */
float VoltaicVoltage(int pin) {
  int numReadings = 5;
  int totalValue = 0;
  for (int i = 0; i < numReadings; i++) {
    totalValue += analogRead(pin);
    delay(10);  // Short delay between readings
  }
  float voltage = (3.3 * (totalValue / (float)numReadings)) / 4095.0; 
  return(voltage);
}

/*
 *=======================================================================================================================
 * VoltaicVoltage() - Voltaic Cell Percent Charge 
 *   Full charge: 4.2V cell → 2.1V on SBU (100%)  
 *   75% charge:  ~3.9V cell → ~1.95V on SBU  
 *   50% charge:  ~3.7V cell → ~1.85V on SBU  
 *   25% charge:  ~3.4V cell → ~1.7V on SBU  
 *   Empty:       3.2V cell → 1.6V on SBU (0%)
 *=======================================================================================================================
 */
float VoltaicPercent(float half_cell_voltage) {
  float cellV = half_cell_voltage * 2.0;
  
  if (cellV >= 4.20) return 100.0;
  if (cellV <= 3.20) return 0.0;
  
  // Simple linear approximation over Voltaic's specified 3.2-4.2V range
  // (Li-ion curve is flat in middle, so voltage alone is rough anyway)
  float percent = ((cellV - 3.20) / (4.20 - 3.20)) * 100.0;
  return constrain(percent, 0, 100);
}

/*
 * ======================================================================================================================
 * DS_TakeReading() - measure every second             
 * ======================================================================================================================
 */
void DS_TakeReading() {
  dg_buckets[dg_bucket] = (int) analogRead(DISTANCE_GAUGE_PIN) * dg_resolution_adjust;
  dg_bucket = (++dg_bucket) % DG_BUCKETS; // Advance bucket index for next reading
}

/* 
 *=======================================================================================================================
 * DS_Median()
 *=======================================================================================================================
 */
float DS_Median() {
  int i;
  
  mysort(dg_buckets, DG_BUCKETS);
  i = (DG_BUCKETS+1) / 2 - 1; // -1 as array indexing in C starts from 0
  
  return (dg_buckets[i]); 
}

/* 
 *=======================================================================================================================
 * Do_WRDA_Samples() - Do Wind, Distance and Air Quality 1 minute of 1 second samples
 *=======================================================================================================================
 */
void Do_WRDA_Samples() {
  if (!cf_nowind || PM25AQI_exists || (cf_op1 == OP1_STATE_DIST_5M) || (cf_op1 == OP1_STATE_DIST_10M)) {
    Output ("WRDA_Sample()");

    if (PM25AQI_exists) {
      Output("AQS:WAKEUP");
      
      digitalWrite(PM25AQI_PIN, HIGH); // Wakeup Air Quality Sensor
      
      // Clear readings
      pm25aqi_clear();
    }

    if (!cf_nowind) {
      // Init default values.
      wind.gust = 0.0;
      wind.gust_direction = -1;
      wind.bucket_idx = 0;
      // Clear windspeed interrupt count by reading and tossing
      Wind_SampleSpeed(); 
    }

    // Take 60 1s samples of wind speed and direction and fill arrays with values.
    Output("SAMPLING");
    for (int i=0; i< 60; i++) {
      if (!cf_nowind) {
        Wind_TakeReading();
      }

      if ((cf_op1 == OP1_STATE_DIST_5M) || (cf_op1 == OP1_STATE_DIST_10M)) {
        DS_TakeReading();
      }

      if (PM25AQI_exists) {
        PM25_AQI_Data aqid;
        
        if (i==30) {
          mux_channel_set(MUX_AQ_CHANNEL);
          pmaq.read(&aqid); // Toss 1st reading after wakeup
          mux_deselect_all();
          if (SerialConsoleEnabled) Serial.println(); // put a return after the printed dots
        }

        if ((i>30 && i<=40)) { // 10 1s samples
          mux_channel_set(MUX_AQ_CHANNEL);
          if (pmaq.read(&aqid)) {
            pm25aqi_obs.count++;
            pm25aqi_obs.max_s10  += aqid.pm10_standard;
            pm25aqi_obs.max_s25  += aqid.pm25_standard;
            pm25aqi_obs.max_s100 += aqid.pm100_standard;
            pm25aqi_obs.max_e10  += aqid.pm10_env;
            pm25aqi_obs.max_e25  += aqid.pm25_env;
            pm25aqi_obs.max_e100 += aqid.pm100_env;
            sprintf (Buffer32Bytes, "AQ[%d][%d]", pm25aqi_obs.count, pm25aqi_obs.max_s10);
            Output (Buffer32Bytes);  
          }
          else {
            pm25aqi_obs.fail_count++;
          } 
          mux_deselect_all();     
        }
        if (i == 41) {
          Output("AQS:SLEEP");
          
          // Pulling the PM25AQI SET pin LOW to put the sensor to sleep can cause I2C communication issues 
          // with other devices if the sensor shares the I2C bus, as this pin controls internal circuitry that 
          // may affect the bus state or sensor logic. This scenario is documented with some I2C sensor variants 
          // (PMSA003I/PM25AQI) and can lead to bus lockup if the sensor does not fully release the 
          // SDA/SCL lines when "asleep".
              
          // Disconnect mux channel before we power dower the AQ sensor. To avoid the above.
          mux_deselect_all();  
          digitalWrite(PM25AQI_PIN, LOW); // Put to Sleep Air Quality Sensor  
        }
      }
      if (SerialConsoleEnabled) Serial.print(".");  // Provide Serial Console some feedback as we loop and wait til next observation
      OLED_spin();
      delay (1000);
    }

    if (SerialConsoleEnabled) Serial.println();  // Send a newline out to cleanup after all the periods we have been logging
    
    if (PM25AQI_exists) {
      if (pm25aqi_obs.fail_count > pm25aqi_obs.count) {
        // Fail if half our sample reads failed. - I think this is reasonable - rjb
        Output("AQS:FAIL");
        pm25aqi_obs.max_s10 = -999;
        pm25aqi_obs.max_s25 = -999;
        pm25aqi_obs.max_s100 = -999;
        pm25aqi_obs.max_e10 = -999;
        pm25aqi_obs.max_e25 = -999;
        pm25aqi_obs.max_e100 = -999;
      }
      else {
        // Do average
        Output("AQS:OK");
        pm25aqi_obs.max_s10  = (pm25aqi_obs.max_s10 / pm25aqi_obs.count);
        pm25aqi_obs.max_s25  = (pm25aqi_obs.max_s25 / pm25aqi_obs.count);
        pm25aqi_obs.max_s100 = (pm25aqi_obs.max_s100 / pm25aqi_obs.count);
        pm25aqi_obs.max_e10  = (pm25aqi_obs.max_e10 / pm25aqi_obs.count);
        pm25aqi_obs.max_e25  = (pm25aqi_obs.max_e25 / pm25aqi_obs.count);
        pm25aqi_obs.max_e100 = (pm25aqi_obs.max_e100 / pm25aqi_obs.count); 
      }      
    }

    if (!cf_nowind) {
      Wind_GustUpdate(); // Update Gust and Gust Direction readings
    }
  }
}

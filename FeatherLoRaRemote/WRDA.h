/*
 * ======================================================================================================================
 *  WRDA.h - Wind Rain Distance Air Functions
 * ======================================================================================================================
 */

 // Prototyping functions to aviod compile function unknown issue.
 void mux_channel_set(uint8_t channel);
#ifndef MUX_AQ_CHANNEL
#define MUX_AQ_CHANNEL 7
#endif

 /*
 * ======================================================================================================================
 *  Rain Gauges
 * ======================================================================================================================
 */
#define RAINGAUGE1_IRQ_PIN  A3
#define RAINGAUGE2_IRQ_PIN  A4

/*
 * ======================================================================================================================
 *  Rain Gauge 1 - Optipolar Hall Effect Sensor SS451A
 * ======================================================================================================================
 */
volatile unsigned int raingauge1_interrupt_count=0;
uint64_t raingauge1_interrupt_stime; // Send Time
uint64_t raingauge1_interrupt_ltime; // Last Time
uint64_t raingauge1_interrupt_toi;   // Time of Interrupt

/*
 * ======================================================================================================================
 *  raingauge1_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void raingauge1_interrupt_handler()
{
  if ((millis() - raingauge1_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    raingauge1_interrupt_ltime = millis();
    raingauge1_interrupt_count++;
  }   
}

/*
 * ======================================================================================================================
 *  Rain Gauge 2 - Optipolar Hall Effect Sensor SS451A
 * ======================================================================================================================
 */
volatile unsigned int raingauge2_interrupt_count=0;
uint64_t raingauge2_interrupt_stime; // Send Time
uint64_t raingauge2_interrupt_ltime; // Last Time
uint64_t raingauge2_interrupt_toi;   // Time of Interrupt

/*
 * ======================================================================================================================
 *  raingauge2_interrupt_handler() - This function is called whenever a magnet/interrupt is detected by the arduino
 * ======================================================================================================================
 */
void raingauge2_interrupt_handler()
{
  if ((millis() - raingauge2_interrupt_ltime) > 500) { // Count tip if a half second has gone by since last interrupt
    raingauge2_interrupt_ltime = millis();
    raingauge2_interrupt_count++;
  }   
}

/*
 * ======================================================================================================================
 *  SAMPLE Buckets
 * ======================================================================================================================  
 */
#define SAMPLES 60

/*
 * ======================================================================================================================
 *  Wind Related Setup
 * 
 *  NOTE: With interrupts tied to the anemometer rotation we are essentually sampling all the time.  
 *        We record the interrupt count, ms duration and wind direction every second.
 *        One revolution of the anemometer results in 2 interrupts. There are 2 magnets on the anemometer.
 * 
 *        Station observations are logged every minute
 *        Wind and Direction are sampled every second producing 60 samples 
 *        The one second wind speed sample are produced from the interrupt count and ms duration.
 *        Wind Observations a 
 *        Reported Observations
 *          Wind Speed = Average of the 60 samples.
 *          Wind Direction = Average of the 60 vectors from Direction and Speed.
 *          Wind Gust = Highest 3 consecutive samples from the 60 samples. The 3 samples are then averaged.
 *          Wind Gust Direction = Average of the 3 Vectors from the Wind Gust samples.
 *          
 * ======================================================================================================================
 */
 
#define ANEMOMETER_IRQ_PIN  A2
#define WIND_READINGS       SAMPLES       // One minute of 1s Samples

typedef struct {
  int direction;
  float speed;
} WIND_BUCKETS_STR;

typedef struct {
  WIND_BUCKETS_STR bucket[WIND_READINGS];
  int bucket_idx;
  float gust;
  int gust_direction;
  int sample_count;
} WIND_STR;
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
uint64_t anemometer_interrupt_stime;

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
 *=======================================================================================================================
 * Wind_SampleSpeed() - Return a wind speed based on interrupts and duration wind
 * 
 * Optipolar Hall Effect Sensor SS451A - Anemometer
 * speed  = (( (signals/2) * (2 * pi * radius) ) / time) * calibration_factor
 * speed in m/s =  (   ( (interrupts/2) * (2 * 3.14156 * 0.079) )  / (time_period in ms / 1000)  )  * 2.64
 *=======================================================================================================================
 */
float Wind_SampleSpeed() {
  unsigned long delta_ms, time_ms;
  float wind_speed;
  
  time_ms = millis();

  // Handle the clock rollover after about 50 days.
  if (time_ms < anemometer_interrupt_stime) {
    delta_ms = (0xFFFFFFFF - anemometer_interrupt_stime) + time_ms;
  }
  else {
    delta_ms = millis()-anemometer_interrupt_stime;
  }
  
  if (anemometer_interrupt_count && (delta_ms>0)) {
    wind_speed = ( ( anemometer_interrupt_count * 3.14156 * ws_radius)  / 
      (float)( (float)delta_ms / 1000) )  * ws_calibration;
  }
  else {
    wind_speed = 0.0;
  }

  anemometer_interrupt_count = 0;
  anemometer_interrupt_stime = millis(); 
  return (wind_speed);
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
      SystemStatusBits &= ~SSB_AS5600; // Turn Off Bit
      
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
  SystemStatusBits |= SSB_AS5600;  // Turn On Bit
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
 * Wind_ClearSampleCount()    
 * ======================================================================================================================
 */
void Wind_ClearSampleCount() {
  wind.sample_count = 0;
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
  wind.sample_count++;
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
    SystemStatusBits |= SSB_AS5600;  // Turn On Bit
  }
  else {
    msgp = (char *) "WD:OK";
    AS5600_exists = true;
  }
  Output (msgp);
}

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
 
#define DISTANCE_GAUGE_PIN     A5
#define DG_BUCKETS SAMPLES

unsigned int dg_bucket = 0;
unsigned int dg_resolution_adjust = 5; // Default is 5m sensor
unsigned int dg_buckets[DG_BUCKETS];

/*
 * ======================================================================================================================
 * DistanceGauge_TakeReading - measure every second             
 * ======================================================================================================================
 */
void DistanceGauge_TakeReading() {
  dg_buckets[dg_bucket] = (int) analogRead(DISTANCE_GAUGE_PIN) * dg_resolution_adjust;
  dg_bucket = (++dg_bucket) % DG_BUCKETS; // Advance bucket index for next reading
}

/* 
 *=======================================================================================================================
 * DG_Initialize() - Distance Sensor
 *=======================================================================================================================
 */
void DS_Initialize() {
  Output ("DS:INIT");
  if (cf_ds_enable) {
    if (cf_ds_enable == 5) {
      dg_resolution_adjust = 5;
      Output ("DIST=5M");
    }
    else if (cf_ds_enable == 10) {
      dg_resolution_adjust = 10;
      Output ("DIST=10M");
    }
    else {
      Output ("DS:Config Invalid");
      cf_ds_enable = 0;
    }
  }
  else {
    Output ("DS:Not Configured");
  }
}

/* 
 *=======================================================================================================================
 * DistanceGauge_Median()
 *=======================================================================================================================
 */
float DistanceGauge_Median() {
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
  if (cf_ds_enable || AS5600_exists || PM25AQI_exists) {
    Output ("WRDA_Sample()");

    if (PM25AQI_exists) {
      Output("AQS:WAKEUP");
      
      digitalWrite(PM25AQI_PIN, HIGH); // Wakeup Air Quality Sensor
      
      // Clear readings
      pm25aqi_clear();
    }

    if (AS5600_exists) {
      // Clear windspeed counter  
      anemometer_interrupt_count = 0;
      anemometer_interrupt_stime = millis();
  
      // Init default values.
      wind.gust = 0.0;
      wind.gust_direction = -1;
      wind.bucket_idx = 0;
    }

    // Take 60 1s samples of wind speed and direction and fill arrays with values.
    Output("SAMPLING");
    for (int i=0; i< 60; i++) {
      if (AS5600_exists) {
        Wind_TakeReading();
      }
      if (cf_ds_enable) {
        DistanceGauge_TakeReading();
      }
      if (PM25AQI_exists) {
        PM25_AQI_Data aqid;
        
        if (i==30) {
          mux_channel_set(MUX_AQ_CHANNEL);
          pmaq.read(&aqid); // Toss 1st reading after wakeup
          mux_channel_set(0);
        }
        if (i>30 && i<=40) { // 10 1s samples
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
          mux_channel_set(0);         
        }
        if (i == 41) {
          Output("AQS:SLEEP");
          
          // Pulling the PM25AQI SET pin LOW to put the sensor to sleep can cause I2C communication issues 
          // with other devices if the sensor shares the I2C bus, as this pin controls internal circuitry that 
          // may affect the bus state or sensor logic. This scenario is documented with some I2C sensor variants 
          // (PMSA003I/PM25AQI) and can lead to bus lockup if the sensor does not fully release the 
          // SDA/SCL lines when "asleep".
              
          // Move to another mux channel before we power dower the AQ sensor. To avoid the above.
          mux_channel_set(0);  
          
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

    if (AS5600_exists) {
      Wind_GustUpdate(); // Update Gust and Gust Direction readings
    }
  }
}

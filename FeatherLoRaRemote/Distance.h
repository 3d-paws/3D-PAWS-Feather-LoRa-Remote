/*
 * ======================================================================================================================
 *  Distance.h - Distance Sensor Functions
 * ======================================================================================================================
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
 
/*
 * =======================================================================================================================
 *  Distance Sensor
 * =======================================================================================================================
 */
#define DS_PIN     A4
#define DS_BUCKETS 60

unsigned int ds_bucket = 0;
unsigned int ds_buckets[DS_BUCKETS];

/* 
 *=======================================================================================================================
 * DS_Median()
 *=======================================================================================================================
 */
float DS_Median() {
  int i;

  for (i=0; i<DS_BUCKETS; i++) {
    // delay(500);
    delay(250);
    ds_buckets[i] = (int) analogRead(DS_PIN);
    // sprintf (Buffer32Bytes, "DS[%02d]:%d", i, ds_buckets[i]);
    // OutputNS (Buffer32Bytes);
  }
  
  mysort(ds_buckets, DS_BUCKETS);
  i = (DS_BUCKETS+1) / 2 - 1; // -1 as array indexing in C starts from 0

  if (cf_ds_type) {  // 0 = 5m, 1 = 10m
    return (ds_buckets[i]*10);
  }
  else {
    return (ds_buckets[i]*5);
  }
}

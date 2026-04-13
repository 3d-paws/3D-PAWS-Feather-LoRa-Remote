/*
 * ======================================================================================================================
 *  gps.h - GPS Definations
 * 
 * Adafruit Mini GPS PA1010D - UART and I2C - STEMMA QT https://www.adafruit.com/product/4415
 * ======================================================================================================================
 */
#include <SparkFun_I2C_GPS_Arduino_Library.h> // Command and control the I2C GPS module
#include <TinyGPS++.h>                        // Parse GPS data stream

#define GPS_ADDRESS 0x10
#define GPS_RST_PIN 5

#define GPS_STANDBY_MODE 0
#define GPS_BACKUP_MODE 1
#define GPS_PERIODIC_MODE 2

// Extern variables
extern bool   gps_exists;
extern bool   gps_valid;
extern char   gps_timestamp[32];
extern double gps_lat;
extern double gps_lon;
extern double gps_altm;
extern double gps_altf;
extern int    gps_sat;
extern float  gps_hdop;
extern bool   gps_on;

// Function prototypes
void gps_displayInfo();
bool gps_aquire();
void gps_keepoff();
void gps_initialize();



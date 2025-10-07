/*
 * ======================================================================================================================
 *  sensors.h - I2C Sensor Definations
 * ======================================================================================================================
 */

#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_VEML7700.h>
#include <Adafruit_PM25AQI.h>
#include <Adafruit_HDC302x.h>
#include <Adafruit_LPS35HW.h>
#include <i2cArduino.h>
#include <LeafSens.h>
#include <i2cMultiSm.h>

/*
 * ======================================================================================================================
 *  BMX280 humidity - I2C - Temperature, pressure sensor & altitude - Support 2 of any combination
 * 
 *  https://www.asknumbers.com/PressureConversion.aspx
 *  Pressure is returned in the SI units of Pascals. 100 Pascals = 1 hPa = 1 millibar. 
 *  Often times barometric pressure is reported in millibar or inches-mercury. 
 *  For future reference 1 pascal = 0.000295333727 inches of mercury, or 1 inch Hg = 3386.39 Pascal. 
 *
 *  Looks like you divide by 100 and you get millibars which matches NWS page
 * 
 *  Surface Observations and Station Elevation 
 *  https://forecast.weather.gov/product.php?issuedby=BOU&product=OSO&site=bou 
 * 
 * The Bosch BMPXXX sensors all have this issue of saving the last reading. 
 * The recommended solution is to always throw away the first reading after any long 
 * period of inactivity (sleep or power on).
 * SEE https://forums.adafruit.com/viewtopic.php?t=209906
 * ======================================================================================================================
 */

// #define BMX_STATION_ELEVATION 1017.272  // default 1013.25
#define BMX_ADDRESS_1         0x77      // BMP Default Address - Connecting SDO to GND will change BMP to 0x76
#define BMX_ADDRESS_2         0x76      // BME Default Address - Connecting SDO to GND will change BME to 0x77
#define BMP280_CHIP_ID        0x58
#define BME280_BMP390_CHIP_ID 0x60
#define BMP388_CHIP_ID        0x50
#define BMX_TYPE_UNKNOWN      0
#define BMX_TYPE_BMP280       1
#define BMX_TYPE_BME280       2
#define BMX_TYPE_BMP388       3
#define BMX_TYPE_BMP390       4

// Extern declarations for sensor objects
extern Adafruit_BMP280 bmp1;
extern Adafruit_BMP280 bmp2;
extern Adafruit_BME280 bme1;
extern Adafruit_BME280 bme2;
extern Adafruit_BMP3XX bm31;
extern Adafruit_BMP3XX bm32;

// Extern declarations for variables
extern byte BMX_1_chip_id;
extern byte BMX_2_chip_id;
extern bool BMX_1_exists;
extern bool BMX_2_exists;
extern byte BMX_1_type;
extern byte BMX_2_type;
extern const char *bmxtype[];

/*
 * ======================================================================================================================
 *  HTU21D-F - I2C - Humidity & Temp Sensor
 * ======================================================================================================================
 */
extern Adafruit_HTU21DF htu;
extern bool HTU21DF_exists;

/*
 * ======================================================================================================================
 *  MCP9808 - I2C - Temperature sensor
 * 
 * I2C Address is:  0011,A2,A1,A0
 *                  0011000 = 0x18  where A2,1,0 = 0 MCP9808_I2CADDR_DEFAULT  
 *                  0011001 = 0x19  where A0 = 1
 *                  0011001 = 0x1A  where A1 = 1
 *                  0011011 = 0x1B  where A0 & A1 = 1
 * ======================================================================================================================
 */
#define MCP_ADDRESS_1     0x18        // Default
#define MCP_ADDRESS_2     0x19        // A0 set high, VDD
#define MCP_ADDRESS_3     0x1A        // A1 set high, VDD
#define MCP_ADDRESS_4     0x1B        // A0 & A1 set high, VDD

extern Adafruit_MCP9808 mcp1;
extern Adafruit_MCP9808 mcp2;
extern Adafruit_MCP9808 mcp3;
extern Adafruit_MCP9808 mcp4;
extern bool MCP_1_exists;
extern bool MCP_2_exists;
extern bool MCP_3_exists;
extern bool MCP_4_exists;

/*
 * ======================================================================================================================
 *  SHTX - I2C - Temperature & Humidity sensor (SHT31)  - Note the SHT40, SHT45 use same i2c address
 * ======================================================================================================================
 */
#define SHT_ADDRESS_1     0x44
#define SHT_ADDRESS_2     0x45        // ADR pin set high, VDD

extern Adafruit_SHT31 sht1;
extern Adafruit_SHT31 sht2;
extern bool SHT_1_exists;
extern bool SHT_2_exists;

/*
 * ======================================================================================================================
 *  HIH8 - I2C - Temperature & Humidity sensor (HIH8000)  - 
 * ======================================================================================================================
 */
#define HIH8000_ADDRESS   0x27
extern bool HIH8_exists;

/*
 * ======================================================================================================================
 *  Wet Bulb Temperature - Derived from Temperature and Humidity Sensors
 * ======================================================================================================================
 */
extern bool WBT_exists;

/*
 * ======================================================================================================================
 *  Heat Index Temperature - Derived from Temperature and Humidity Sensors
 * ======================================================================================================================
 */
extern bool HI_exists;

/*
 * ======================================================================================================================
 *  Wet Bulb Globe Temperature - Derived from Temperature and Humidity Sensonrs
 * ======================================================================================================================
 */
extern bool WBGT_exists;

/*
 * ======================================================================================================================
 *  Si1145 - I2C - UV/IR/Visible Light Sensor
 *  The SI1145 has a fixed I2C address (0x60), you can only connect one sensor per microcontroller!
 * ======================================================================================================================
 */
extern Adafruit_SI1145 uv;
extern bool SI1145_exists;
extern float si_last_vis;
extern float si_last_ir;
extern float si_last_uv;

/*
 * ======================================================================================================================
 *  VEML7700 - I2C - Lux Sensor
 * ======================================================================================================================
 */
#define VEML7700_ADDRESS   0x10
extern Adafruit_VEML7700 veml;
extern bool VEML7700_exists;

/*
 * ======================================================================================================================
 *  B_LUX_V30B - I2C - Lux Sensor
 * ======================================================================================================================
 */
#define BLX_ADDRESS   0x4A
extern bool BLX_exists;

/*
 * ======================================================================================================================
 *  PM25AQI - I2C - Air Quality Sensor
 * 
 *  PM25_AQI_Data variables returned are unsigned integers
 *    pm10_standard     < Standard Particle PM1.0 concentration unit µg 𝑚3
 *    pm25_standard     < Standard Particle PM2.5 concentration unit µg 𝑚3
 *    pm100_standard    < Standard Particle PM10.0 concentration unit µg 𝑚3
 *    pm10_env          < Atmospheric Environmental PM1.0 concentration unit µg 𝑚3
 *    pm25_env          < Atmospheric Environmental PM2.5 concentration unit µg 𝑚3
 *    pm100_env         < Atmospheric Environmental PM10.0 concentration unit µg 𝑚3
 *    particles_03um    < Particles with diameter beyond 0.3 µ 𝑚 in 0.1L of air
 *    particles_05um    < Particles with diameter beyond 0.5 µ 𝑚 in 0.1L of air
 *    particles_10um    < Particles with diameter beyond 1.0 µ 𝑚 in 0.1L of air
 *    particles_25um    < Particles with diameter beyond 2.5 µ 𝑚 in 0.1L of air
 *    particles_50um    < Particles with diameter beyond 5.0 µ 𝑚 in 0.1L of air
 *    particles_100um   < Particles with diameter beyond 10.0 µ 𝑚 in 0.1L of air
 * 
 *  pms = Particulate Matter Standard
 *  pme = Particulate Matter Environmental
 * 
 *  Variable Tags for what we monitor and report on
 *    pm1s10
 *    pm1s25
 *    pm1s100
 *    pm1e10
 *    pm1e25
 *    pm1e100
 * 
 * How the sensor works internally
 * Mainly output is the quality and number of each particles with different size per unit volume, the unit volume of 
 * particle number is 0.1L and the unit of mass concentration is μg/m³
 * Internal sampling is divided into two sub modes: stable mode and fast mode. If the concentration change is small 
 * the sensor would run at stable mode with the real interval of 2.3s. And if the change is big the sensor would be 
 * changed to fast mode automatically with the interval of 200~800ms, the higher of the concentration, the shorter of
 * the interval. 
 * 
 * We sample the sensor every second keeping highest 1 second returned values for the observation interval. 
 * ======================================================================================================================
 */
#define PM25AQI_PIN 5  // D5
#define PM25AQI_ADDRESS   0x12

typedef struct{
  int32_t max_s10, max_s25, max_s100; 
  int32_t max_e10, max_e25, max_e100;
  int count=0;
  int fail_count=0;
} PM25AQI_OBS_STR;

extern PM25AQI_OBS_STR pm25aqi_obs;
extern Adafruit_PM25AQI pmaq;
extern bool PM25AQI_exists;

/*
 * ======================================================================================================================
 *  HDC302x - I2C - Precision Temperature & Humidity Sensor
 *    Note HDC uses the same I2C Address as SHT. To avoid conflict we are using 0x46 as hdc1 and 0x47 and hdc2 
 *    manufacturerID = 0x3000  -- uint16_t Adafruit_HDC302x::readManufacturerID()
 * ======================================================================================================================
 */
#define HDC_ADDRESS_1     0x46        // A1=1, A0=0  Need to solder jumper
#define HDC_ADDRESS_2     0x47        // A1=1, A0=1  Need to solder jumper
#define HDC_ADDRESS_3     0x44        // A1=0, A0=0  Not used, Default setting from vendor
#define HDC_ADDRESS_4     0x45        // A1=0, A0=1  Not used

extern Adafruit_HDC302x hdc1;
extern Adafruit_HDC302x hdc2;
extern bool HDC_1_exists;
extern bool HDC_2_exists;

/*
 * ======================================================================================================================
 *  LPS35HW - I2C - Pressure and Temperature
 *    Chip ID = 0xB1,  Library init checks this.
 * ======================================================================================================================
 */
#define LPS_ADDRESS_1     0x5D        // Default
#define LPS_ADDRESS_2     0x5C        // With jumper

extern Adafruit_LPS35HW lps1;
extern Adafruit_LPS35HW lps2;
extern bool LPS_1_exists;
extern bool LPS_2_exists;

/*
 * ======================================================================================================================
 *  Tinovi Leaf Wetness
 *    Chip ID = 0x61,  Library init checks this.
 * ======================================================================================================================
 */
#define TLW_ADDRESS     0x61

extern LeafSens tlw;
extern bool TLW_exists;

/*
 * ======================================================================================================================
 *  Tinovi MultiLevel Soil Moisture (4 Soil and 2 Temperature)
 *    Chip ID = 0x63,  Library init checks this.
 * ======================================================================================================================
 */
#define TSM_ADDRESS     0x63

extern SVCS3 tsm;
extern bool TSM_exists;

/*
 * ======================================================================================================================
 *  Tinovi MultiLevel Soil Moisture (4 Soil and 2 Temperature)
 *    Chip ID = 0x63,  Library init checks this.
 * ======================================================================================================================
 */
#define TMSM_ADDRESS    0x65

extern SVMULTI tmsm;
extern bool TMSM_exists;

/*
 * ======================================================================================================================
 *  Prototype Functions
 * ======================================================================================================================
 */
byte get_Bosch_ChipID (byte address);
void bmx_initialize();
void htu21d_initialize();
void mcp9808_initialize();
void sht_initialize();
void hih8_initialize();
bool hih8_getTempHumid(float *t, float *h);
void wbt_initialize();
double wbt_calculate(double T, double RH);
void hi_initialize();
float hi_calculate(float T, float RH);
void wbgt_initialize();
double wbgt_using_hi(double HIc);
double wbgt_using_wbt(double Ta, double Tg, double Tw);
void si1145_initialize();
void vlx_initialize();
bool blx_getconfig();
void blx_initialize();
float blx_takereading();
void pm25aqi_clear();
void pm25aqi_initialize();
void hdc_initialize();
void lps_initialize();
void tlw_initialize();
void tsm_initialize();
void tmsm_initialize();
void I2C_Check_Sensors();

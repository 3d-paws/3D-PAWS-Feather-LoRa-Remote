/*
 * ======================================================================================================================
 *  sensors.cpp - I2C based sensors
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/ssbits.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/sensors.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  BMX280 humidity - I2C - Temperature, pressure sensor & altitude - Support 2 of any combination
 * ======================================================================================================================
 */
Adafruit_BMP280 bmp1;
Adafruit_BMP280 bmp2;
Adafruit_BME280 bme1;
Adafruit_BME280 bme2;
Adafruit_BMP3XX bm31;
Adafruit_BMP3XX bm32;
byte BMX_1_chip_id = 0x00;
byte BMX_2_chip_id = 0x00;
bool BMX_1_exists = false;
bool BMX_2_exists = false;
byte BMX_1_type=BMX_TYPE_UNKNOWN;
byte BMX_2_type=BMX_TYPE_UNKNOWN;
const char *bmxtype[] = {"UNKN", "BMP280", "BME280", "BMP388", "BMP390"};

/*
 * ======================================================================================================================
 *  HTU21D-F - I2C - Humidity & Temp Sensor
 * ======================================================================================================================
 */
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
bool HTU21DF_exists = false;

/*
 * ======================================================================================================================
 *  MCP9808 - I2C - Temperature sensor
 * ======================================================================================================================  
 */
Adafruit_MCP9808 mcp1;
Adafruit_MCP9808 mcp2;
Adafruit_MCP9808 mcp3;
Adafruit_MCP9808 mcp4;
bool MCP_1_exists = false;
bool MCP_2_exists = false;
bool MCP_3_exists = false;
bool MCP_4_exists = false;

/*
 * ======================================================================================================================
 *  SHTX - I2C - Temperature & Humidity sensor (SHT31)  - Note the SHT40, SHT45 use same i2c address
 * ======================================================================================================================
 */
Adafruit_SHT31 sht1;
Adafruit_SHT31 sht2;
bool SHT_1_exists = false;
bool SHT_2_exists = false;

/*
 * ======================================================================================================================
 *  HIH8 - I2C - Temperature & Humidity sensor (HIH8000)  -
 * ======================================================================================================================
 */
bool HIH8_exists = false;

/*
 * ======================================================================================================================
 *  Wet Bulb Temperature - Derived from Temperature and Humidity Sensors
 * ======================================================================================================================
 */
bool WBT_exists = false;

/*
 * ======================================================================================================================
 *  Heat Index Temperature - Derived from Temperature and Humidity Sensors
 * ======================================================================================================================
 */
bool HI_exists = false;

/*
 * ======================================================================================================================
 *  Wet Bulb Globe Temperature - Derived from Temperature and Humidity Sensonrs
 * ======================================================================================================================
 */
bool WBGT_exists = false;

/*
 * ======================================================================================================================
 *  Si1145 - I2C - UV/IR/Visible Light Sensor
 * ======================================================================================================================
 */
Adafruit_SI1145 uv = Adafruit_SI1145();
bool SI1145_exists = false;
float si_last_vis = 0.0;
float si_last_ir = 0.0;
float si_last_uv = 0.0;

/*
 * ======================================================================================================================
 *  VEML7700 - I2C - Lux Sensor
 * ======================================================================================================================
 */
Adafruit_VEML7700 veml = Adafruit_VEML7700();
bool VEML7700_exists = false;

/*
 * ======================================================================================================================
 *  B_LUX_V30B - I2C - Lux Sensor
 * ======================================================================================================================
 */
bool BLX_exists = false;

/*
 * ======================================================================================================================
 *  PM25AQI - I2C - Air Quality Sensor
 * ======================================================================================================================
 */
PM25AQI_OBS_STR pm25aqi_obs;
Adafruit_PM25AQI pmaq = Adafruit_PM25AQI();
bool PM25AQI_exists = false;

/*
 * ======================================================================================================================
 *  HDC302x - I2C - Precision Temperature & Humidity Sensor
 * ======================================================================================================================
 */
Adafruit_HDC302x hdc1;
Adafruit_HDC302x hdc2;
bool HDC_1_exists = false;
bool HDC_2_exists = false;

/*
 * ======================================================================================================================
 *  LPS35HW - I2C - Pressure and Temperature
 * ======================================================================================================================
 */
Adafruit_LPS35HW lps1;
Adafruit_LPS35HW lps2;
bool LPS_1_exists = false;
bool LPS_2_exists = false;

/*
 * ======================================================================================================================
 *  Tinovi Leaf Wetness
 *    Chip ID = 0x61,  Library init checks this.
 * ======================================================================================================================
 */
LeafSens tlw;
bool TLW_exists = false;

/*
 * ======================================================================================================================
 *  Tinovi MultiLevel Soil Moisture (4 Soil and 2 Temperature)
 * ======================================================================================================================
 */
SVCS3 tsm;
bool TSM_exists = false;

/*
 * ======================================================================================================================
 *  Tinovi MultiLevel Soil Moisture (4 Soil and 2 Temperature)
 * ======================================================================================================================
 */
SVMULTI tmsm;
bool TMSM_exists = false;
 
/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * get_Bosch_ChipID ()  -  Return what Bosch chip is at specified address
 *   Chip ID BMP280 = 0x58 temp, pressure           - I2C ADDRESS 0x77  (SD0 to GND = 0x76)  
 *   Chip ID BME280 = 0x60 temp, pressure, humidity - I2C ADDRESS 0x77  (SD0 to GND = 0x76)  Register 0xE0 = Reset
 *   Chip ID BMP388 = 0x50 temp, pressure           - I2C ADDRESS 0x77  (SD0 to GND = 0x76)
 *   Chip ID BMP390 = 0x60 temp, pressure           - I2C ADDRESS 0x77  (SD0 to GND = 0x76)
 *=======================================================================================================================
 */
byte get_Bosch_ChipID (byte address) {
  byte chip_id = 0;
  byte error;

  Output ("get_Bosch_ChipID()");
  // The i2c_scanner uses the return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.

  // Important! Need to check the 0x00 register first. Doing a 0x0D (not chip id loaction) on a bmp388 
  // will return a value that could match one of the IDs 

  // Check Register 0x00
  sprintf (msgbuf, "  I2C:%02X Reg:%02X", address, 0x00);
  Output (msgbuf);
  Wire.begin();
  Wire.beginTransmission(address);
  Wire.write(0x00);  // BM3 CHIPID REGISTER
  error = Wire.endTransmission();
    //  0:success
    //  1:data too long to fit in transmit buffer
    //  2:received NACK on transmit of address
    //  3:received NACK on transmit of data
    //  4:other error 
  if (error) {
    sprintf (msgbuf, "  ERR_ET:%d", error);
    Output (msgbuf);
  }
  else if (Wire.requestFrom(address, 1)) {  // Returns the number of bytes returned from the slave device 
    chip_id = Wire.read();
    if (chip_id == BMP280_CHIP_ID) { // 0x58
      sprintf (msgbuf, "  CHIPID:%02X BMP280", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!
    }
    else if (chip_id == BMP388_CHIP_ID) {  // 0x50
      sprintf (msgbuf, "  CHIPID:%02X BMP388", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!   
    }
    else if (chip_id == BME280_BMP390_CHIP_ID) {  // 0x60
      sprintf (msgbuf, "  CHIPID:%02X BME/390", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!   
    }
    else {
      sprintf (msgbuf, "  CHIPID:%02X InValid", chip_id);
      Output (msgbuf);      
    }
  }
  else {
    sprintf (msgbuf, "  ERR_RF:0");
    Output (msgbuf);
  }

  // Check Register 0xD0
  chip_id = 0;
  sprintf (msgbuf, "  I2C:%02X Reg:%02X", address, 0xD0);
  Output (msgbuf);
  Wire.begin();
  Wire.beginTransmission(address);
  Wire.write(0xD0);  // BM2 CHIPID REGISTER
  error = Wire.endTransmission();
    //  0:success
    //  1:data too long to fit in transmit buffer
    //  2:received NACK on transmit of address
    //  3:received NACK on transmit of data
    //  4:other error 
  if (error) {
    sprintf (msgbuf, "  ERR_ET:%d", error);
    Output (msgbuf);
  }
  else if (Wire.requestFrom(address, 1)) {  // Returns the number of bytes returned from the slave device 
    chip_id = Wire.read(); 
    if (chip_id == BMP280_CHIP_ID) { // 0x58
      sprintf (msgbuf, "  CHIPID:%02X BMP280", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!
    }
    else if (chip_id == BMP388_CHIP_ID) {  // 0x50
      sprintf (msgbuf, "  CHIPID:%02X BMP388", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!   
    }
    else if (chip_id == BME280_BMP390_CHIP_ID) {  // 0x60
      sprintf (msgbuf, "  CHIPID:%02X BME/390", chip_id);
      Output (msgbuf);
      return (chip_id); // Found a Sensor!   
    }
    else {
      sprintf (msgbuf, "  CHIPID:%02X InValid", chip_id);
      Output (msgbuf);   
    }
  }
  else {
    sprintf (msgbuf, "  ERR_RF:0");
    Output (msgbuf);
  }
  return(0);
}

/* 
 *=======================================================================================================================
 * bmx_initialize() - Bosch sensor initialize
 *=======================================================================================================================
 */
void bmx_initialize() {
  Output("BMX:INIT");
  
  // 1st Bosch Sensor - Need to see which (BMP, BME, BM3) is plugged in
  BMX_1_chip_id = get_Bosch_ChipID(BMX_ADDRESS_1);

  switch (BMX_1_chip_id) {
    case BMP280_CHIP_ID :
      if (!bmp1.begin(BMX_ADDRESS_1)) { 
        msgp = (char *) "BMP1 ERR";
        BMX_1_exists = false;
        SystemStatusBits |= SSB_BMX_1;  // Turn On Bit          
      }
      else {
        BMX_1_exists = true;
        BMX_1_type = BMX_TYPE_BMP280;
        msgp = (char *) "BMP1 OK";
        [[maybe_unused]] float p = bmp1.readPressure();
      }
    break;

    case BME280_BMP390_CHIP_ID :
      if (!bme1.begin(BMX_ADDRESS_1)) { 
        if (!bm31.begin_I2C(BMX_ADDRESS_1)) {  // Perhaps it is a BMP390
          msgp = (char *) "BMX1 ERR";
          BMX_1_exists = false;
          SystemStatusBits |= SSB_BMX_1;  // Turn On Bit          
        }
        else {
          BMX_1_exists = true;
          BMX_1_type = BMX_TYPE_BMP390;
          msgp = (char *) "BMP390_1 OK"; 
          [[maybe_unused]] float p = bm31.readPressure();       
        }      
      }
      else {
        BMX_1_exists = true;
        BMX_1_type = BMX_TYPE_BME280;
        msgp = (char *) "BME280_1 OK";
        [[maybe_unused]] float p = bme1.readPressure();
      }
    break;

    case BMP388_CHIP_ID :
      if (!bm31.begin_I2C(BMX_ADDRESS_1)) { 
        msgp = (char *) "BM31 ERR";
        BMX_1_exists = false;
        SystemStatusBits |= SSB_BMX_1;  // Turn On Bit          
      }
      else {
        BMX_1_exists = true;
        BMX_1_type = BMX_TYPE_BMP388;
        msgp = (char *) "BM31 OK";
        [[maybe_unused]] float p = bm31.readPressure();
      }
    break;

    default:
      msgp = (char *) "BMX_1 NF";
    break;
  }
  Output (msgp);

  // 2nd Bosch Sensor - Need to see which (BMP, BME, BM3) is plugged in
  BMX_2_chip_id = get_Bosch_ChipID(BMX_ADDRESS_2);
  switch (BMX_2_chip_id) {
    case BMP280_CHIP_ID :
      if (!bmp2.begin(BMX_ADDRESS_2)) { 
        msgp = (char *) "BMP2 ERR";
        BMX_2_exists = false;
        SystemStatusBits |= SSB_BMX_2;  // Turn On Bit          
      }
      else {
        BMX_2_exists = true;
        BMX_2_type = BMX_TYPE_BMP280;
        msgp = (char *) "BMP2 OK";
        [[maybe_unused]] float p = bmp2.readPressure();
      }
    break;

    case BME280_BMP390_CHIP_ID :
      if (!bme2.begin(BMX_ADDRESS_2)) { 
        if (!bm32.begin_I2C(BMX_ADDRESS_2)) {  // Perhaps it is a BMP390
          msgp = (char *) "BMX2 ERR";
          BMX_2_exists = false;
          SystemStatusBits |= SSB_BMX_2;  // Turn On Bit          
        }
        else {
          BMX_2_exists = true;
          BMX_2_type = BMX_TYPE_BMP390;
          msgp = (char *) "BMP390_2 OK"; 
          [[maybe_unused]] float p = bm32.readPressure();         
        }
      }
      else {
        BMX_2_exists = true;
        BMX_2_type = BMX_TYPE_BME280;
        msgp = (char *) "BME280_2 OK";
        [[maybe_unused]] float p = bme2.readPressure();
      }
    break;

    case BMP388_CHIP_ID :
      if (!bm32.begin_I2C(BMX_ADDRESS_2)) { 
        msgp = (char *) "BM32 ERR";
        BMX_2_exists = false;
        SystemStatusBits |= SSB_BMX_2;  // Turn On Bit          
      }
      else {
        BMX_2_exists = true;
        BMX_2_type = BMX_TYPE_BMP388;
        msgp = (char *) "BM32 OK";
        [[maybe_unused]] float p = bm32.readPressure();
      }
    break;

    default:
      msgp = (char *) "BMX_2 NF";
    break;
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * htu21d_initialize() - HTU21D sensor initialize
 *=======================================================================================================================
 */
void htu21d_initialize() {
  Output("HTU21D:INIT");
  
  // HTU21DF Humidity & Temp Sensor (I2C ADDRESS = 0x40)
  if (!htu.begin()) {
    msgp = (char *) "HTU NF";
    HTU21DF_exists = false;
    SystemStatusBits |= SSB_HTU21DF;  // Turn On Bit
  }
  else {
    HTU21DF_exists = true;
    msgp = (char *) "HTU OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * mcp9808_initialize() - MCP9808 sensor initialize
 *=======================================================================================================================
 */
void mcp9808_initialize() {
  Output("MCP9808:INIT");
  
  // 1st MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x18)
  mcp1 = Adafruit_MCP9808();
  if (!mcp1.begin(MCP_ADDRESS_1)) {
    msgp = (char *) "MCP1 NF";
    MCP_1_exists = false;
    SystemStatusBits |= SSB_MCP_1;  // Turn On Bit
  }
  else {
    MCP_1_exists = true;
    msgp = (char *) "MCP1 OK";
  }
  Output (msgp);

  // 2nd MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x19)
  mcp2 = Adafruit_MCP9808();
  if (!mcp2.begin(MCP_ADDRESS_2)) {
    msgp = (char *) "MCP2 NF";
    MCP_2_exists = false;
    SystemStatusBits |= SSB_MCP_2;  // Turn On Bit
  }
  else {
    MCP_2_exists = true;
    msgp = (char *) "MCP2 OK";
  }
  Output (msgp);

  // 3rd MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x20)
  mcp3 = Adafruit_MCP9808();
  if (!mcp3.begin(MCP_ADDRESS_3)) {
    msgp = (char *) "MCP3 NF";
    MCP_3_exists = false;
    SystemStatusBits |= SSB_MCP_3;  // Turn On Bit
  }
  else {
    MCP_3_exists = true;
    msgp = (char *) "MCP3 OK";
  }
  Output (msgp);

  // 4rd MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x21)
  mcp4 = Adafruit_MCP9808();
  if (!mcp4.begin(MCP_ADDRESS_4)) {
    msgp = (char *) "MCP4 NF";
    MCP_4_exists = false;
    // SystemStatusBits |= SSB_MCP_4;  // Turn On Bit
  }
  else {
    MCP_4_exists = true;
    msgp = (char *) "MCP4 OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * sht_initialize() - SHT31 sensor initialize
 *=======================================================================================================================
 */
void sht_initialize() {
  Output("SHT:INIT");
  
  // 1st SHT31 I2C Temperature/Humidity Sensor (I2C ADDRESS = 0x44)
  sht1 = Adafruit_SHT31();
  if (!sht1.begin(SHT_ADDRESS_1)) {
    msgp = (char *) "SHT1 NF";
    SHT_1_exists = false;
    SystemStatusBits |= SSB_SHT_1;  // Turn On Bit
  }
  else {
    SHT_1_exists = true;
    msgp = (char *) "SHT1 OK";
  }
  Output (msgp);

  // 2nd SHT31 I2C Temperature/Humidity Sensor (I2C ADDRESS = 0x45)
  sht2 = Adafruit_SHT31();
  if (!sht2.begin(SHT_ADDRESS_2)) {
    msgp = (char *) "SHT2 NF";
    SHT_2_exists = false;
    SystemStatusBits |= SSB_SHT_2;  // Turn On Bit
  }
  else {
    SHT_2_exists = true;
    msgp = (char *) "SHT2 OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * hih8_initialize() - HIH8000 sensor initialize
 *=======================================================================================================================
 */
void hih8_initialize() {
  Output("HIH8:INIT");

  if (I2C_Device_Exist(HIH8000_ADDRESS)) {
    HIH8_exists = true;
    msgp = (char *) "HIH8 OK";
  }
  else {
    msgp = (char *) "HIH8 NF";
    HIH8_exists = false;
    SystemStatusBits |= SSB_HIH8;  // Turn On Bit
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * hih8_getTempHumid() - Get Temp and Humidity
 *   Call example:  status = hih8_getTempHumid(&t, &h);
 *=======================================================================================================================
 */
bool hih8_getTempHumid(float *t, float *h) {
  if (HIH8_exists) {
    uint16_t humidityBuffer    = 0;
    uint16_t temperatureBuffer = 0;
  
    Wire.begin();
    Wire.beginTransmission(HIH8000_ADDRESS);

    Wire.write(0x00); // set the register location for read request

    delayMicroseconds(200); // give some time for sensor to process request

    if (Wire.requestFrom(HIH8000_ADDRESS, 4) == 4) {

      // Get raw humidity data
      humidityBuffer = Wire.read();
      humidityBuffer <<= 8;
      humidityBuffer |= Wire.read();
      humidityBuffer &= 0x3FFF;   // 14bit value, get rid of the upper 2 status bits

      // Get raw temperature data
      temperatureBuffer = Wire.read();
      temperatureBuffer <<= 8;
      temperatureBuffer |= Wire.read();
      temperatureBuffer >>= 2;  // Remove the last two "Do Not Care" bits (shift left is same as divide by 4)

      Wire.endTransmission();

      *h = humidityBuffer * 6.10e-3;
      *t = temperatureBuffer * 1.007e-2 - 40.0;

      // QC Check
      *h = (isnan(*h) || (*h < QC_MIN_RH) || (*h >QC_MAX_RH)) ? QC_ERR_RH : *h;
      *t = (isnan(*t) || (*t < QC_MIN_T)  || (*t >QC_MAX_T))  ? QC_ERR_T  : *t;
      return (true);
    }
    else {
      Wire.endTransmission();
      return(false);
    }
  }
  else {
    return (false);
  }
}

/* 
 *=======================================================================================================================
 * wbt_initialize() - Wet Bulb Temperature
 *=======================================================================================================================
 */
void wbt_initialize() {
  Output("WBT:INIT");
  if (MCP_1_exists && SHT_1_exists) {
    WBT_exists = true;
    Output ("WBT:OK");
  }
  else {
    Output ("WBT:NF");
  }
}

/* 
 *=======================================================================================================================
 * wbt_calculate() - Compute Web Bulb Temperature
 * 
 * By definition, wet-bulb temperature is the lowest temperature a portion of air can acquire by evaporative 
 * cooling only. When air is at its maximum (100 %) humidity, the wet-bulb temperature is equal to the normal 
 * air temperature (dry-bulb temperature). As the humidity decreases, the wet-bulb temperature becomes lower 
 * than the normal air temperature. Forecasters use wet-bulb temperature to predict rain, snow, or freezing rain.
 * 
 * SEE https://journals.ametsoc.org/view/journals/apme/50/11/jamc-d-11-0143.1.xml
 * SEE https://www.omnicalculator.com/physics/wet-bulb
 * 
 * Tw = T * atan[0.151977(RH + 8.3,3659)^1/2] + atan(T + RH%) - atan(RH - 1.676311)  + 0.00391838(RH)^3/2 * atan(0.023101 * RH%) - 4.686035
 * 
 * [ ] square bracket denote grouping for order of operations. 
 *     In Arduino code, square brackets are not used for mathematical operations. Instead, parentheses ( ).
 * sqrt(x) computes the square root of x, which is x to the 1/2.
 * pow(RH, 1.5) calculates RH to the 3/2, which is the relative humidity raised to the power of 1.5.
 *=======================================================================================================================
 */
double wbt_calculate(double T, double RH) {
  if ((T == -999.9) || (RH == -999.9)) {
    return (-999.9);
  }

  // Output("WBT:CALC");

  // Equation components
  double term1 = T * atan(0.151977 * sqrt(RH + 8.313659));
  double term2 = atan(T + RH);
  double term3 = atan(RH - 1.676311);
  double term4 = 0.00391838 * pow(RH, 1.5) * atan(0.023101 * RH);
  double constant = 4.686035;

  // Wet bulb temperature calculation
  double Tw = term1 + term2 - term3 + term4 - constant;

  Tw = (isnan(Tw) || (Tw < QC_MIN_T)  || (Tw >QC_MAX_T))  ? QC_ERR_T  : Tw;
  return (Tw);
}

/* 
 *=======================================================================================================================
 * hi_initialize() - Heat Index Temperature
 *=======================================================================================================================
 */
void hi_initialize() {
  Output("HI:INIT");
  if (SHT_1_exists) {
    HI_exists = true;
    Output ("HI:OK");
  }
  else {
    Output ("HI:NF");
  }
}

/* 
 *=======================================================================================================================
 * hi_calculate() - Compute Heat Index Temperature Returns Celsius
 * 
 * SEE https://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
 * 
 * The regression equation of Rothfusz is:
 * HI = -42.379 + 2.04901523*T + 10.14333127*RH - .22475541*T*RH - .00683783*T*T - .05481717*RH*RH + .00122874*T*T*RH + 
 *      .00085282*T*RH*RH - .00000199*T*T*RH*RH
 * 
 * The Rothfusz regression is not appropriate when conditions of temperature and humidity 
 * warrant a heat index value below about 80 degrees F. In those cases, a simpler formula 
 * is applied to calculate values consistent with Steadman's results:
 * HI = 0.5 * {T + 61.0 + [(T-68.0)*1.2] + (RH*0.094)} 
 *=======================================================================================================================
 */
float hi_calculate(float T, float RH) {
  float HI;
  float HI_f;

  if ((T == -999.9) || (RH == -999.9)) {
    return (-999.9);
  }

  // Convert temperature from Celsius to Fahrenheit
  float T_f = T * 9.0 / 5.0 + 32.0;

  // Steadman's equation
  HI_f = 0.5 * (T_f + 61.0 + ((T_f - 68.0)*1.2) + (RH * 0.094));

  // Compute the average of the simple HI with the actual temperature [deg F]
  HI_f = (HI_f + T_f) / 2;

  if (HI_f >= 80.0) { 
    // Use Rothfusz's equation
    
    // Constants for the Heat Index formula
    float c1 = -42.379;
    float c2 = 2.04901523;
    float c3 = 10.14333127;
    float c4 = -0.22475541;
    float c5 = -0.00683783;
    float c6 = -0.05481717;
    float c7 = 0.00122874;
    float c8 = 0.00085282;
    float c9 = -0.00000199;
    
    // Heat Index calculation
    HI_f = c1 + (c2 * T_f) + (c3 * RH) + (c4 * T_f * RH) +
                (c5 * T_f * T_f) + (c6 * RH * RH) + 
                (c7 * T_f * T_f * RH) + (c8 * T_f * RH * RH) +
                (c9 * T_f * T_f * RH * RH);

    if ((RH < 13.0) && ((T_f > 80.0) && (T_f < 112.0)) ) {
      // If the RH is less than 13% and the temperature is between 80 and 112 degrees F, 
      // then the following adjustment is subtracted from HI: 
      // ADJUSTMENT = [(13-RH)/4]*SQRT{[17-ABS(T-95.)]/17}

      float Adjustment = ( (13 - RH) / 4 ) * sqrt( (17 - abs(T_f - 95.0) ) / 17 );

      HI_f = HI_f - Adjustment;

    }
    else if ((RH > 85.0) && ((T_f > 80.0) && (T_f < 87.0)) ) {
      // If the RH is greater than 85% and the temperature is between 80 and 87 degrees F, 
      // then the following adjustment is added to HI: 
      // ADJUSTMENT = [(RH-85)/10] * [(87-T)/5]

      float Adjustment = ( (RH - 85) / 10 ) * ( (87.0 - T_f) / 5 );

      HI_f = HI_f + Adjustment;
    }
  }

  // Convert Heat Index from Fahrenheit to Celsius
  HI = (HI_f - 32.0) * 5.0 / 9.0;

  // Quality Control Check
  HI = (isnan(HI) || (HI < QC_MIN_HI)  || (HI >QC_MAX_HI))  ? QC_ERR_HI  : HI;

  return (HI);
}

/* 
 *=======================================================================================================================
 * wbgt_initialize() - Wet Bulb Globe Temperature
 *=======================================================================================================================
 */
void wbgt_initialize() {
  Output("WBGT:INIT");
  if (SHT_1_exists) {
    WBGT_exists = true;
    if (MCP_3_exists) {
      Output ("WBGT:OK w/Globe");
    }
    else {
      Output ("WBGT:OK wo/Globe");
    }
  }
  else {
    Output ("WBGT:NF");
  }
}

/* 
 *=======================================================================================================================
 * wbgt_using_hi() - Compute Web Bulb Globe Temperature using Heat Index
 *=======================================================================================================================
 */
double wbgt_using_hi(double HIc) {

  if (HIc == -999.9) {
    return (-999.9);
  }

  double HIf = HIc * 9.0 / 5.0 + 32.0;

  // Below produces Wet Bulb Globe Temperature in Celsius
  double TWc = -0.0034 * pow(HIf, 2) + 0.96 * HIf - 34;

  TWc = (isnan(TWc) || (TWc < QC_MIN_T)  || (TWc >QC_MAX_T))  ? QC_ERR_T  : TWc;
  return (TWc);
}

/* 
 *=======================================================================================================================
 * wbgt_using_wbt() - Compute Web Bulb Globe Temperature using web bulb temperature
 *=======================================================================================================================
 */
double wbgt_using_wbt(double Ta, double Tg, double Tw) {
  // Ta = mcp1 temp
  // Tg = mcp3 temp
  // Tw = wbt_calculate(Ta, RH)

  double wbgt = (0.7 * Tw) + (0.2 * Tg) + (0.1 * Ta);  // This will be Celsius

  wbgt = (isnan(wbgt) || (wbgt < QC_MIN_T)  || (wbgt >QC_MAX_T))  ? QC_ERR_T  : wbgt;

  return (wbgt);
}

/* 
 *=======================================================================================================================
 * si1145_initialize() - SI1145 sensor initialize
 *=======================================================================================================================
 */
void si1145_initialize() {
  Output("SI1145:INIT");
  
  // SSB_SI1145 UV index & IR & Visible Sensor (I2C ADDRESS = 0x60)
  if (! uv.begin(&Wire)) {
    Output ("SI:NF");
    SI1145_exists = false;
    SystemStatusBits |= SSB_SI1145;  // Turn On Bit
  }
  else {
    SI1145_exists = true;
    Output ("SI:OK");
    si_last_vis = uv.readVisible();
    si_last_ir = uv.readIR();
    si_last_uv = uv.readUV()/100.0;

    sprintf (msgbuf, "SI:VI[%d.%02d]", (int)si_last_vis, (int)(si_last_vis*100.0)%100); 
    Output (msgbuf);
    sprintf (msgbuf, "SI:IR[%d.%02d]", (int)si_last_ir, (int)(si_last_ir*100.0)%100); 
    Output (msgbuf);
    sprintf (msgbuf, "SI:UV[%d.%02d]", (int)si_last_uv, (int)(si_last_uv*100.0)%100); 
    Output (msgbuf);
  }
}

/* 
 *=======================================================================================================================
 * vlx_initialize() - VEML7700 sensor initialize
 * 
 * SEE https://learn.microsoft.com/en-us/windows/win32/sensorsapi/understanding-and-interpreting-lux-values
 * 
 * This data set is for illustration and may not be completely accurate for all users or situations.
 * 
 * Lighting condition     From (lux)     To (lux)     Mean value (lux)     Lighting step
 * Pitch Black            0              10           5                    1
 * Very Dark              11             50           30                   2
 * Dark Indoors           51             200          125                  3
 * Dim Indoors            201            400          300                  4
 * Normal Indoors         401            1000         700                  5
 * Bright Indoors         1001           5000         3000                 6
 * Dim Outdoors           5001           10,000       7500                 7
 * Cloudy Outdoors        10,001         30,000       20,000               8
 * Direct Sunlight        30,001         100,000      65,000               9
 * 
 * From www.vishay.com - Designing the VEML7700 Into an Application
 * 1    lx Full moon overhead at tropical latitudes
 * 3.4  lx Dark limit of civil twilight under a clear sky
 * 50   lx Family living room
 * 80   lx Hallway / bathroom
 * 100  lx Very dark overcast day
 * 320  lx to 500 lx Office lighting
 * 400  lx Sunrise or sunset on a clear day
 * 1000 lx Overcast day; typical TV studio lighting
 * 
 *=======================================================================================================================
 */
void vlx_initialize() {
  Output("VLX:INIT");

  if (veml.begin()) {
    VEML7700_exists = true;
    msgp = (char *) "VLX OK";
  }
  else {
    msgp = (char *) "VLX NF";
    VEML7700_exists = false;
    SystemStatusBits |= SSB_VLX;  // Turn On Bit
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * blx_getconfig() - DFRobot_B_LUX_V30B sensor config - 
 *     Value returned keeps changing same with DF Robot Arduino library, WHY?
 *=======================================================================================================================
 */
/*
bool blx_getconfig() {
  uint8_t data; // Array to hold the 4 bytes of data

  Wire.beginTransmission(BLUX30_ADDRESS);
  Wire.write(0x04); // Point to the config register address
  Wire.endTransmission();  // false tells the I2C master to not release the bus between the write and read operations

  // Request 4 bytes from the device
  // The Wire library automatically handles the toggling of the least significant bit (LSB) of the address for reads.
  Wire.requestFrom(BLUX30_ADDRESS, 1);
  if (Wire.available() == 1) { // Check if 4 bytes were received
    data = Wire.read(); // Read each byte into the array

    sprintf (msgbuf, "BLUX30 CFGREG %0X", data);
    Output (msgbuf);
    return(true);

  } else {
    return (false);
  }
}
*/

/* 
 *=======================================================================================================================
 * blx_initialize() - DFRobot_B_LUX_V30B sensor
 *=======================================================================================================================
 */
void blx_initialize() {
  Output("BLX:INIT");

  if (I2C_Device_Exist(BLX_ADDRESS)) {
    BLX_exists = true;
    msgp = (char *) "BLX:OK";
  }
  else {
    BLX_exists = false;
    msgp = (char *) "BLX:NF";
    SystemStatusBits |= SSB_BLX;  // Turn On Bit
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * blx_takereading() - DFRobot_B_LUX_V30B sensor reading
 *=======================================================================================================================
 */
float blx_takereading() {
  float lux;
  uint32_t raw;
  uint8_t data[4]; // Array to hold the 4 bytes of data
  const unsigned long timeout = 1000; // Timeout in milliseconds
  unsigned long startTime;

  Wire.beginTransmission(BLX_ADDRESS);
  Wire.write(0x00); // Point to the data register address
  Wire.endTransmission(false); // false tells the I2C master to not release the bus between the write and read operations

  // Request 4 bytes from the device
  Wire.requestFrom(BLX_ADDRESS, 4);

  startTime = millis(); // Record the start time
  while (Wire.available() < 4) { // Wait for all bytes to be received
    if (millis() - startTime > timeout) { // Check if timeout has been reached
      return -1; // Return error code if timeout occurs
    }
    delay(1); // Short delay to prevent busy-waiting
  }

  for (int i = 0; i < 4; i++) {
    data[i] = Wire.read(); // Read each byte into the array
  }

  raw = data[3];
  raw = (raw<<8)|data[2];
  raw = (raw<<8)|data[1];
  raw = (raw<<8)|data[0];

  lux = ((float)raw*1.4) / 1000;  // Is 1.4 scaling multiplier based on the sensor's internal calibration ?
                                    // Is divide by 1000 converting from millilux ?

  // sprintf (msgbuf, "BLUX30 LUX %f RAW %lu\n", lux, raw);
  // Output (msgbuf);
  return(lux);
}

/* 
 *=======================================================================================================================
 * pm25aqi_clear() - clear observation
 *=======================================================================================================================
 */
void pm25aqi_clear() {
  pm25aqi_obs.max_s10 = 0;
  pm25aqi_obs.max_s25 = 0;
  pm25aqi_obs.max_s100 = 0;
  pm25aqi_obs.max_e10 = 0;
  pm25aqi_obs.max_e25 = 0;
  pm25aqi_obs.max_e100 = 0;
  pm25aqi_obs.count = 0;
  pm25aqi_obs.fail_count = 0;
}

/* 
 *=======================================================================================================================
 * pm25aqi_initialize() - air quality sensor
 *=======================================================================================================================
 */
void pm25aqi_initialize() {
  Output("PM25AQI:INIT");
  Wire.beginTransmission(PM25AQI_ADDRESS);
  if (Wire.endTransmission()) {
    msgp = (char *) "PM:NF";
    PM25AQI_exists = false;
    SystemStatusBits |= SSB_PM25AQI;  // Turn On Bit
  }
  else {
    if (! pmaq.begin_I2C()) {      // connect to the sensor over I2C
      msgp = (char *) "PM:Begin NF";
      PM25AQI_exists = false;
    }
    else {
      msgp = (char *) "PM:OK";
      PM25AQI_exists = true;
      
      pm25aqi_clear();
    }
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * hdc_initialize() - HDC3002c sensor initialize
 *=======================================================================================================================
 */
void hdc_initialize() {
  Output("HDC:INIT");
  
  // 1st HDC I2C Temperature/Humidity Sensor (I2C ADDRESS = 0x44)
  hdc1 = Adafruit_HDC302x();
  if (!hdc1.begin(HDC_ADDRESS_1, &Wire)) {
    msgp = (char *) "HDC1 NF";
    HDC_1_exists = false;
    SystemStatusBits |= SSB_HDC_1;  // Turn On Bit
  }
  else {
    double t,h;
    hdc1.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0);
    HDC_1_exists = true;
    msgp = (char *) "HDC1 OK";
  }
  Output (msgp);

  // 2nd HDC I2C Temperature/Humidity Sensor (I2C ADDRESS = 0x45)
  hdc2 = Adafruit_HDC302x();
  if (!hdc2.begin(HDC_ADDRESS_2, &Wire)) {
    msgp = (char *) "HDC2 NF";
    HDC_2_exists = false;
    SystemStatusBits |= SSB_HDC_2;  // Turn On Bit
  }
  else {
    double t,h;
    hdc2.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0);
    HDC_2_exists = true;
    msgp = (char *) "HDC2 OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * lps_initialize() - LPS35HW Pressure and Temperature initialize
 *=======================================================================================================================
 */
void lps_initialize() {
  Output("LPS:INIT");
  
  // 1st LPS I2C Pressure/Temperature Sensor (I2C ADDRESS = 0x5D)
  lps1 = Adafruit_LPS35HW();
  if (!lps1.begin_I2C(LPS_ADDRESS_1, &Wire)) {
    msgp = (char *) "LPS1 NF";
    LPS_1_exists = false;
    SystemStatusBits |= SSB_LPS_1;  // Turn On Bit
  }
  else {
    [[maybe_unused]] float t,p;
    t = lps1.readTemperature();
    p = lps1.readPressure();
    LPS_1_exists = true;
    msgp = (char *) "LPS1 OK";
  }
  Output (msgp);

  // 2nd LPS I2C Pressure/Temperature Sensor (I2C ADDRESS = 0x5C)
  lps2 = Adafruit_LPS35HW();
  if (!lps2.begin_I2C(LPS_ADDRESS_2, &Wire)) {
    msgp = (char *) "LPS2 NF";
    LPS_2_exists = false;
    SystemStatusBits |= SSB_LPS_2;  // Turn On Bit
  }
  else {
    [[maybe_unused]] float t,p;
    t = lps2.readTemperature();
    p = lps2.readPressure();
    LPS_2_exists = true;
    msgp = (char *) "LPS2 OK";
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * tlw_initialize() -  Tinovi Leaf Wetness initialize
 *=======================================================================================================================
 */
void tlw_initialize() {
  Output("TLW:INIT");
  
  // Tinovi Leaf Wetness initialize (I2C ADDRESS = 0x61)
  if (!I2C_Device_Exist(TLW_ADDRESS)) { 
    msgp = (char *) "TLW NF";
    TLW_exists = false;
  }
  else {
    tlw.init(TLW_ADDRESS);
    msgp = (char *) "TLW OK";
    TLW_exists = true;
    SystemStatusBits |= SSB_TLW;  // Turn On Bit
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * tsm_initialize() -  Tinovi Soil Moisture initialize
 *=======================================================================================================================
 */
void tsm_initialize() {
  Output("TSM:INIT");
  
  // Tinovi Soil Moisture initialize (I2C ADDRESS = 0x63)
  if (!I2C_Device_Exist(TSM_ADDRESS)) { 
    msgp = (char *) "TSM NF";
    TSM_exists = false;
  }
  else {
    tsm.init(TSM_ADDRESS);
    msgp = (char *) "TSM OK";
    TSM_exists = true;
    SystemStatusBits |= SSB_TSM;  // Turn On Bit
  }
  Output (msgp);
}

/* 
 *=======================================================================================================================
 * tmsm_initialize() -  Tinovi MultiLevel Soil Moisture initialize
 *=======================================================================================================================
 */
void tmsm_initialize() {
  Output("TMSM:INIT");
  
  // Tinovi MultiLevel Soil Moisture initialize (I2C ADDRESS = 0x65)
  if (!I2C_Device_Exist(TMSM_ADDRESS)) { 
    msgp = (char *) "TMSM NF";
    TMSM_exists = false;
  }
  else {
    tmsm.init(TMSM_ADDRESS);
    msgp = (char *) "TMSM OK";
    TMSM_exists = true;
    SystemStatusBits |= SSB_TMSM;  // Turn On Bit
  }
  Output (msgp);
}

/*
 * ======================================================================================================================
 * I2C_Check_Sensors() - See if each I2C sensor responds on the bus and take action accordingly             
 * ======================================================================================================================
 */
void I2C_Check_Sensors() {

  // BMX_1 Barometric Pressure 
  if (I2C_Device_Exist (BMX_ADDRESS_1)) {
    // Sensor online but our state had it offline
    if (BMX_1_exists == false) {
      if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
        if (bmp1.begin(BMX_ADDRESS_1)) { 
          BMX_1_exists = true;
          Output ("BMP1 ONLINE");
          SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
        } 
      }
      else if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
        if (bme1.begin(BMX_ADDRESS_1)) { 
          BMX_1_exists = true;
          Output ("BME1 ONLINE");
          SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
        }          
      }
      else {
        if (bm31.begin_I2C(BMX_ADDRESS_1)) { 
          BMX_1_exists = true;
          Output ("BM31 ONLINE");
          SystemStatusBits &= ~SSB_BMX_1; // Turn Off Bit
        }                  
      }      
    }
  }
  else {
    // Sensor offline but our state has it online
    if (BMX_1_exists == true) {
      BMX_1_exists = false;
      Output ("BMX1 OFFLINE");
      SystemStatusBits |= SSB_BMX_1;  // Turn On Bit 
    }    
  }

  // BMX_2 Barometric Pressure 
  if (I2C_Device_Exist (BMX_ADDRESS_2)) {
    // Sensor online but our state had it offline
    if (BMX_2_exists == false) {
      if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
        if (bmp2.begin(BMX_ADDRESS_2)) { 
          BMX_2_exists = true;
          Output ("BMP2 ONLINE");
          SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
        } 
      }
      else if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
        if (bme2.begin(BMX_ADDRESS_2)) { 
          BMX_2_exists = true;
          Output ("BME2 ONLINE");
          SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
        }          
      }
      else {
         if (bm32.begin_I2C(BMX_ADDRESS_2)) { 
          BMX_2_exists = true;
          Output ("BM32 ONLINE");
          SystemStatusBits &= ~SSB_BMX_2; // Turn Off Bit
        }                         
      }     
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (BMX_2_exists == true) {
      BMX_2_exists = false;
      Output ("BMX2 OFFLINE");
      SystemStatusBits |= SSB_BMX_2;  // Turn On Bit 
    }    
  }

#ifdef NOWAY    // Sensor fails to update if this code is enabled
  // MCP9808 Precision I2C Temperature Sensor
  if (I2C_Device_Exist (MCP_ADDRESS_1)) {
    // Sensor online but our state had it offline
    if (MCP_1_exists == false) {
      // See if we can bring sensor online
      if (mcp1.begin(MCP_ADDRESS_1)) {
        MCP_1_exists = true;
        Output ("MCP ONLINE");
        SystemStatusBits &= ~SSB_MCP_1; // Turn Off Bit
      }
    }
  }
  else {
    // Sensor offline but we our state has it online
    if (MCP_1_exists == true) {
      MCP_1_exists = false;
      Output ("MCP OFFLINE");
      SystemStatusBits |= SSB_MCP_1;  // Turn On Bit
    }   
  }
#endif
}

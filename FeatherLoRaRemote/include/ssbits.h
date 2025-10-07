/*
 * ======================================================================================================================
 *  ssbits.h - System Status Bits Definations  - Sent ast part of the observation as hth (Health)
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * OFF= SSB &= ~SSB_PWRON
 * ON = SSB |= SSB_PWROFF
 * 0  = OK
 * ======================================================================================================================
 */
#define SSB_PWRON           0x1       // Set at power on, but cleared after first observation
#define SSB_SD              0x2       // Set if SD missing at boot or other SD related issues
#define SSB_RTC             0x4       // Set if RTC missing at boot
#define SSB_OLED            0x8       // Set if OLED missing at boot, but cleared after first observation
#define SSB_N2S             0x10      // Set when Need to Send observations exist
#define SSB_FROM_N2S        0x20      // Set in transmitted N2S observation when finally transmitted
#define SSB_EEPROM          0x40      // Set if 24LC32 EEPROM missing

#define SSB_AS5600          0x80      // Set if wind direction sensor AS5600 has issues
#define SSB_BMX_1           0x100     // Set if Barometric Pressure & Altitude Sensor missing
#define SSB_BMX_2           0x200     // Set if Barometric Pressure & Altitude Sensor missing
#define SSB_HTU21DF         0x400     // Set if Humidity & Temp Sensor missing
#define SSB_SI1145          0x800     // Set if UV index & IR & Visible Sensor missing
#define SSB_MCP_1           0x1000    // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_MCP_2           0x2000    // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_MCP_3           0x4000    // Set if MCP9808 I2C Temperature Sensor missing
#define SSB_LORA            0x8000    // Set if LoRa Radio missing at startup
#define SSB_SHT_1           0x10000   // Set if SHTX1 Sensor missing
#define SSB_SHT_2           0x20000   // Set if SHTX2 Sensor missing
#define SSB_HIH8            0x40000   // Set if HIH8000 Sensor missing
#define SSB_VLX             0x80000   // Set if VEML7700 Sensor missing
#define SSB_PM25AQI         0x100000  // Set if PM25AQI Sensor missing
#define SSB_HDC_1           0x200000  // Set if HDC302x I2C Temperature Sensor missing
#define SSB_HDC_2           0x400000  // Set if HDC302x I2C Temperature Sensor missing
#define SSB_BLX             0x800000  // Set if BLUX30 I2C Sensor missing
#define SSB_LPS_1           0x1000000 // Set if LPS35HW I2C Sensor missing
#define SSB_LPS_2           0x2000000 // Set if LPS35HW I2C Sensor missing
#define SSB_TLW             0x4000000 // Set if Tinovi Leaf Wetness I2C Sensor missing
#define SSB_TSM             0x8000000 // Set if Tinovi Soil Moisture I2C Sensor missing
#define SSB_TMSM            0x10000000 // Set if Tinovi MultiLevel Soil Moisture I2C Sensor missing

// Extern variables
extern unsigned int SystemStatusBits;

// Function prototypes
extern void JPO_ClearBits();

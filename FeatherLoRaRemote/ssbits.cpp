/*
 * ======================================================================================================================
 *  ssbits.cpp - System Status Bits Definations  - Sent ast part of the observation as hth (Health)
 * ======================================================================================================================
 */
#include "include/main.h"
#include "include/ssbits.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
unsigned int SystemStatusBits = SSB_PWRON; // Set bit 0 for initial value power on. Bit 0 is cleared after first obs

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */
 
/*
 * ======================================================================================================================
 * JPO_ClearBits() - Clear System Status Bits related to initialization
 * ======================================================================================================================
 */
void JPO_ClearBits() {
  if (JustPoweredOn) {
    JustPoweredOn = false;
    SystemStatusBits &= ~SSB_PWRON;   // Turn Off Power On Bit
    // SystemStatusBits &= ~SB_SD;    // Turn Off SD Missing Bit - Required keep On
    // SystemStatusBits &= ~SSB_RTC;  // Turn Off RTC Missing Bit - Required keep On
    SystemStatusBits &= ~SSB_EEPROM;  // Turn Off EEPROM Missing Bit
    SystemStatusBits &= ~SSB_OLED;    // Turn Off OLED Missing Bit
    SystemStatusBits &= ~SSB_LORA;    // Turn Off LoRa Missing Bit
    SystemStatusBits &= ~SSB_BMX_1;   // Turn Off BMX_1 Not Found Bit
    SystemStatusBits &= ~SSB_BMX_2;   // Turn Off BMX_2 Not Found Bit
    SystemStatusBits &= ~SSB_HTU21DF; // Turn Off HTU Not Found Bit
    SystemStatusBits &= ~SSB_MCP_1;   // Turn Off MCP_1 Not Found Bit
    SystemStatusBits &= ~SSB_MCP_2;   // Turn Off MCP_2 Not Found Bit
    SystemStatusBits &= ~SSB_MCP_3;   // Turn Off MCP_2 Not Found Bit
    SystemStatusBits &= ~SSB_SHT_1;   // Turn Off SHT_1 Not Found Bit
    SystemStatusBits &= ~SSB_SHT_2;   // Turn Off SHT_1 Not Found Bit
    SystemStatusBits &= ~SSB_HIH8;    // Turn Off HIH Not Found Bit
    SystemStatusBits &= ~SSB_VLX;     // Turn Off VEML7700 Not Found Bit
    SystemStatusBits &= ~SSB_SI1145;  // Turn Off UV,IR, VIS Not Found Bit
    SystemStatusBits &= ~SSB_PM25AQI; // Turn Off PM25AQI Not Found Bit
    SystemStatusBits &= ~SSB_HDC_1;   // Turn Off HDC302x Not Found Bit
    SystemStatusBits &= ~SSB_HDC_2;   // Turn Off HDC302x Not Found Bit
    SystemStatusBits &= ~SSB_BLX;     // Turn Off BLUX30 Not Found Bit
    SystemStatusBits &= ~SSB_LPS_1;   // Turn Off LPS35HW Not Found Bit
    SystemStatusBits &= ~SSB_LPS_2;   // Turn Off LPS35HW Not Found Bit
    SystemStatusBits &= ~SSB_TLW;     // Turn Off Tinovi Leaf Wetness Not Found Bit
    SystemStatusBits &= ~SSB_TSM;     // Turn Off Tinovi Soil Moisture Not Found Bit
    SystemStatusBits &= ~SSB_TMSM;    // Turn Off Tinovi MultiLevel Soil Moisture Not Found Bit
  }
}

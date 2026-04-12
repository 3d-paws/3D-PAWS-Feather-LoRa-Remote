/*
 * ======================================================================================================================
 *  dsmux.h - Dallas One Wire Mux Include for DS2482S-800 8 Channel I2C to 1-Wire Bus Adapter
 * ======================================================================================================================
 */
#include <Adafruit_DS248x.h>

/*
  Default address is 0x18. Not using!
  
  The MCP9808 uses the same 8 addresses, though we only used the first 4 of them for mcp1 - mcp4. 

  We will use the highest address @ 0x1F

  The table below shows all possible addresses
  | A2 | A1 | A0 | 7‑bit I²C address |
  | -- | -- | -- | ----------------- |
  | 0  | 0  | 0  | 0x18              |
  | 0  | 0  | 1  | 0x19              |
  | 0  | 1  | 0  | 0x1A              |
  | 0  | 1  | 1  | 0x1B              |
  | 1  | 0  | 0  | 0x1C              | 
  | 1  | 0  | 1  | 0x1D              |
  | 1  | 1  | 0  | 0x1E              |
  | 1  | 1  | 1  | 0x1F              | Using this address

  Example Serial Console power on discovery output with temperature probes on 0,1,4,7 Probe address is informational only. 
  
  DSMUX:INIT
  DSMUX Channel Scan
   dst0=22.25 28:3B:18:79:A2:19:03:77
   dst1=22.56 28:FB:90:18:04:00:00:99
   dst4=22.62 28:64:89:36:04:00:00:E6
   dst7=22.56 28:D0:86:36:04:00:00:52
  DSMUX 4 Found

  Reported in INFO as "dsmux":"0,1,4,7"

  Reported in OBS as "dst0":22.6,"dst1":22.6,"dst4":22.7,"dst7":22.6

*/

#define DSMUX_ADDRESS 0x1F
#define DS248X_CHANNELS 8
#define DS18B20_CMD_SKIP_ROM 0xCC
#define DS18B20_CMD_CONVERT_T 0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

// Extern variables
extern  Adafruit_DS248x ds248x;
extern bool DSMUX_exists;
extern bool dsmux_sensor_exists[DS248X_CHANNELS];

// Function prototypes
void dsmux_initialize();
void dsmux_obs_do(int &sidx);
/*
 * ======================================================================================================================
 *  mux.h - PCA9548 I2C MUX Defincations
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  A MUX enabled sensor can not also be on the main i2c bus.
 *    MUX Support Tinovi Soil Moisture sensors
 * ======================================================================================================================
 */

#define MUX_CHANNELS 8
#define MUX_AQ_CHANNEL 7
#define MAX_CHANNEL_SENSORS 10
#define MUX_ADDR 0x70

typedef enum {
  UNKN, m_bmp, m_bme, m_b38, m_b39, m_htu, m_sht, m_mcp, m_hdc, m_lps, m_hih, m_tlw, m_tsm, m_si 
} SENSOR_TYPE;

typedef enum { 
  OFFLINE,
  ONLINE
} SENSOR_STATE;

typedef struct {
  SENSOR_STATE  state;
  SENSOR_TYPE   type;
  byte          id;
  byte          address;
} CH_SENSOR;

typedef struct {
  bool            inuse;                // Set to true when an observation is stored here         
  CH_SENSOR       sensor[MAX_CHANNEL_SENSORS];
} MULTIPLEXER_STR;

// Extern variables
extern const char *sensor_state[];
extern bool MUX_exists;
extern MULTIPLEXER_STR mux[MUX_CHANNELS];
extern MULTIPLEXER_STR *mc;
extern CH_SENSOR *chs;
extern const char *sensor_type[];

// Function prototypes
void mux_deselect_all();
void mux_channel_set(uint8_t channel);
void mux_obs_do(int &sidx);
void mux_scan();
void mux_initialize();

/*
 * ======================================================================================================================
 *  smt.h - Soil Moaisture (Analog) and Soil Temp (OneWire) Pairs Definations
 * ======================================================================================================================
 */

#define NPROBES           1        // Max Number of Soil Probes
#define PROBES_POWER_PIN  11       // Pin for enambling power to Dallas and Soil Moisture Sensors
#define DS0_PIN           A1

// Extern variables
extern bool  ds_found[NPROBES];
extern float ds_reading[NPROBES];
extern int   sm_reading[NPROBES];
extern int   sm_pn[NPROBES];
extern int   st_pn[NPROBES];

// Function prototypes
void smt_initialize();
void DoSoilReadings();

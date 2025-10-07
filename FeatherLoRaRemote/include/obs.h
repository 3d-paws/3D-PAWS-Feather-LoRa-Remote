/*
 * ======================================================================================================================
 *  obs.h - Observation Definations
 * ====================================================================================================================== 
 */

#define OBSERVATION_INTERVAL 60   // Seconds
#define MAX_SENSORS          64
#define LORA_PAYLOAD         222
#define OBS_HEADER           110
#define OBS_SPACE            112

typedef enum {
  F_OBS, 
  I_OBS, 
  U_OBS
} OBS_TYPE;

typedef struct {
  char          id[12];
  int           type;
  float         f_obs;
  int           i_obs;
  unsigned long u_obs;
  bool          inuse;
} SENSOR;

typedef struct {
  bool            inuse;                // Set to true when an observation is stored here         
  time_t          ts;                   // TimeStamp
  SENSOR          sensor[MAX_SENSORS];
} OBSERVATION_STR;

// Extern variables
extern OBSERVATION_STR obs;

// Function prototypes
void OBS_Clear();
void OBS_Send();
void OBS_Take();
void OBS_Do();

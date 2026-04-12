/*
 * ======================================================================================================================
 *  wrda.h - Wind Rain Distance Air Definations
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  Wind Related Setup
 * 
 *  NOTE: With interrupts tied to the anemometer rotation we are essentually sampling all the time.  
 *        We record the interrupt count, ms duration and wind direction every second.
 *        One revolution of the anemometer results in 2 interrupts. There are 2 magnets on the anemometer.
 * 
 *        Station observations are logged every minute
 *        Wind and Direction are sampled every second producing 60 samples 
 *        The one second wind speed sample are produced from the interrupt count and ms duration.
 *        Wind Observations a 
 *        Reported Observations
 *          Wind Speed = Average of the 60 samples.
 *          Wind Direction = Average of the 60 vectors from Direction and Speed.
 *          Wind Gust = Highest 3 consecutive samples from the 60 samples. The 3 samples are then averaged.
 *          Wind Gust Direction = Average of the 3 Vectors from the Wind Gust samples.
 *          
 * ======================================================================================================================
 */
#define ANEMOMETER_IRQ_PIN  A2
#define WIND_READINGS       60       // One minute of 1s Samples

typedef struct {
  int direction;
  float speed;
} WIND_BUCKETS_STR;

typedef struct {
  WIND_BUCKETS_STR bucket[WIND_READINGS];
  int bucket_idx;
  float gust;
  int gust_direction;
} WIND_STR;

/*
 * ======================================================================================================================
 *  Option Pin Defination Setup
 * ======================================================================================================================
 */
#define OP1_PIN  A4
#define OP2_PIN  A5
#define OP3_PIN  A0
#define OP4_PIN  A1

/*
 * ======================================================================================================================
 *  Pin OP1 State Setup
 * ======================================================================================================================
 */
#define OP1_STATE_NULL       0
#define OP1_STATE_RAW        1
#define OP1_STATE_RAIN       2
#define OP1_STATE_DIST_5M    5
#define OP1_STATE_DIST_10M   10

/*
 * ======================================================================================================================
 *  Pin OP2 State Setup
 * ======================================================================================================================
 */
#define OP2_STATE_NULL       0
#define OP2_STATE_RAW        1
#define OP2_STATE_VOLTAIC    2

/*
 * ======================================================================================================================
 *  Pin OP3 State Setup
 * ======================================================================================================================
 */
#define OP3_STATE_NULL       0
#define OP3_STATE_RAW        1

/*
 * ======================================================================================================================
 *  Pin OP4 State Setup
 * ======================================================================================================================
 */
#define OP4_STATE_NULL       0
#define OP4_STATE_RAW        1


#define RAINGAUGE1_IRQ_PIN  A3
#define RAINGAUGE2_IRQ_PIN  OP1_PIN
#define DISTANCE_GAUGE_PIN  OP1_PIN
#define VOLTAIC_VOLTAGE_PIN OP2_PIN
#define DG_BUCKETS 60


// Extern variables
extern volatile unsigned int anemometer_interrupt_count;
extern unsigned long anemometer_interrupt_stime;

extern volatile unsigned int raingauge1_interrupt_count;
extern unsigned long raingauge1_interrupt_stime;
extern volatile unsigned long raingauge1_interrupt_ltime;

extern volatile unsigned int raingauge2_interrupt_count;
extern unsigned long raingauge2_interrupt_stime;
extern volatile unsigned long raingauge2_interrupt_ltime;

extern unsigned int dg_resolution_adjust;

extern bool AS5600_exists;

// Function prototype
void anemometer_interrupt_handler();
void raingauge1_interrupt_handler();
float raingauge1_sample();
void raingauge2_interrupt_handler();
float raingauge2_sample();

bool RainEnabled();
int Wind_DirectionVector();
float Wind_SpeedAverage();
float Wind_Gust();
int Wind_GustDirection();
void Do_WRDA_Samples();
void as5600_initialize();
float Pin_ReadAvg(int pin);
float VoltaicVoltage(int pin);
float VoltaicPercent(float half_cell_voltage);
void DS_TakeReading();
float DS_Median();




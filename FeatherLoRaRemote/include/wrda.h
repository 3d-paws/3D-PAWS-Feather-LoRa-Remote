/*
 * ======================================================================================================================
 *  wrda.h - Wind Rain Distance Air Definations
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  SAMPLE Buckets
 * ======================================================================================================================  
 */
#define SAMPLES 60

/*
 * ======================================================================================================================
 *  Rain Gauges
 * ======================================================================================================================
 */
#define RAINGAUGE1_IRQ_PIN  A3
#define RAINGAUGE2_IRQ_PIN  A4

/*
 * ======================================================================================================================
 *  Distance Gauges
 * ======================================================================================================================
 */
#define DISTANCE_GAUGE_PIN  A5
#define DG_BUCKETS SAMPLES


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
#define WIND_READINGS       SAMPLES       // One minute of 1s Samples

typedef struct {
  int direction;
  float speed;
} WIND_BUCKETS_STR;

typedef struct {
  WIND_BUCKETS_STR bucket[WIND_READINGS];
  int bucket_idx;
  float gust;
  int gust_direction;
  int sample_count;
} WIND_STR;

// Extern variables
extern volatile unsigned int raingauge1_interrupt_count;
extern uint64_t raingauge1_interrupt_stime;
extern uint64_t raingauge1_interrupt_ltime;

extern volatile unsigned int raingauge2_interrupt_count;
extern uint64_t raingauge2_interrupt_stime;
extern uint64_t raingauge2_interrupt_ltime;

extern bool AS5600_exists;

// Function prototype
void raingauge1_interrupt_handler();
void raingauge2_interrupt_handler();
void anemometer_interrupt_handler();

void as5600_initialize();
void DS_Initialize();

float DistanceGauge_Median();
float Wind_SpeedAverage();
int Wind_DirectionVector();
float Wind_Gust();
int Wind_GustDirection();
void Wind_ClearSampleCount();
void Do_WRDA_Samples();

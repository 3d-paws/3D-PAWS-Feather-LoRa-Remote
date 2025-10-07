/*
 * ======================================================================================================================
 *  support.h - Support Functions Definations
 * ======================================================================================================================
 */

// Measuring Battery - SEE https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/power-management
#define VBATPIN A7

// Extern variables
extern int LED_PIN;
extern const char* pinNames[];
extern char DeviceID[25];

// Function prototypes
float vbat_get();
bool I2C_Device_Exist(byte address);
void Blink(int count, int between);
void FadeOn(unsigned int time,int increament);
void FadeOff(unsigned int time,int decreament);
void mysort(unsigned int a[], int n);
bool isnumeric(char *s);
void GetDeviceID();
void obs_interval_initialize();
int seconds_to_next_obs();

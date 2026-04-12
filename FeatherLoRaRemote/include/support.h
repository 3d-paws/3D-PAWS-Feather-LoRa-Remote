/*
 * ======================================================================================================================
 *  support.h - Support Functions Definations
 * ======================================================================================================================
 */

// Extern variables

// Function prototypes
bool I2C_Device_Exist(uint8_t address);
void Blink(int count, int between);
void FadeOn(unsigned int time,int increament);
void FadeOff(unsigned int time,int decreament);
void mysort(unsigned int a[], int n);
bool isnumeric(char *s);

void obs_interval_initialize();
int seconds_to_next_obs();

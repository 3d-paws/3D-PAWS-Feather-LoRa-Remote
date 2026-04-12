/*
 * ======================================================================================================================
 *  time.h - Time Management Definations
 * ======================================================================================================================
 */
#include <RTClib.h>

#define PCF8523_ADDRESS         0x68       // I2C address for PCF8523 RTC
#define TM_VALID_YEAR_START     2026
#define TM_VALID_YEAR_END       2035

// Extern variables
extern RTC_PCF8523 rtc;
extern DateTime now;
extern unsigned long wakeuptime;
extern char timestamp[32];
extern bool RTC_valid;

// Function prototypes
uint32_t rtc_unixtime();
void rtc_timestamp();
bool rtc_refresh();
void rtc_initialize();
bool rtc_readserial();

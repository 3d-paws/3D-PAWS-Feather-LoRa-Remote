/*
 * ======================================================================================================================
 *  time.h - Time Management Definations
 * ======================================================================================================================
 */
#define PCF8523_ADDRESS 0x68       // I2C address for PCF8523 RTC

// Extern variables
extern RTC_PCF8523 rtc;
extern DateTime now;
extern unsigned long wakeuptime;
extern char timestamp[32];
extern bool RTC_valid;

// Function prototypes
void rtc_timestamp();
void rtc_initialize();
bool rtc_readserial();

/*
 * ======================================================================================================================
 *  time.h - Time Management Definations
 * ======================================================================================================================
 */

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

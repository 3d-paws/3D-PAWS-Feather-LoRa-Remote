/*
 * ======================================================================================================================
 *  main.h - Main Code Definations
 * ======================================================================================================================
 */
#include <Arduino.h>

#define MAX_MSGBUF_SIZE 256
#define RTC_UPDATE_INTERVAL 4                // 4 Hours from boot

// Extern variables
extern char versioninfo[];
extern char msgbuf[MAX_MSGBUF_SIZE];
extern char *msgp;
extern char Buffer32Bytes[32];
extern bool JustPoweredOn;

// Function prototypes
int seconds_to_next_obs();

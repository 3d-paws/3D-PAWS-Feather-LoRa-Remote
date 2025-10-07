/*
 * ======================================================================================================================
 *  main.h - Main Code Definations
 * ======================================================================================================================
 */
#define VERSION_INFO "FLoRaRemote-250925"

#define PCF8523_ADDRESS 0x68       // I2C address for PCF8523
#define MAX_MSGBUF_SIZE 256

// Extern variables
extern char msgbuf[MAX_MSGBUF_SIZE];
extern char *msgp;
extern char Buffer32Bytes[32];
extern bool JustPoweredOn;

// Function prototypes
int seconds_to_next_obs();

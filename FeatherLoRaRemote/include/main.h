/*
 * ======================================================================================================================
 *  main.h - Main Code Definations
 * ======================================================================================================================
 */
#define MAX_MSGBUF_SIZE 256

// Extern variables
extern char versioninfo[];
extern char msgbuf[MAX_MSGBUF_SIZE];
extern char *msgp;
extern char Buffer32Bytes[32];
extern bool JustPoweredOn;

// Function prototypes
int seconds_to_next_obs();

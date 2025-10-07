/*
 * ======================================================================================================================
 *  sdcard.h - SD Card Definations
 * ======================================================================================================================
 */
#define SD_ChipSelect 10    // GPIO 10 is Pin 10 on Feather and D5 on Particle Boron Board

// Extern variables
extern File SD_fp;
extern bool SD_exists;

// Function prototypes
void SD_initialize();
void SD_LogObservation(char *observations);
void SD_ClearRainTotals();

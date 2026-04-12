/*
 * ======================================================================================================================
 *  feather.h - Feather Related Board Functions and Definations
 * ======================================================================================================================
 */
#include <Arduino.h>

/*
 * =======================================================================================================================
 *  Measuring Battery - // Measuring Battery 
 *    SEE https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/power-management
 *  
 *  This voltage will 'float' at 4.2V when no battery is plugged in, due to the lipoly charger output,  so its not a good 
 *  way to detect if a battery is plugged in or not (there is no simple way to detect if a battery is plugged in)
 * =======================================================================================================================
 */

#define VBATPIN      A7
#define LED_PIN      LED_BUILTIN

// Extern variables
extern char DeviceID[17];
extern const char* pinNames[];

// Function prototypes
void SystemReset();
float vbat_get();
void GetDeviceID();
/*
 * ======================================================================================================================
 * feather.cpp - Feather Related Board Functions and Definations
 * ======================================================================================================================
 */
#include "include/feather.h"

const char* pinNames[] = {
  "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
  "D8", "D9", "D10", "D11", "D12", "D13",
  "A0", "A1", "A2", "A3", "A4", "A5"
};
char DeviceID[17];           // A generated ID based on board's 128-bit serial number converted down to 64bits

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 * SystemReset() - reboot the feather
 * ======================================================================================================================
 */
void SystemReset() {
  // Resets the device, just like hitting the reset button or powering down and back up.
  __disable_irq();     // Prevent pending interrupts from interfering.
  NVIC_SystemReset();  // Trigger system reset [web:4]
}

/*
 *=======================================================================================================================
 * vbat_get() -- return battery voltage
 *=======================================================================================================================
 */
float vbat_get() {
  float v = analogRead(VBATPIN);
  v *= 2;    // we divided by 2, so multiply back
  v *= 3.3;  // Multiply by 3.3V, our reference voltage
  v /= 1023.0; // convert to voltage
  return (v);
}

/*
 * ======================================================================================================================
 * GetDeviceID() - 
 * 
 * This function reads the 128-bit (16-byte) unique serial number from the 
 * SAMD21 microcontroller's flash memory, compresses it to 12 bytes, and 
 * converts it to a 24-character hexadecimal string.
 * 
 * The compression is done by XORing the first 8 bytes with the last 8 bytes 
 * of the original 16-byte identifier, ensuring all 128 bits contribute to 
 * the final result.
 * 
 * SAM D21/DA1 Family Datas Sheet
 * 10.3.3 Serial Number
 * Each device has a unique 128-bit serial number which is a concatenation of four 32-bit words contained at the
 * following addresses:
 * Word 0: 0x0080A00C
 * Word 1: 0x0080A040
 * Word 2: 0x0080A044
 * Word 3: 0x0080A048
 * The uniqueness of the serial number is guaranteed only when using all 128 bits.
 * ======================================================================================================================
 */
void GetDeviceID() {
  // Pointer to the unique 128-bit serial number in flash
  uint32_t *uniqueID = (uint32_t *)0x0080A00C;
  
  // Temporary buffer to store 16 bytes (128 bits)
  uint8_t fullId[16];
  
  // Extract all 16 bytes from the 128-bit serial number
  for (int i = 0; i < 4; i++) {
    uint32_t val = uniqueID[i];
    fullId[i*4] = (val >> 24) & 0xFF;
    fullId[i*4 + 1] = (val >> 16) & 0xFF;
    fullId[i*4 + 2] = (val >> 8) & 0xFF;
    fullId[i*4 + 3] = val & 0xFF;
  }

  // Compress 16 bytes to 8 bytes using XOR
  uint8_t compressedId[8];
  for (int i = 0; i < 8; i++) {
    compressedId[i] = fullId[i] ^ fullId[i + 8];
  }

  memset (DeviceID, 0, 17);
  for (int i = 0; i < 8; i++) {
    sprintf (DeviceID+strlen(DeviceID), "%02x", compressedId[i]);
  }
  DeviceID[16] = '\0'; // Ensure null-termination
}

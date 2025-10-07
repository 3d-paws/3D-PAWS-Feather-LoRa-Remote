/*
 * ======================================================================================================================
 *  cf.cpp - Configuration File Functions
 * ======================================================================================================================
 */

#include <Arduino.h>
#include <SD.h>

#include "include/ssbits.h"
#include "include/output.h"
#include "include/lora.h"
#include "include/main.h"
#include "include/cf.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
char *cf_aes_pkey=NULL;
long cf_aes_myiv=0;
int cf_lora_unitid=2;
int cf_lora_gwid=1;
int cf_lora_txpower=13;
int cf_lora_freq=915;
int cf_obs_period=15;
int cf_rg1_enable=0;
int cf_rg2_enable=0;
int cf_ds_enable=0;
int cf_ds_baseline=0;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 * =======================================================================================================================
 * Support functions for Config file
 * 
 *  https://arduinogetstarted.com/tutorials/arduino-read-config-from-sd-card
 *  
 *  myInt_1    = SD_findInt(F("myInt_1"));
 *  myFloat_1  = SD_findFloat(F("myFloat_1"));
 *  myString_1 = SD_findString(F("myString_1"));
 *  
 *  CONFIG.TXT content example
 *  myString_1=Hello
 *  myInt_1=2
 *  myFloat_1=0.74
 * =======================================================================================================================
 */

int SD_findKey(const __FlashStringHelper * key, char * value) {
  
  LoRaDisableSPI(); // Disable LoRA SPI0 Chip Select
  
  File configFile = SD.open(CF_NAME);

  if (!configFile) {
    sprintf (Buffer32Bytes, "SD File [%s] NF", CF_NAME);
    Output (Buffer32Bytes);
    return(0);
  }

  char key_string[KEY_MAX_LENGTH];
  char SD_buffer[KEY_MAX_LENGTH + VALUE_MAX_LENGTH + 1]; // 1 is = character
  int key_length = 0;
  int value_length = 0;

  // Flash string to string
  PGM_P keyPoiter;
  keyPoiter = reinterpret_cast<PGM_P>(key);
  byte ch;
  do {
    ch = pgm_read_byte(keyPoiter++);
    if (ch != 0)
      key_string[key_length++] = ch;
  } while (ch != 0);

  // check line by line
  while (configFile.available()) {
    // UNIX uses LF = \n
    // WINDOWS uses CFLF = \r\n
    int buffer_length = configFile.readBytesUntil('\n', SD_buffer, LINE_MAX_LENGTH);
    if (SD_buffer[buffer_length - 1] == '\r')
      buffer_length--; // trim the \r

    if (buffer_length > (key_length + 1)) { // 1 is = character
      if (memcmp(SD_buffer, key_string, key_length) == 0) { // equal
        if (SD_buffer[key_length] == '=') {
          value_length = buffer_length - key_length - 1;
          memcpy(value, SD_buffer + key_length + 1, value_length);
          break;
        }
      }
    }
  }

  configFile.close();  // close the file
  return value_length;
}

int HELPER_ascii2Int(char *ascii, int length) {
  int sign = 1;
  int number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

long HELPER_ascii2Long(char *ascii, int length) {
  int sign = 1;
  long number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

float HELPER_ascii2Float(char *ascii, int length) {
  int sign = 1;
  int decimalPlace = 0;
  float number  = 0;
  float decimal = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c == '.')
        decimalPlace = 1;
      else if (c >= '0' && c <= '9') {
        if (!decimalPlace)
          number = number * 10 + (c - '0');
        else {
          decimal += ((float)(c - '0') / pow(10.0, decimalPlace));
          decimalPlace++;
        }
      }
    }
  }

  return (number + decimal) * sign;
}

String HELPER_ascii2String(char *ascii, int length) {
  String str;
  str.reserve(length);
  str = "";

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str += String(c);
  }
  return str;
}

char* HELPER_ascii2CharStr(char *ascii, int length) {
  char *str;
  int i = 0;
  str = (char *) malloc (length+1);
  str[0] = 0;
  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str[i] = c;
    str[i+1] = 0;
  }
  return str;
}

bool SD_available(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return value_length > 0;
}

int SD_findInt(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Int(value_string, value_length);
}

float SD_findFloat(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Float(value_string, value_length);
}

String SD_findString(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2String(value_string, value_length);
}

char* SD_findCharStr(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2CharStr(value_string, value_length);
}

long SD_findLong(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Long(value_string, value_length);
}

/* 
 * =======================================================================================================================
 * SD_ReadConfigFile()
 * =======================================================================================================================
 */
void SD_ReadConfigFile() {
  cf_aes_pkey     = SD_findCharStr(F("aes_pkey"));
  sprintf(msgbuf, "%s=[%s]",  F("CF:aes_pkey"), cf_aes_pkey);         Output (msgbuf);

  cf_aes_myiv     = SD_findLong(F("aes_myiv"));
  sprintf(msgbuf, "%s=[%lu]", F("CF:aes_myiv"), cf_aes_myiv);         Output (msgbuf);

  cf_lora_unitid  = SD_findInt(F("lora_unitid"));
  sprintf(msgbuf, "%s=[%d]",  F("CF:lora_unitid"), cf_lora_unitid);   Output (msgbuf);

  cf_lora_gwid    = SD_findInt(F("lora_gwid"));
  sprintf(msgbuf, "%s=[%d]",  F("CF:lora_gwid"), cf_lora_gwid);       Output (msgbuf);
  
  cf_lora_txpower = SD_findInt(F("lora_txpower"));
  if (cf_lora_txpower <= 0) { cf_lora_txpower = 13; } // Safty Check
  sprintf(msgbuf, "%s=[%d]",  F("CF:lora_txpower"), cf_lora_txpower); Output (msgbuf);

  cf_lora_freq   = SD_findInt(F("lora_freq"));
  if (cf_lora_freq <= 0) { cf_lora_freq = 915; } // Safty Check
  sprintf(msgbuf, "%s=[%d]",  F("CF:lora_freq"), cf_lora_freq);       Output (msgbuf);

  cf_obs_period   = SD_findInt(F("obs_period"));
  if (cf_obs_period <= 0) { cf_obs_period = 15; } // Safty Check
  sprintf(msgbuf, "%s=[%d]",  F("CF:obs_period"), cf_obs_period);     Output (msgbuf);

  // Rain
  cf_rg1_enable   = SD_findInt(F("rg1_enable"));
  sprintf(msgbuf, "%s=[%d]",  F("CF:rg1_enable"), cf_rg1_enable);     Output (msgbuf);

  cf_rg2_enable   = SD_findInt(F("rg2_enable"));
  sprintf(msgbuf, "%s=[%d]",  F("CF:rg2_enable"), cf_rg2_enable);     Output (msgbuf);

  // Distance
  cf_ds_enable    = SD_findInt(F("ds_enable"));
  sprintf(msgbuf, "%s=[%d]",  F("CF:ds_enable"), cf_ds_enable);       Output (msgbuf);

  cf_ds_baseline = SD_findInt(F("ds_baseline"));
  sprintf(msgbuf, "%s=[%d]",  F("CF:ds_baseline"), cf_ds_baseline);   Output (msgbuf);
}

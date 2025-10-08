/*
 * ======================================================================================================================
 *  lora.cpp - LoRa Functions
 * ======================================================================================================================
 */

/*
  The recommended antenna wire lengths for Adafruit LoRa Feather boards are frequency-dependent quarter-wave whip antennas:
    For 433 MHz, the antenna length should be 6.5 inches (16.5 cm).
    For 868 MHz, the antenna length should be 3.25 inches (8.2 cm).
    For 915 MHz, the antenna length should be 3 inches (7.8 cm).
 */
 
#include <RH_RF95.h>
#include <AES.h>

#include "include/ssbits.h"
#include "include/output.h"
#include "include/cf.h"
#include "include/main.h"
#include "include/lora.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

 /*
 * =======================================================================================================================
 *  AES Encryption - These need to be changed here and on the RaspberryPi (They need to match)
 * =======================================================================================================================
 */
// Private Key
uint8_t  AES_KEY[16]; // Now filled in from CONFIG.TXT


// Initialization Vector must be and always will be 128 bits (16 bytes.)
// The real iv is actually my_iv repeated twice EX: 01234567 = 0123456701234567
// 01234567 - > 0x12D687 = 0x00 0x00 0x00 0x00 0x00 0x12 0xD6 0x87
// unsigned long long int my_iv = 01234567;
unsigned long long int AES_MYIV;

/*
 * =======================================================================================================================
 *  AES Instance
 * =======================================================================================================================
 */
AES aes;

/*
 * =======================================================================================================================
 *  Singleton instance of the radio driver
 *
 *  The RH_RF95 driver uses interrupts to react to events in the RFM module,
 *  such as the reception of a new packet, or the completion of transmission
 *  of a packet.  The RH_RF95 driver interrupt service routine reads status from
 *  and writes data to the the RFM module via the SPI interface. It is very
 *  important therefore, that if you are using the RH_RF95 driver with another
 *  SPI based deviced, that you disable interrupts while you transfer data to
 *  and from that other device.  Use cli() to disable interrupts and sei() to
 *  reenable them.
 *
 *  Feather, uses these to control the radio module
 *  #8 - used as the radio CS (chip select) pin
 *  #3 - used as the radio GPIO0 / IRQ (interrupt request) pin.
 *  #4 - used as the radio Reset pin
 * =======================================================================================================================
 */
RH_RF95 rf95(LORA_SS, LORA_INT);

bool LORA_exists = false;
unsigned int SendMsgCount=0; // Count of Messages transmitted

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * =======================================================================================================================
 * LoRaDisableSPI() 
 * =======================================================================================================================
 */
void LoRaDisableSPI() {
  // Disable LoRA SPI0 Chip Select
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
}

/*
 * =======================================================================================================================
 * LoRaSleep() 
 * =======================================================================================================================
 */
void LoRaSleep() {
  if (LORA_exists) {
    rf95.sleep(); // LoRa will stay in sleep mode until woken by changing mode to idle, transmit or receive.
                  // (eg by calling send(), recv(), available() etc
  }
}

/*
 * =======================================================================================================================
 * SendAESLoraWanMsg() 
 * =======================================================================================================================
 */
void SendAESLoraWanMsg (int bits,char *msg, int msgLength)
{
  if (LORA_exists) {
    int padedLength = msgLength + N_BLOCK - msgLength % N_BLOCK;
    byte cipher [padedLength] ;
    byte iv [N_BLOCK] ;
    byte *b;
    aes.iv_inc();
    aes.set_IV(AES_MYIV);
    aes.get_IV(iv);
    aes.do_aes_encrypt((byte *)msg, msgLength, cipher, AES_KEY, bits, iv); // Results are placed in cypher variable

    rf95.send(cipher, padedLength);
    rf95.waitPacketSent();

    LoRaDisableSPI(); // Disable LoRA SPI0 Chip Select
  
    Output("LoRa Transmitted");
  }
  else {
    Output("LoRa TX Failed");
  }
}

/*
 * =======================================================================================================================
 * SendLoRaMessage() - Send LoRa Observation Message  - We need to keep our message size 176 bytes. Have seen docs say 200 
 * 
 * Message Type 2 - Observation
 *   NCS    Length (N) and Checksum (CS)
 *   MT,    Message Type, LR or IF
 *   INT,   Station ID
 *   INT,   Transmit Counter
 *   OBS    JSON Observation
 * =======================================================================================================================
 */
void SendLoRaMessage(char *ops, const char *mtype) {
  int msgLength;
  unsigned short checksum;

  // Build LoRa message
  strcpy (msgbuf, "NCS");                 // N will be replaced with binary value (byte) representing (string length - 1)
                                          //    This is how we can send variable length AES encrypted strings
                                          //    The receiving side need to know characters folling this first byte
                                          // CS is the place holder for the Checksum
  // Message type
  sprintf (msgbuf+strlen(msgbuf), "%s,", mtype); //  LoRa Remote Message Type

  // Station ID
  sprintf (msgbuf+strlen(msgbuf), "%d,", cf_lora_unitid);    // Must be unique if multiple are transmitting

  // Transmit Counter
  sprintf (msgbuf+strlen(msgbuf), "%d,", SendMsgCount++);

  // Observations in JSON format
  sprintf (msgbuf+strlen(msgbuf), "%s", ops);

  msgLength = strlen(msgbuf);

  sprintf (Buffer32Bytes, "OBS MSG LEN[%d]", msgLength);
  Output (Buffer32Bytes);
  
  // Compute checksum
  checksum=0;
  for(int i=3;i<msgLength;i++) {
     checksum += msgbuf[i];
  }

  // Let serial console see this LoRa message
  Serial_write (msgbuf);
  
  msgbuf[0] = msgLength;
  msgbuf[1] = checksum >> 8;
  msgbuf[2] = checksum % 256;
   
  SendAESLoraWanMsg (128, msgbuf, msgLength);
}

/* 
 *=======================================================================================================================
 * lora_cf_validate() - Validate LoRa variables from CONFIG.TXT
 *=======================================================================================================================
 */
bool lora_cf_validate() {
  if (cf_aes_pkey == NULL) {
    Output ("AES PKEY !SET");
    return (false);
  }
  else if (strlen (cf_aes_pkey) != 16) {
    Output ("AES PKEY !16 Bytes");
    return (false);    
  }
  else if (cf_aes_myiv == 0) {
    Output ("AES MYIV !SET");
    return (false);
  }
  else if ((cf_lora_txpower<5) || (cf_lora_txpower>23)) {
    Output ("LORA PWR ERR");
    return (false);
  }
  else if ((cf_lora_freq!=915) && (cf_lora_freq!=866) && (cf_lora_freq!=433)) {
    Output ("LORA FREQ ERR");
    return (false);        
  }
  else if ((cf_lora_unitid<0) || (cf_lora_unitid>254)) {
    Output ("LORA ADDR ERR");
    return (false);
  }
  else if ((cf_lora_gwid<0) || (cf_lora_gwid>254)) {
    Output ("LORA GWID ERR");
    return (false);
  }
  else { 
    memcpy ((char *)AES_KEY, cf_aes_pkey, 16);
    sprintf(msgbuf, "AES_KEY[%s]", cf_aes_pkey); Output (msgbuf);

    AES_MYIV=cf_aes_myiv;
    sprintf(msgbuf, "AES_MYIV[%u]", AES_MYIV); Output (msgbuf);

    Output ("LORA CFV OK");
    return (true);
  }
}

/* 
 *=======================================================================================================================
 * lora_initialize()
 *=======================================================================================================================
 */
void lora_initialize() {
  // Validate LoRa variables from CONFIG.TXT
  if (!lora_cf_validate() || !rf95.init()) {
    Output("LoRa:Init failed");
    SystemStatusBits |= SSB_LORA;  // Turn On Bit
  }
  else {
    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
    // you can set transmitter powers from 5 to 23 dBm:
    rf95.setTxPower(cf_lora_txpower, false);    // false = PA_BOOST enabled
    rf95.setFrequency(cf_lora_freq);

    // This is our Node ID
    rf95.setThisAddress(cf_lora_unitid);
    rf95.setHeaderFrom(cf_lora_unitid);
  
    // Node ID of where we're sending packets
    rf95.setHeaderTo(cf_lora_gwid);

    LORA_exists = true;
    Output ("LORA OK");
    
  }
}

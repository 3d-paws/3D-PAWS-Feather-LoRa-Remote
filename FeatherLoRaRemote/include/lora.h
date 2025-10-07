/*
 * ======================================================================================================================
 *  lora.h - LoRa Definations
 * ======================================================================================================================
 */
#define LORA_SS   8
#define LORA_INT  3     // Feather 32u4 LoRa used pin 7

// Extern variables
extern uint8_t  AES_KEY[16];
extern unsigned long long int AES_MYIV;
extern bool LORA_exists;
extern unsigned int SendMsgCount;

// Function prototypes
void LoRaDisableSPI();
void LoRaSleep();
void SendAESLoraWanMsg (int bits,char *msg, int msgLength);
void SendLoRaMessage(char *ops, const char *mtype);
bool lora_cf_validate();
void lora_initialize();

#ifndef __STM32_DUNKS_LIB
#define __STM32_DUNKS_LIB


#include "stm32f100_io_enum.h"

#define  DESCRIPTION_LEN 16
#define  MAX_BITBAGED_PWM 10
typedef struct {
    char description[DESCRIPTION_LEN];
    port_out_type_t type;
    u32 port_number;
    u32 data;
    u32 reserved;
} output;

GPIO_TypeDef* get_GPIO_bank_from_p(output* io);
uint32_t get_GPIO_pin_num_from_p(output* io);
uint64_t read_temp(int* error);
uint8_t ds_crc(uint64_t data);
void IO_on_p(output* io);
void IO_off_p(output* io);
void IO_on_d(char* description);
void IO_off_d(char* description);
uint8_t IO_read_p(output* io);
void IO_Init(output* io);
void Delay(uint32_t nTime);
void TimingDelay_Decrement(void);
void PulseLed(void);
void tickDelay(u32 uSecs);          // TODO create ds18s20.h
void initPort(output* add);         // TODO create ds18s20.h
void SetSpeed(int standard);        // TODO create ds18s20.h
int OWReset(void);
void OWWriteBit(int bit);
int OWReadBit(void);
void OWWriteByte(int data);
uint8_t OWReadByte(void);
int OWTouchByte(int data);
void OWBlock(unsigned char *data, int data_len);
void outp(output* portaddr, u32 val);
u32 inp(output* portaddr);
int enumerate_sensors(void);


// http://www.maximintegrated.com/en/app-notes/index.mvp/id/187
// method declarations
int  OWFirst();
int  OWNext();
int  OWVerify();
void OWTargetSetup(unsigned char family_code);
void OWFamilySkipSetup();
int  OWSearch();
unsigned char docrc8(unsigned char value);

// global search state
unsigned char ROM_NO[8];
int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;
unsigned char crc8;

#endif  // __STM32_DUNKS_LIB

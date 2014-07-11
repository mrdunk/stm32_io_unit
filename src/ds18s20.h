#ifndef __DS18S20_H
#define __DS18S20_H

#include "stm32f100_io_pins.h"
#include "stm32_io.h"

uint64_t read_temp(int* error);

// http://www.maximintegrated.com/en/app-notes/index.mvp/id/187
// method declarations
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

//-----------------------------------------------------------------------------
// Set global variables expected by Maxim example code.
//
void one_wire_init_port(Port_Container_t* portaddr);

//-----------------------------------------------------------------------------
// Set the 1-Wire timing to 'standard' (standard=1) or 'overdrive' (standard=0).
//
void SetSpeed(int standard);

/* Set output of tristate pin.
 * If high, we set the GPIO pin to open drain input and use an external pull-up resistor.
 * If low, we set GPIO pin to output. Pin should already have been set low duting initPort(). 
 */
 void outp(Port_Container_t* portaddr, u32 val);

//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : no device present
//
int OWFirst();

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
int OWNext();

//-----------------------------------------------------------------------------
// Read 1-Wire data byte and return it
//
uint8_t OWReadByte(void);

//-----------------------------------------------------------------------------
// Write 1-Wire data byte
//
void OWWriteByte(int data);

//-----------------------------------------------------------------------------
// Read a bit from the 1-Wire bus and return it. Provide 10us recovery time.
//
int OWReadBit(void);

//-----------------------------------------------------------------------------
// Send a 1-Wire write bit. Provide 10us recovery time.
//
void OWWriteBit(int bit);

//-----------------------------------------------------------------------------
// Generate a 1-Wire reset, return 0 if no presence detect was found,
// return 1 otherwise.
// (NOTE: Does not handle alarm presence from DS2404/DS1994)
//
int OWReset(void);

//-----------------------------------------------------------------------------
// Set the 1-Wire timing to 'standard' (standard=1) or 'overdrive' (standard=0).
//
void SetSpeed(int standard);

// Pause for exactly 'tick' number of ticks = 0.25us
void tickDelay(u32 uSecs);

// http://www.mikroe.com/forum/viewtopic.php?t=3884
uint8_t ds_crc(uint64_t data);




#endif  // __DS18S20_H

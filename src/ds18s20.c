/* Taken from Maxim example:
 * http://www.maximintegrated.com/app-notes/index.mvp/id/126
 */

#include "ds18s20.h"


uint64_t read_temp(int* error){
    static uint64_t count;
    static uint64_t scratchpad;
    static uint64_t temp;
    static int i;

    if(!OWReadByte()){
        // Still converting temperature.
        return temp;
    }

    scratchpad = 0;
    *error = 0;

    // Match Rom to address in ROM_NO
    OWReset();
    OWWriteByte(0x55);
    for (i = 0; i < 8; ++i){
        OWWriteByte(ROM_NO[i]);
    }
    // and read scratch register.
    OWWriteByte(0xBE);
    for(count = 0; count < 8; ++count){
        scratchpad |= ((uint64_t)OWReadByte() << (count * 8));
    }

    if(OWReadByte() != ds_crc(scratchpad)){
        // CRC does not match.
        *error = 1;
    } else {
        // read temp.
        scratchpad &= 0xffff;
        temp = scratchpad;
    }

    // Match Rom to address in ROM_NO
    OWReset();
    OWWriteByte(0x55);
    for (i = 0; i < 8; ++i){
        OWWriteByte(ROM_NO[i]);
    }
    // Start new conversion.
    OWWriteByte(0x44);

    return temp;
}

// http://www.mikroe.com/forum/viewtopic.php?t=3884
uint8_t ds_crc(uint64_t data)
{
    uint8_t shift_reg=0, data_bit, sr_lsb, fb_bit, i, j, byte;

    for (i=0; i<8; ++i){        /* for each byte */
        byte = data >> (i * 8);
        for(j=0; j<8; j++){     /* for each bit */
            data_bit = (byte>>j)&0x01;
            sr_lsb = shift_reg & 0x01;
            fb_bit = (data_bit ^ sr_lsb) & 0x01;
            shift_reg = shift_reg >> 1;
            if (fb_bit){
                shift_reg = shift_reg ^ 0x8c;
            }
        }
    }
    return(shift_reg);
}

Port_Container_t* PORTADDRESS = 0;
u32 gpio_pin_output;
u32 gpio_pin_input;

void one_wire_init_port(Port_Container_t* portaddr){
    if(PORTADDRESS == portaddr){
        // Allready set to this port.
        return;
    }

    // Maxim code expects this GPIO pin define.
    PORTADDRESS = portaddr;

    // Configure GPIO.
//    portaddr->type = BINARY_OUT_OD;
//    IO_Init(portaddr);
    io_off_p(portaddr);

    // Set these values to be used toggling tristate pin between input and output
    // in outp().
    gpio_pin_input = (u32)(0x4 << (8 * GPIO_pin_num[portaddr->port_number]));
    gpio_pin_output = (u32)(0x1 << (8 * GPIO_pin_num[portaddr->port_number]));

    // Set GPIO high ready for 1-wire communications.
    outp(portaddr, 1);
}

/* Set output of tristate pin.
 * If high, we set the GPIO pin to open drain input and use an external pull-up resistor.
 * If low, we set GPIO pin to output. Pin should already have been set low duting initPort(). 
 */
void outp(Port_Container_t* portaddr, u32 val){
    if(val){
        // Release pin.
        // This needs to happen as quickly as posible so we do not use the standard pin configuration functions
        // but rather manipulate the GPIO control registers directly.

        //IO_on_p(portaddr);
        //portaddr->type = BINARY_IN_OD;
        //IO_Init(portaddr);

        GPIO_bank[portaddr->port_number]->CRL = gpio_pin_input;
        //GPIOC->CRL = 0x4;

    } else {
        // drive pin low.

        //portaddr->type = BINARY_OUT_OD;
        //IO_Init(portaddr);
        //io_off_p(portaddr);

        GPIO_bank[portaddr->port_number]->CRL = gpio_pin_output;
    }
}

inline u32 inp(Port_Container_t* portaddr){
    return io_read_p(portaddr);
}

// Pause for exactly 'tick' number of ticks = 0.25us
void tickDelay(u32 uSecs){
    static u32 firstRun = 0;

    if(!firstRun){
        firstRun = 1;

        /* Enable timer clock  - use TIMER6 */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);


        /* Time base configuration */
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
        TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)(SystemCoreClock / 1000000) - 1;  // ticks of 1/4 microsecond.
        TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
        TIM_TimeBaseStructure.TIM_ClockDivision = 0;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

        /* Enable counter */
        TIM_Cmd(TIM6, ENABLE);
    }

    TIM_SetCounter(TIM6, 0);
    while((u32)TIM6->CNT < uSecs);
}

// 'tick' values
int A,B,C,D,E,F,G,H,I,J;

//-----------------------------------------------------------------------------
// Set the 1-Wire timing to 'standard' (standard=1) or 'overdrive' (standard=0).
//
void SetSpeed(int standard)
{
        // Adjust tick values depending on speed
        if (standard)
        {
                // Standard Speed
                A = 6;
                B = 64;
                C = 60;
                D = 10;
                E = 9;
                F = 55;
                G = 0;
                H = 480;
                I = 70;
                J = 410;
        }
        else
        {
                // Overdrive Speed
                A = 1 * 4;
                B = 7.5 * 4;
                C = 7.5 * 4;
                D = 2.5 * 4;
                E = 0.75 * 4;
                F = 7 * 4;
                G = 2.5 * 4;
                H = 70 * 4;
                I = 8.5 * 4;
                J = 40 * 4;
        }
}

//-----------------------------------------------------------------------------
// Generate a 1-Wire reset, return 0 if no presence detect was found,
// return 1 otherwise.
// (NOTE: Does not handle alarm presence from DS2404/DS1994)
//
int OWReset(void)
{
        int result;
        tickDelay(G);
        outp(PORTADDRESS,0x00); // Drives DQ low
        tickDelay(H);
        outp(PORTADDRESS,0x01); // Releases the bus
        tickDelay(I);
        result = inp(PORTADDRESS) ^ 0x01; // Sample for presence pulse from slave
        tickDelay(J); // Complete the reset sequence recovery
        return result; // Return sample presence pulse result
}

//-----------------------------------------------------------------------------
// Send a 1-Wire write bit. Provide 10us recovery time.
//
void OWWriteBit(int bit)
{
        if (bit)
        {
                // Write '1' bit
                outp(PORTADDRESS,0x00); // Drives DQ low
                tickDelay(A);
                outp(PORTADDRESS,0x01); // Releases the bus
                tickDelay(B); // Complete the time slot and 10us recovery
        }
        else
        {
                // Write '0' bit
                outp(PORTADDRESS,0x00); // Drives DQ low
                tickDelay(C);
                outp(PORTADDRESS,0x01); // Releases the bus
                tickDelay(D);
        }
}

//-----------------------------------------------------------------------------
// Read a bit from the 1-Wire bus and return it. Provide 10us recovery time.
//
int OWReadBit(void)
{
        int result;

        outp(PORTADDRESS,0x00); // Drives DQ low
        tickDelay(A);
        outp(PORTADDRESS,0x01); // Releases the bus
        tickDelay(E);
        result = inp(PORTADDRESS) & 0x01; // Sample the bit value from the slave
        tickDelay(F); // Complete the time slot and 10us recovery

        return result;
}

//-----------------------------------------------------------------------------
// Write 1-Wire data byte
//
void OWWriteByte(int data)
{
        int loop;

        // Loop to write each bit in the byte, LS-bit first
        for (loop = 0; loop < 8; loop++)
        {
                OWWriteBit(data & 0x01);

                // shift the data byte for the next bit
                data >>= 1;
        }
}

//-----------------------------------------------------------------------------
// Read 1-Wire data byte and return it
//
uint8_t OWReadByte(void)
{
        uint8_t loop, result=0;

        for (loop = 0; loop < 8; loop++)
        {
                // shift the result to get it ready for the next bit
                result >>= 1;

                // if result is one, then set MS bit
                if (OWReadBit())
                        result |= 0x80;
        }
        return result;
}

//-----------------------------------------------------------------------------
// Write a 1-Wire data byte and return the sampled result.
//
int OWTouchByte(int data)
{
        int loop, result=0;

        for (loop = 0; loop < 8; loop++)
        {
                // shift the result to get it ready for the next bit
                result >>= 1;

                // If sending a '1' then read a bit else write a '0'
                if (data & 0x01)
                {
                        if (OWReadBit())
                                result |= 0x80;
                }
                else
                        OWWriteBit(0);

                // shift the data byte for the next bit
                data >>= 1;
        }
        return result;
}

//-----------------------------------------------------------------------------
// Write a block 1-Wire data bytes and return the sampled result in the same
// buffer.
//
void OWBlock(unsigned char *data, int data_len)
{
        int loop;

        for (loop = 0; loop < data_len; loop++)
        {
                data[loop] = OWTouchByte(data[loop]);
        }
}

//-----------------------------------------------------------------------------
// Set all devices on 1-Wire to overdrive speed. Return '1' if at least one
// overdrive capable device is detected.
//
int OWOverdriveSkip(unsigned char *data, int data_len)
{
        // set the speed to 'standard'
        SetSpeed(1);

        // reset all devices
        if (OWReset()) // Reset the 1-Wire bus
                return 0; // Return if no devices found

        // overdrive skip command
        OWWriteByte(0x3C);

        // set the speed to 'overdrive'
        SetSpeed(0);

        // do a 1-Wire reset in 'overdrive' and return presence result
        return OWReset();
}


/* http://www.maximintegrated.com/en/app-notes/index.mvp/id/187 */
// definitions
#define FALSE 0
#define TRUE  1

//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : no device present
//
int OWFirst()
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = FALSE;
   LastFamilyDiscrepancy = 0;

   return OWSearch();
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
int OWNext()
{
   // leave the search state alone
   return OWSearch();
}

//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
int OWSearch()
{
   int id_bit_number;
   int last_zero, rom_byte_number, search_result;
   int id_bit, cmp_id_bit;
   unsigned char rom_byte_mask, search_direction;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = 0;
   crc8 = 0;

   // if the last call was not the last one
   if (!LastDeviceFlag)
   {
      // 1-Wire reset
      if (!OWReset())
      {
         // reset the search
         LastDiscrepancy = 0;
         LastDeviceFlag = FALSE;
         LastFamilyDiscrepancy = 0;
         return FALSE;
      }

      // issue the search command 
      OWWriteByte(0xF0);  

      // loop to do the search
      do
      {
         // read a bit and its complement
         id_bit = OWReadBit();
         cmp_id_bit = OWReadBit();

         // check for no devices on 1-wire
         if ((id_bit == 1) && (cmp_id_bit == 1))
            break;
         else
         {
            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit)
               search_direction = id_bit;  // bit write value for search
            else
            {
               // if this discrepancy if before the Last Discrepancy
               // on a previous next then pick the same as last time
               if (id_bit_number < LastDiscrepancy)
                  search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               else
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == LastDiscrepancy);

               // if 0 was picked then record its position in LastZero
               if (search_direction == 0)
               {
                  last_zero = id_bit_number;

                  // check for Last discrepancy in family
                  if (last_zero < 9)
                     LastFamilyDiscrepancy = last_zero;
               }
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
              ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
              ROM_NO[rom_byte_number] &= ~rom_byte_mask;

            // serial number search direction write bit
            OWWriteBit(search_direction);

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0)
            {
                docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

      // if the search was successful then
      if (!((id_bit_number < 65) || (crc8 != 0)))
      {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
         LastDiscrepancy = last_zero;

         // check for last device
         if (LastDiscrepancy == 0)
            LastDeviceFlag = TRUE;
         
         search_result = TRUE;
      }
   }

   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || !ROM_NO[0])
   {
      LastDiscrepancy = 0;
      LastDeviceFlag = FALSE;
      LastFamilyDiscrepancy = 0;
      search_result = FALSE;
   }

   return search_result;
}

//--------------------------------------------------------------------------
// Verify the device with the ROM number in ROM_NO buffer is present.
// Return TRUE  : device verified present
//        FALSE : device not present
//
int OWVerify()
{
   unsigned char rom_backup[8];
   int i,rslt,ld_backup,ldf_backup,lfd_backup;

   // keep a backup copy of the current state
   for (i = 0; i < 8; i++)
      rom_backup[i] = ROM_NO[i];
   ld_backup = LastDiscrepancy;
   ldf_backup = LastDeviceFlag;
   lfd_backup = LastFamilyDiscrepancy;

   // set search to find the same device
   LastDiscrepancy = 64;
   LastDeviceFlag = FALSE;

   if (OWSearch())
   {
      // check if same device found
      rslt = TRUE;
      for (i = 0; i < 8; i++)
      {
         if (rom_backup[i] != ROM_NO[i])
         {
            rslt = FALSE;
            break;
         }
      }
   }
   else
     rslt = FALSE;

   // restore the search state 
   for (i = 0; i < 8; i++)
      ROM_NO[i] = rom_backup[i];
   LastDiscrepancy = ld_backup;
   LastDeviceFlag = ldf_backup;
   LastFamilyDiscrepancy = lfd_backup;

   // return the result of the verify
   return rslt;
}

//--------------------------------------------------------------------------
// Setup the search to find the device type 'family_code' on the next call
// to OWNext() if it is present.
//
void OWTargetSetup(unsigned char family_code)
{
   int i;

   // set the search state to find SearchFamily type devices
   ROM_NO[0] = family_code;
   for (i = 1; i < 8; i++)
      ROM_NO[i] = 0;
   LastDiscrepancy = 64;
   LastFamilyDiscrepancy = 0;
   LastDeviceFlag = FALSE;
}

//--------------------------------------------------------------------------
// Setup the search to skip the current device type on the next call
// to OWNext().
//
void OWFamilySkipSetup()
{
   // set the Last discrepancy to last family discrepancy
   LastDiscrepancy = LastFamilyDiscrepancy;
   LastFamilyDiscrepancy = 0;

   // check for end of list
   if (LastDiscrepancy == 0)
      LastDeviceFlag = TRUE;
}

static unsigned char dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current 
// global 'crc8' value. 
// Returns current global crc8 value
//
unsigned char docrc8(unsigned char value)
{
   // See Application Note 27
   
   // TEST BUILD
   crc8 = dscrc_table[crc8 ^ value];
   return crc8;
}

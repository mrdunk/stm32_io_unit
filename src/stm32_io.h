#ifndef __STM32_IO_H
#define __STM32_IO_H

//#include <stdlib.h>

#include "stm32f10x.h"
#include "stm32f100_io_pins.h"
#include "stm32f10x_tim.h"
//#include "ds18s20.h"

#include <stdint.h>

#define DESCRIPTION_LEN 12
#define MAX_CONTAINERS 32
#define HEAP_USAGE 256
#define MAX_BITBAGED_PWM 10

/* All the information about an IO pin. 
 *
 *  descrip: A text description which forms the key for pin lookups.
 *  type: The type of IO to configure this pin for.
 *  port_number: IO pin. See stm32f100_io_pins.h .
 *  p_data: A pointer to a data array. The exact usage varies with IO type.
 *  data_write_itterator: A pointer within the data array to be used when writing data to the array.
 *  data_read_itteratorr: A pointer within the data array to be used when reading data from the array.
 */
struct Port_Container {
    char descrip[DESCRIPTION_LEN];
    port_type_t type;
    port_numbers_t port_number;
    u32* p_data;
    u32 data_write_itterator;
    u32 data_read_itterator;
};
typedef struct Port_Container Port_Container_t;

/* Array containing all registered IO pins. */
Port_Container_t containers[MAX_CONTAINERS];
u32 container_size;


/* Register IO instance.
 * If "descrip" matches one already registered, that entry is overwritten.
 *
 * Args:
 *  (char) descrip[]:   Unique string containing description.
 *  (port_type_t) type: One of the Port types listed in stm32f100_io_enum.h.
 *  (port_numbers_t) port_number: Index of port as listed in stm32f100_io_enum.h.
*/
void io_register(char descrip[DESCRIPTION_LEN], port_type_t type, port_numbers_t port_number);

void IO_Init(Port_Container_t* io);

Port_Container_t* io_dectiption_to_p(char descrip[DESCRIPTION_LEN]);

void io_clear(char descrip[DESCRIPTION_LEN]);
void io_clear_p(Port_Container_t* p_container);

void io_set_u32(char descrip[DESCRIPTION_LEN], u32 data);
void io_set_u32_p(Port_Container_t* p_container, u32 data);
void io_set_u8(char descrip[DESCRIPTION_LEN], u8 data);
void io_set_u8_p(Port_Container_t* p_container, u8 data);

void io_update(char descrip[DESCRIPTION_LEN]);
void io_update_p(Port_Container_t* p_container);

u32 io_data_alloc(Port_Container_t* p_container, u32 set_data_size);
u32 _io_data_alloc_new(Port_Container_t* p_container, u32 set_data_size);
u32 _io_data_alloc_shrink(Port_Container_t* p_container, u32 set_data_size);
u32 _io_data_alloc_grow(Port_Container_t* p_container, u32 set_data_size);

/* Read one u32 from data ring buffer.*/
u32 _data_read_u32(Port_Container_t* p_container);

/* Read one u32 from data ring buffer without moving allong buffer.*/
u32 _data_read_u32_no_itterate(Port_Container_t* p_container);

u8 _data_read_u8(Port_Container_t* p_container);
u8 _data_read_u8_no_itterate(Port_Container_t* p_container);

void Timer_Init(TIM_TypeDef* tim);

/* Function servicing Bitbanged PWM. Should be called once per ms. */
void _PWMBitbanged(void);

/* Switch on an IO pin that has been configured as BINARY_OUT_PP or BINARY_OUT_OD without reading from bffers.*/
inline void io_on_p(Port_Container_t* p_container);

/* Switch off an IO pin that has been configured as BINARY_OUT_PP or BINARY_OUT_OD without reading from bffers.*/
inline void io_off_p(Port_Container_t* p_container);

/* return value of an IO pin that has been configured as BINARY_OUT_PP or BINARY_OUT_OD without writing to bffers.*/
inline u32 io_read_p(Port_Container_t* p_container);

#endif  // __STM32_IO_H

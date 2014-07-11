#ifndef __STM32F100_io_enum_H
#define __STM32F100_io_enum_H

#include <stdint.h>
#include "stm32f10x.h"


typedef enum {
    BINARY_OUT_PP = 0,
    BINARY_OUT_OD = 1,
    PWM = 2,
    PWM_BITBANG = 3,
    BINARY_IN_PU = 4,
    BINARY_IN_PD = 5,
    BINARY_IN_OD = 6,
    SERIAL_1_WIRE = 7,
    TEST = 0xff,
} port_type_t;

typedef enum {
    DATA_SIZE_PWM = 1,
    DATA_SIZE_OUT = 1,
    DATA_SIZE_D_IN = 1,
    DATA_SIZE_A_IN = 1,
    DATA_SIZE_1_WIRE = 0,
} port_data_size;

typedef enum {
    PortA0 = 0,
    PortA1 = 1,
    PortA2 = 2,
    PortA3 = 3,
    PortA4 = 4,
    PortA5 = 5,
    PortA6 = 6,
    PortA7 = 7,
    PortA8 = 8,
    PortA9 = 9,
    PortA10 = 10,
    PortA11 = 11,
    PortA12 = 12,
    PortA13 = 13,
    PortA14 = 14,
    PortA15 = 15,
    PortB0 = 16,
    PortB1 = 17,
    PortB2 = 18,
    PortB3 = 19,
    PortB4 = 20,
    PortB5 = 21,
    PortB6 = 22,
    PortB7 = 23,
    PortB8 = 24,
    PortB9 = 25,
    PortB10 = 26,
    PortB11 = 27,
    PortB12 = 28,
    PortB13 = 29,
    PortB14 = 30,
    PortB15 = 31,
    PortC0 = 32,
    PortC1 = 33,
    PortC2 = 34,
    PortC3 = 35,
    PortC4 = 36,
    PortC5 = 37,
    PortC6 = 38,
    PortC7 = 39,
    PortC8 = 40,
    PortC9 = 41,
    PortC10 = 42,
    PortC11 = 43,
    PortC12 = 44,
    PortC13 = 45,
    PortC14 = 46,
    PortC15 = 47,
} port_numbers_t;

extern GPIO_TypeDef* GPIO_bank[48];
extern const port_data_size port_type_data_size[];
extern const uint16_t GPIO_pin[];
extern const uint8_t GPIO_pin_num[];
extern const uint32_t GPIO_clk[];
extern sc32 PortTimerAlt[][2];
extern sc32 PortTimer[][2];

#endif  // __STM32F100_io_enum_H

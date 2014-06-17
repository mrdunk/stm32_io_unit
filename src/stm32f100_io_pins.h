#ifndef __STM32F100_io_pins_H
#define __STM32F100_io_pins_H

#include "stm32f10x_tim.h"

static GPIO_TypeDef* GPIO_bank[] = {
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOB,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC,
    GPIOC
};

static uint16_t GPIO_pin[] = {
    GPIO_Pin_0,
    GPIO_Pin_1,
    GPIO_Pin_2,
    GPIO_Pin_3,
    GPIO_Pin_4,
    GPIO_Pin_5,
    GPIO_Pin_6,
    GPIO_Pin_7,
    GPIO_Pin_8,
    GPIO_Pin_9,
    GPIO_Pin_10,
    GPIO_Pin_11,
    GPIO_Pin_12,
    GPIO_Pin_13,
    GPIO_Pin_14,
    GPIO_Pin_15,
    GPIO_Pin_0,
    GPIO_Pin_1,
    GPIO_Pin_2,
    GPIO_Pin_3,
    GPIO_Pin_4,
    GPIO_Pin_5,
    GPIO_Pin_6,
    GPIO_Pin_7,
    GPIO_Pin_8,
    GPIO_Pin_9,
    GPIO_Pin_10,
    GPIO_Pin_11,
    GPIO_Pin_12,
    GPIO_Pin_13,
    GPIO_Pin_14,
    GPIO_Pin_15,
    GPIO_Pin_0,
    GPIO_Pin_1,
    GPIO_Pin_2,
    GPIO_Pin_3,
    GPIO_Pin_4,
    GPIO_Pin_5,
    GPIO_Pin_6,
    GPIO_Pin_7,
    GPIO_Pin_8,
    GPIO_Pin_9,
    GPIO_Pin_10,
    GPIO_Pin_11,
    GPIO_Pin_12,
    GPIO_Pin_13,
    GPIO_Pin_14,
    GPIO_Pin_15
};

static uint8_t GPIO_pin_num[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf
};

static uint32_t GPIO_clk[] = {
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOC
};

sc32 PortTimer[][2] = {
    {(u32)TIM2, 1},  // A0  TIM2_CH1
    {(u32)TIM2, 2},  // A1  TIM2_CH2
    {0   , 0},  // A2  TIM2_CH3   alt: TIM15_CH1
    {0   , 0},  // A3  TIM2_CH4   alt: TIM15_CH2
    {0   , 0},  // A4
    {0   , 0},  // A5  
    {(u32)TIM3, 1},  // A6  TIM3_CH1
    {(u32)TIM3, 2},  // A7  TIM3_CH2
    {(u32)TIM1, 1},  // A8  TIM1_CH1
    {(u32)TIM1, 2},  // A9  TIM1_CH2
    {(u32)TIM1, 3},  // A10  TIM1_CH3
    {(u32)TIM1, 4},  // A11  TIM1_CH4
    {0   , 0},  // A12
    {0   , 0},  // A13
    {0   , 0},  // A14
    {0   , 0},  // A15
    {(u32)TIM3, 3},  // B0  TIM3_CH3
    {(u32)TIM3, 4},  // B1  TIM3_CH4
    {0   , 0},  // B2  
    {0   , 0},  // B3              alt: TIM2_CH2
    {0   , 0},  // B4              alt: TIM3_CH1
    {0   , 0},  // B5              alt: TIM3_CH2
    {(u32)TIM4, 1},  // B6  TIM4_CH1
    {(u32)TIM4, 2},  // B7  TIM4_CH2
    {(u32)TIM4, 3},  // B8  TIM4_CH3
    {(u32)TIM4, 4},  // B9  TIM4_CH4
    {0   , 0},  // B10            alt: TIM2_CH3
    {0   , 0},  // B11            alt: TIM2_CH4  
    {0   , 0},  // B12  
    {0   , 0},  // B13
    {0   , 0},  // B14            alt: TIM15_CH1
    {0   , 0},  // B15            alt: TIM15_CH1
    {0   , 0},  // C0  
    {0   , 0},  // C1
    {0   , 0},  // C2
    {0   , 0},  // C3
    {0   , 0},  // C4
    {0   , 0},  // C5
    {0   , 0},  // C6            alt: TIM3_CH1
    {0   , 0},  // C7            alt: TIM3_CH2
    {0   , 0},  // C8            alt: TIM3_CH3
    {0   , 0},  // C9            alt: TIM3_CH4
    {0   , 0},  // C10
    {0   , 0},  // C11
    {0   , 0},  // C12
    {0   , 0},  // C13
    {0   , 0},  // C14
    {0   , 0},  // C15
};

sc32 PortTimerAlt[][2] = {
    {0   , 0},  // A0  TIM2_CH1
    {0   , 0},  // A1  TIM2_CH2
    {(u32)TIM15, 1},  // A2  TIM2_CH3   alt: TIM15_CH1
    {(u32)TIM15, 2},  // A3  TIM2_CH4   alt: TIM15_CH2
    {0   , 0},  // A4
    {0   , 0},  // A5  
    {0   , 0},  // A6  TIM3_CH1
    {0   , 0},  // A7  TIM3_CH2
    {0   , 0},  // A8  TIM1_CH1
    {0   , 0},  // A9  TIM1_CH2
    {0   , 0},  // A10  TIM1_CH3
    {0   , 0},  // A11  TIM1_CH4
    {0   , 0},  // A12
    {0   , 0},  // A13
    {0   , 0},  // A14
    {0   , 0},  // A15
    {0   , 0},  // B0  TIM3_CH3
    {0   , 0},  // B1  TIM3_CH4
    {0   , 0},  // B2  
    {(u32)TIM2, 2},  // B3              alt: TIM2_CH2
    {(u32)TIM3, 1},  // B4              alt: TIM3_CH1
    {(u32)TIM3, 2},  // B5              alt: TIM3_CH2
    {0   , 0},  // B6  TIM4_CH1
    {0   , 0},  // B7  TIM4_CH2
    {0   , 0},  // B8  TIM4_CH3
    {0   , 0},  // B9  TIM4_CH4
    {(u32)TIM2, 3},  // B10            alt: TIM2_CH3
    {(u32)TIM2, 4},  // B11            alt: TIM2_CH4  
    {0   , 0},  // B12  
    {(u32)TIM15, 1},  // B13
    {(u32)TIM15, 2},  // B14            alt: TIM15_CH1
    {0   , 0},  // B15            alt: TIM15_CH2
    {0   , 0},  // C0  
    {0   , 0},  // C1
    {0   , 0},  // C2
    {0   , 0},  // C3
    {0   , 0},  // C4
    {0   , 0},  // C5
    {(u32)TIM3, 1},  // C6            alt: TIM3_CH1
    {(u32)TIM3, 2},  // C7            alt: TIM3_CH2
    {(u32)TIM3, 3},  // C8            alt: TIM3_CH3
    {(u32)TIM3, 4},  // C9            alt: TIM3_CH4
    {0   , 0},  // C10
    {0   , 0},  // C11
    {0   , 0},  // C12
    {0   , 0},  // C13
    {0   , 0},  // C14
    {0   , 0},  // C15
};



#endif

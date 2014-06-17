/**
  ******************************************************************************
  * @file    Demo/src/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    09/13/2010
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include <string.h>

#include "stm32f100_io_pins.h"
#include "stm32_dunks_lib.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
void PWMBitbanged(void);

/* Private macro -------------------------------------------------------------*/
/* Private consts ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
// import these from .ld script.
extern unsigned int _sheap;  // Start of Heap.
extern unsigned int _eheap;  // End of Heap.
extern u32 systick_triggered;

u32 KeyState = 0;
static __IO uint32_t TimingDelay;

output* pwm_bitbanged[MAX_BITBAGED_PWM];
u32 num_PWM_bitbanged = 0;

output* outputs = (output*)&_sheap;
u32 output_cnt = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void Timer_Init(TIM_TypeDef* tim){
  /* Compute the prescaler value */
  uint16_t PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;

  /* Time base configuration */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  TIM_TimeBaseStructure.TIM_Period = 665;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(tim, &TIM_TimeBaseStructure);

  /* enable counter */
  TIM_Cmd(tim, ENABLE);
}

output* DescriptionToPointer(char* description){
    output* io = 0;
    u32 itter;
    for(itter = 0; itter < output_cnt; ++itter){
        if(strncmp(description, (((output*)&_sheap) + itter)->description, DESCRIPTION_LEN) == 0){
            // Description matches so this is the one we want.
            io = ((output*)&_sheap) + itter;
            break;
        }
    }

    return io;
}

void PWM_set_p(output* io, u32 val){
    //static u32 t1_init = 0;  // TODO
    //static u32 t2_init = 0;
    static u32 t3_init = 0;


    if(io->type == PWM){
        if(t3_init == 0 && (TIM_TypeDef *)PortTimerAlt[io->port_number][0] == TIM3){
            Timer_Init(TIM3);
            t3_init = 1;
        }

        // Make val operateover a range of 0-100.
        val = (val * 666) / 100;

        static TIM_OCInitTypeDef  TIM_OCInitStructure;
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

        /* PWM1 Mode configuration: Channel1 */
        TIM_OCInitStructure.TIM_Pulse = val;

        if(PortTimer[io->port_number][0] != 0){
            //TODO
        } else if(PortTimerAlt[io->port_number][0] != 0){
            switch(PortTimerAlt[io->port_number][1]){
                case 1:
                    TIM_OC1Init((TIM_TypeDef *)PortTimerAlt[io->port_number][0], &TIM_OCInitStructure);
                    TIM_OC1PreloadConfig((TIM_TypeDef *)PortTimerAlt[io->port_number][0], TIM_OCPreload_Enable);
                    break;
                case 2:
                    TIM_OC2Init((TIM_TypeDef *)PortTimerAlt[io->port_number][0], &TIM_OCInitStructure);
                    TIM_OC2PreloadConfig((TIM_TypeDef *)PortTimerAlt[io->port_number][0], TIM_OCPreload_Enable);
                    break;
                case 3:
                    TIM_OC3Init((TIM_TypeDef *)PortTimerAlt[io->port_number][0], &TIM_OCInitStructure);
                    TIM_OC3PreloadConfig((TIM_TypeDef *)PortTimerAlt[io->port_number][0], TIM_OCPreload_Enable);
                    break;
                case 4:
                    TIM_OC4Init((TIM_TypeDef *)PortTimerAlt[io->port_number][0], &TIM_OCInitStructure);
                    TIM_OC4PreloadConfig((TIM_TypeDef *)PortTimerAlt[io->port_number][0], TIM_OCPreload_Enable);
            }
            // Use alternative pin.
            TIM_ARRPreloadConfig((TIM_TypeDef *)PortTimerAlt[io->port_number][0], ENABLE);
        }
    } else if(io->type == PWM_BITBANG){
        io->data =  val;
    }
}

void PWM_set_d(char* description, u32 val){
    PWM_set_p(DescriptionToPointer(description), val);
}

void IO_Init(output* io){
    static GPIO_InitTypeDef  GPIO_InitStructure;

    if(io->type == BINARY_OUT_OD){
        /* Enable GPIOx Clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

        RCC_APB2PeriphClockCmd(GPIO_clk[io->port_number], ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_pin[io->port_number];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(GPIO_bank[io->port_number], &GPIO_InitStructure);
    } else if(io->type == BINARY_OUT_PP){
        /* Enable GPIOx Clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

        RCC_APB2PeriphClockCmd(GPIO_clk[io->port_number], ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_pin[io->port_number];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(GPIO_bank[io->port_number], &GPIO_InitStructure);
    } else if(io->type == PWM){
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        RCC_APB2PeriphClockCmd(GPIO_clk[io->port_number] | RCC_APB2Periph_AFIO, ENABLE);
        GPIO_InitStructure.GPIO_Pin =  GPIO_pin[io->port_number];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(GPIO_bank[io->port_number], &GPIO_InitStructure);
        GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
    } else if(io->type == PWM_BITBANG){
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

        RCC_APB2PeriphClockCmd(GPIO_clk[io->port_number], ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_pin[io->port_number];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(GPIO_bank[io->port_number], &GPIO_InitStructure);

        // Add pointer to array to be serviced by timer.
        pwm_bitbanged[num_PWM_bitbanged] = io;
        ++num_PWM_bitbanged;
    } else if(io->type == BINARY_IN_PU){
        /* Enable the BUTTON Clock */
        RCC_APB2PeriphClockCmd(GPIO_clk[io->port_number], ENABLE);

        /* Configure Button pin as input Pulled Up */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Pin = GPIO_pin[io->port_number];
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIO_bank[io->port_number], &GPIO_InitStructure);
    } else if(io->type == BINARY_IN_PD){
        /* Enable the BUTTON Clock */
        RCC_APB2PeriphClockCmd(GPIO_clk[io->port_number], ENABLE);

        /* Configure Button pin as input floating */
        GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPD;
        GPIO_InitStructure.GPIO_Pin = GPIO_pin[io->port_number];
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIO_bank[io->port_number], &GPIO_InitStructure);
    } else if(io->type == BINARY_IN_OD){
        /* Enable the BUTTON Clock */
        RCC_APB2PeriphClockCmd(GPIO_clk[io->port_number], ENABLE);

        /* Configure Button pin as input floating */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin = GPIO_pin[io->port_number];
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIO_bank[io->port_number], &GPIO_InitStructure);
    }

}

inline GPIO_TypeDef* get_GPIO_bank_from_p(output* io){
    return GPIO_bank[io->port_number];
}

inline uint32_t get_GPIO_pin_num_from_p(output* io){
    return GPIO_pin_num[io->port_number];
}

inline void IO_on_p(output* io){
    GPIO_bank[io->port_number]->BSRR = GPIO_pin[io->port_number];
    //GPIO_SetBits(GPIO_bank[io->port_number], GPIO_pin[io->port_number]);
}

inline void IO_off_p(output* io){
    GPIO_bank[io->port_number]->BRR = GPIO_pin[io->port_number];
    //GPIO_ResetBits(GPIO_bank[io->port_number], GPIO_pin[io->port_number]);
}

void IO_on_d(char* description){
    IO_on_p(DescriptionToPointer(description));
}

void IO_off_d(char* description){
    IO_off_p(DescriptionToPointer(description));
}

inline uint8_t IO_read_p(output* io){
    return GPIO_bank[io->port_number]->IDR & GPIO_pin[io->port_number];
    //return (uint8_t)GPIO_ReadInputDataBit(GPIO_bank[io->port_number], GPIO_pin[io->port_number]);
}

uint8_t IO_read_d(char* description){
    return IO_read_p(DescriptionToPointer(description));
}

void IO_populate(char descrip[DESCRIPTION_LEN], port_out_type_t type, u32 port_number, u32 data){
    output* io = 0;
    u32 itter;
    for(itter = 0; itter < output_cnt; ++itter){
        if(strncmp(descrip, (((output*)&_sheap) + itter)->description, DESCRIPTION_LEN) == 0){
            // Description matches so overwrite.
            io = ((output*)&_sheap) + itter;
            break;
        }
    }

    if(io == 0){
        // No matches in buffer so create new entry.
        io = ((output*)&_sheap) + output_cnt;
        ++output_cnt;
    }

    strncpy(io->description, descrip, DESCRIPTION_LEN);
    io->type = type;
    io->port_number = port_number;
    io->data = data;

    IO_Init(io);
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{

    int crc_error;
    uint64_t temperature;
    int counter_seconds =0;

    IO_populate("led3", PWM, PortC9, 100);
    //IO_populate("led3", PWM_BITBANG, PortC9, 100);
    //IO_populate("led4", PWM, PortC8, 100);
    IO_populate("led4", BINARY_OUT_PP, PortC8, 100);
    IO_populate("PA0", BINARY_IN_PD, PortA0, 1);
    
    IO_populate("PC0", BINARY_IN_OD, PortC0, 0x0);
    //IO_populate("PC1", BINARY_IN_OD, PortC1, 0x0);
    

    output* pc0 = DescriptionToPointer("PC0");
    SetSpeed(1);
    initPort(pc0);
    tickDelay(1000);
    systick_triggered = 0;
    

    /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(SystemCoreClock / 1000))
    { 
        /* Capture error */ 
        while (1);
    }
    

    u32 led3_brightness = 0;
    u32 led4_brightness = 0;

    enumerate_sensors();

    /* main while */
    while(1){
        if(IO_read_d("PA0") == 0) {
            if(KeyState == 1) {
                if(IO_read_d("PA0") == 0) {
                    /* USER Button released */
                    KeyState = 0;
                    led4_brightness += 10;
                    if(led4_brightness > 100) led4_brightness = 0;
                    PWM_set_d("led3", 100);
                    //PWM_set_d("led4", 100);
                    int i;
                    for(i = 0; i < 100; ++i){
                        tickDelay(1000);
                    }
                    PWM_set_d("led3", led4_brightness);
                    //PWM_set_d("led4", led4_brightness);
                }       
            }
        }
        else if(IO_read_d("PA0")){ 
            if(KeyState == 0) {
                if(IO_read_d("PA0")) {
                    KeyState = 1;
                }
            }
        }
       
        if(systick_triggered){
            --systick_triggered;

            if(++counter_seconds == 1000){
                counter_seconds = 0;

                temperature = read_temp(&crc_error);
                pc0->data = temperature;
                IO_off_d("led4");
                if(crc_error){
                    IO_on_d("led4");
                }
            }

            pc0->reserved = systick_triggered;

            if(led3_brightness < 1000){
                PWM_set_d("led3", led3_brightness / 10);
            } else if(led3_brightness > 1950){
                led3_brightness = 50;
            } else {
                PWM_set_d("led3", (2000 - led3_brightness) / 10);
            }
            ++led3_brightness;
            PWMBitbanged();
        }
    }
}

/* Service GPIO pins that use BitBanged PWM.
 * This should be called once per ms.
 */
void PWMBitbanged(void){
    static u32 countMs = 0;
    if(num_PWM_bitbanged == 0){
        return;
    }
    if(++countMs >= 1000){
        countMs = 0;
    }

    // updating every 20ms gives us 50Hz but only 20 set points.
    u32 i, value;
    for(i=0; i < num_PWM_bitbanged; ++i){
        value = pwm_bitbanged[i]->data;

        if((countMs % 20) < value / 5){
            IO_on_p(pwm_bitbanged[i]);
        } else {
            IO_off_p(pwm_bitbanged[i]);
        }
    }
}


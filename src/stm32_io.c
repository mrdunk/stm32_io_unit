#include "stm32_io.h"
#include "ds18s20.h"

#include <string.h>     // strncmp and strncpy

#include <sys/types.h>  // caddr_t
caddr_t _sbrk(int increment);


Port_Container_t* pwm_bitbanged[MAX_BITBAGED_PWM];
u32 num_PWM_bitbanged = 0;

void io_register(char descrip[DESCRIPTION_LEN], port_type_t type, port_numbers_t port_number){

    // Only initialise if it has not been done already.
    static int init = 1;
    if(init){
        init = 0;
        memset(containers, 0, sizeof(Port_Container_t) * MAX_CONTAINERS);
    }

    // Lookup expected data_size for this port type.
    u32 data_size = port_type_data_size[type];
    
    // Look through existing containers for one with same name.
    u32 container_itterate;
    Port_Container_t* p_container_empty = 0;
    Port_Container_t* p_container_match = 0;
    for(container_itterate = 0; container_itterate < MAX_CONTAINERS; ++ container_itterate){
        if(strncmp(containers[container_itterate].descrip, descrip, DESCRIPTION_LEN) == 0){
            p_container_match = &containers[container_itterate];
            if(p_container_match->type == PWM_BITBANG){
                // Sinc we are going to be ovewriting a PWM_BITBANG, we need to decrease the counter.
                --num_PWM_bitbanged;
            }
        }
        if(containers[container_itterate].descrip[0] == 0 && p_container_empty == 0){
            p_container_empty = &containers[container_itterate];
        }
    }

    if(p_container_match == 0){
        // Use first empty slot to store container.
        if(p_container_empty == 0){
            // Error. No free space.
            while(1);
        }
        p_container_match = p_container_empty;
        strncpy(p_container_match->descrip, descrip, DESCRIPTION_LEN);
    }

    p_container_match->type = type;
    p_container_match->port_number = port_number;
    p_container_match->data_write_itterator = 0;
    p_container_match->data_read_itterator = 0;

    if(*(u32*)p_container_match->p_data != data_size){
        io_data_alloc(p_container_match, data_size);
    }

    IO_Init(p_container_match);
}

u32* p_heap;
u32 heap_dirty;

u32 io_data_alloc(Port_Container_t* p_container, u32 set_data_size){

    // Only initialise if it has not been done already.
    static u32 init = 1;
    if(init){
        init = 0;
        p_heap = (u32*)_sbrk(HEAP_USAGE);
        memset(p_heap, 0, HEAP_USAGE * sizeof(u32));
        heap_dirty = 0;
    }

    u32 retval = 0;

    // Allign sizes on 4 byte boundaries.
    u32 set_container_size = (set_data_size +3) & ~3;
    u32 current_data_size = (*p_container->p_data +3) & ~3;

    if(p_container->p_data == 0){
        retval = _io_data_alloc_new(p_container, set_data_size);
    } else if(current_data_size > set_container_size){
        retval = _io_data_alloc_shrink(p_container, set_data_size);
    } else if(current_data_size < set_container_size){
        retval = _io_data_alloc_grow(p_container, set_data_size);
    }

    return retval;
}

u32 _io_data_alloc_new(Port_Container_t* p_container, u32 set_data_size){
    if(p_container->p_data == 0){
        // Allocate new space on heap for data.

        // Allign sizes on 4 byte boundaries.
        u32 set_container_size = (set_data_size +3) & ~3;

        u32* p_heap_check = p_heap;

        u32 empty_space;
        while(1){
            empty_space = 0;
            if(*p_heap_check == 0){
                // This space is empty. Let's see how much there is.
                while(*(++p_heap_check) == 0 && ++empty_space < set_container_size/4);
                if(empty_space >= set_container_size/4){
                    // Found enough space.
                    break;
                }
            } else {
                // No space here. Move to next posible space.
                p_heap_check += ((*p_heap_check +3) & ~3)/4 + 1;
            }
        }

        p_container->p_data = p_heap_check - empty_space;
        *p_container->p_data = set_data_size;
        return set_container_size;
    }

    return 0;
}

u32 _io_data_alloc_shrink(Port_Container_t* p_container, u32 set_data_size){

    // Allign sizes on 4 byte boundaries.
    u32 current_data_size = (*p_container->p_data +3) & ~3;
    u32 set_container_size = (set_data_size +3) & ~3;

    if(current_data_size > set_container_size){
        // Shrink allocation.
        *p_container->p_data = set_data_size;

        // Clear unused memory.
        memset(p_container->p_data + set_container_size/4 + 1, 0, current_data_size - set_container_size);

        // This leaves holes in the allocation.
        // TODO. Do some sort of garbage collaction to re-order data.
        ++heap_dirty;

        return set_container_size;
    }

    return 0;
}

u32 _io_data_alloc_grow(Port_Container_t* p_container, u32 set_data_size){

    // Align sizes on 4 byte boundaries.
    u32 current_data_size = (*p_container->p_data +3) & ~3;
    u32 set_container_size = (set_data_size +3) & ~3;

    if(current_data_size < set_container_size){
        // Grow allocation.

        // Check bytes following current allocation are not used.
        u32* p_check;
        for(p_check = p_container->p_data + current_data_size/4 + 1; p_check < p_container->p_data + set_container_size/4 + 1; ++p_check){
            if(*p_check){
                u32 retval;
                // Following bytes have been used so
                // 1. Create new alloc.
                u32* p_data_old = p_container->p_data;
                u32 data_size_old = *p_container->p_data;
                p_container->p_data = 0;
                retval = _io_data_alloc_new(p_container, set_data_size);
                // 2. Copy contents of old alloc into new one.
                memcpy(p_container->p_data +1, p_data_old +1, data_size_old);
                // 3. Scrap the original allocation.
                memset(p_data_old, 0, data_size_old + 4);

                // Scrapping old alloc leaves holes in the allocation.
                // TODO. Do some sort of garbage collaction to re-order data.
                ++heap_dirty;
                return retval;
            }
        }

        // Grow allocation.
        *p_container->p_data = set_data_size;
        return set_container_size;
    }
    return 0;
}

void IO_Init(Port_Container_t* io){
    static GPIO_InitTypeDef GPIO_InitStructure;

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
    } else if(io->type == BINARY_IN_OD || io->type == SERIAL_1_WIRE){
        /* Enable the BUTTON Clock */
        RCC_APB2PeriphClockCmd(GPIO_clk[io->port_number], ENABLE);

        /* Configure Button pin as input floating */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin = GPIO_pin[io->port_number];
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIO_bank[io->port_number], &GPIO_InitStructure);
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

    if(io->type == SERIAL_1_WIRE){
        // We are only interesed in running the 1-Wire interface at standard speed at this time.
        SetSpeed(1);

        one_wire_init_port(io);

        u32 size = 0;
        u32 rslt = OWFirst();
        while(rslt){
            size = *io->p_data;
            io_data_alloc(io, size + 16);
            io->data_write_itterator = size;
            *io->p_data = size + 16;
            int i;
            for (i = 0; i < 8; ++i){
                io_set_u8_p(io, ROM_NO[i]);
            }

            rslt = OWNext();
        }
    }
}

Port_Container_t* io_dectiption_to_p(char descrip[DESCRIPTION_LEN]){
    u32 container_itterate;
    for(container_itterate = 0; container_itterate < MAX_CONTAINERS; ++ container_itterate){
        if(strncmp(containers[container_itterate].descrip, descrip, DESCRIPTION_LEN) == 0){
            return &containers[container_itterate];
        }
    }
    return 0;
}

void io_clear(char descrip[DESCRIPTION_LEN]){
    io_clear_p(io_dectiption_to_p(descrip));
}

void io_clear_p(Port_Container_t* p_container){
    memset(p_container->p_data +1, 0, *p_container->p_data);
    p_container->data_write_itterator = 0;
}

void io_set_u32(char descrip[DESCRIPTION_LEN], u32 data){
    io_set_u32_p(io_dectiption_to_p(descrip), data);
}

void io_set_u32_p(Port_Container_t* p_container, u32 data){
    *(p_container->p_data + 1 + p_container->data_write_itterator/4) = data;
    p_container->data_write_itterator += 4;
    if(p_container->data_write_itterator >= *p_container->p_data){
        p_container->data_write_itterator = 0;
    }
}

void io_set_u8(char descrip[DESCRIPTION_LEN], u8 data){
    io_set_u8_p(io_dectiption_to_p(descrip), data);
}

void io_set_u8_p(Port_Container_t* p_container, u8 data){
    *((u8*)p_container->p_data + 4 + p_container->data_write_itterator) = data;
    ++p_container->data_write_itterator;
    if(p_container->data_write_itterator >= *p_container->p_data){
        p_container->data_write_itterator = 0;
    }
}

void io_update(char descrip[DESCRIPTION_LEN]){
    io_update_p(io_dectiption_to_p(descrip));
}

void io_update_p(Port_Container_t* p_container){
    //static u32 t1_init = 0;  // TODO
    //static u32 t2_init = 0;
    static u32 t3_init = 0;

    if(p_container){

        if(p_container->type == BINARY_OUT_PP || p_container->type == BINARY_OUT_OD){
            if(_data_read_u8(p_container)){
                GPIO_bank[p_container->port_number]->BSRR = GPIO_pin[p_container->port_number];
            } else {
                GPIO_bank[p_container->port_number]->BRR = GPIO_pin[p_container->port_number];
            }
        } else if(p_container->type == PWM){
            if(t3_init == 0 && (TIM_TypeDef *)PortTimerAlt[p_container->port_number][0] == TIM3){
                Timer_Init(TIM3);
                t3_init = 1;
            }

            // Make val operateover a range of 0-100.
            u32 val = _data_read_u8(p_container);
            val = (val * 666) / 100;

            static TIM_OCInitTypeDef  TIM_OCInitStructure;
            TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
            TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
            TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

            /* PWM1 Mode configuration: Channel1 */
            TIM_OCInitStructure.TIM_Pulse = val;

            if(PortTimer[p_container->port_number][0] != 0){
                //TODO
            } else if(PortTimerAlt[p_container->port_number][0] != 0){
                switch(PortTimerAlt[p_container->port_number][1])
                {
                    case 1:
                        TIM_OC1Init((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], &TIM_OCInitStructure);
                        TIM_OC1PreloadConfig((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], TIM_OCPreload_Enable);
                        break;
                    case 2:
                        TIM_OC2Init((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], &TIM_OCInitStructure);
                        TIM_OC2PreloadConfig((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], TIM_OCPreload_Enable);
                        break;
                    case 3:
                        TIM_OC3Init((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], &TIM_OCInitStructure);
                        TIM_OC3PreloadConfig((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], TIM_OCPreload_Enable);
                        break;
                    case 4:
                        TIM_OC4Init((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], &TIM_OCInitStructure);
                        TIM_OC4PreloadConfig((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], TIM_OCPreload_Enable);
                }
                // Use alternative pin.
                TIM_ARRPreloadConfig((TIM_TypeDef *)PortTimerAlt[p_container->port_number][0], ENABLE);
            }
        } else if(p_container->type == PWM_BITBANG){
            _data_read_u8(p_container);
        } else if(p_container->type == BINARY_IN_PU || p_container->type == BINARY_IN_PD || p_container->type == BINARY_IN_OD){
            io_set_u8_p(p_container, GPIO_bank[p_container->port_number]->IDR & GPIO_pin[p_container->port_number]);
        } else if(p_container->type == SERIAL_1_WIRE){
            one_wire_init_port(p_container);

            if(OWReadByte()){
                // Done converting temperature so now read it.

                // Align itterator on 16 byte boundaries.
                p_container->data_read_itterator = (p_container->data_read_itterator +15) & ~15;
                if(p_container->data_read_itterator >= *p_container->p_data){
                    p_container->data_read_itterator = 0;
                }

                u32 starting_read_itterator = p_container->data_read_itterator;
                
                int i;
                
                // Address a device.
                OWReset();
                OWWriteByte(0x55);
                for (i = 0; i < 8; ++i){
                    OWWriteByte(_data_read_u8(p_container));
                }
                
                // Read scratch register.
                int count;
                uint64_t scratchpad = 0;
                OWWriteByte(0xBE);
                for(count = 0; count < 8; ++count){
                    scratchpad |= ((uint64_t)OWReadByte() << (count * 8));
                }

                p_container->data_write_itterator = p_container->data_read_itterator;
                if(OWReadByte() != ds_crc(scratchpad)){
                    // CRC does not match.
                    // *error = 1;
                    io_set_u32_p(p_container, 0xffffffff);
                } else {
                    io_set_u32_p(p_container, scratchpad & 0xffff);
                }


                // Start new conversion.
                p_container->data_read_itterator = starting_read_itterator;
                OWReset();
                OWWriteByte(0x55);
                for (i = 0; i < 8; ++i){
                    OWWriteByte(_data_read_u8(p_container));
                }
                OWWriteByte(0x44);
            }
        }
    }
}

u32 _data_read_u32(Port_Container_t* p_container){
    u32 ret_val = *(p_container->p_data + 1 + p_container->data_read_itterator/4);
    p_container->data_read_itterator += 4;
    if(p_container->data_read_itterator >= *p_container->p_data){
        p_container->data_read_itterator = 0;
    }
    return ret_val;
}

u32 _data_read_u32_no_itterate(Port_Container_t* p_container){
    u32 ret_val = *(p_container->p_data + 1 + p_container->data_read_itterator/4);
    return ret_val;
}

inline u8 _data_read_u8(Port_Container_t* p_container){
    static u8 ret_val;
    ret_val = *((u8*)p_container->p_data + 4 + p_container->data_read_itterator);
    if(++p_container->data_read_itterator >= *p_container->p_data){
        p_container->data_read_itterator = 0;
    }                   
    return ret_val;
}               
                
u8 _data_read_u8_no_itterate(Port_Container_t* p_container){
    u8 ret_val = *((u8*)p_container->p_data + 4 + p_container->data_read_itterator);
    return ret_val;
}       

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

/* Service GPIO pins that use BitBanged PWM.
 * This is called once per ms from stm32f10x_it.c.
 */
void _PWMBitbanged(void){
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
        value = _data_read_u8_no_itterate(pwm_bitbanged[i]);

        if((countMs % 20) < value / 5){
            io_on_p(pwm_bitbanged[i]);
        } else {
            io_off_p(pwm_bitbanged[i]);
        }
    }
}

inline void io_on_p(Port_Container_t* p_container){
    GPIO_bank[p_container->port_number]->BSRR = GPIO_pin[p_container->port_number];
    //GPIO_SetBits(GPIO_bank[io->port_number], GPIO_pin[io->port_number]);
}

inline void io_off_p(Port_Container_t* p_container){
    GPIO_bank[p_container->port_number]->BRR = GPIO_pin[p_container->port_number];
    //GPIO_ResetBits(GPIO_bank[io->port_number], GPIO_pin[io->port_number]);
}

inline u32 io_read_p(Port_Container_t* p_container){
    return GPIO_bank[p_container->port_number]->IDR & GPIO_pin[p_container->port_number];
}

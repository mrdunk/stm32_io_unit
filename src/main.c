#include "stm32_io.h"

void trigger_50Hz(void);
void trigger_second(void);
void trigger_minute(void);

extern u32 systick_triggered;
u32 hz50_counter;
u32 second_counter;
u32 minute_counter;

int main(void)
{
    io_register("led3", PWM, PortC9);
    io_register("led4", BINARY_OUT_PP, PortC8);
    io_register("PA0", BINARY_IN_PD, PortA0);
    io_register("1wire", SERIAL_1_WIRE, PortC0);

    // These lines for debug only. We can use GDB to view.
    Port_Container_t* p_led3 = io_dectiption_to_p("led3");
    p_led3 = p_led3;

    Port_Container_t* p_led4 = io_dectiption_to_p("led4");
    p_led4 = p_led4;

    Port_Container_t* p_pa0 = io_dectiption_to_p("PA0");
    p_pa0 = p_pa0;

    Port_Container_t* p_pc0 = io_dectiption_to_p("1wire");
    p_pc0 = p_pc0;
   

    // Load valuse to turn led3 into nice pulsing led. 
    io_data_alloc(io_dectiption_to_p("led3"), 28);
    io_set_u8("led3", 8);
    io_set_u8("led3", 7);
    io_set_u8("led3", 6);
    io_set_u8("led3", 5);
    io_set_u8("led3", 6);
    io_set_u8("led3", 7);
    io_set_u8("led3", 8);
    io_set_u8("led3", 10);
    io_set_u8("led3", 12);
    io_set_u8("led3", 15);
    io_set_u8("led3", 25);
    io_set_u8("led3", 35);
    io_set_u8("led3", 50);
    io_set_u8("led3", 65);
    io_set_u8("led3", 75);
    io_set_u8("led3", 85);
    io_set_u8("led3", 90);
    io_set_u8("led3", 95);
    io_set_u8("led3", 90);
    io_set_u8("led3", 85);
    io_set_u8("led3", 75);
    io_set_u8("led3", 65);
    io_set_u8("led3", 50);
    io_set_u8("led3", 35);
    io_set_u8("led3", 25);
    io_set_u8("led3", 15);
    io_set_u8("led3", 12);
    io_set_u8("led3", 10);


    io_set_u32("led4", 1);
    io_update("led3");
    io_update("led4");

    systick_triggered = 0;
    second_counter = 0;
    minute_counter = 0;
    /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        /* Capture error */
        while (1);
    }


    while(1){

        if(systick_triggered){
            --systick_triggered;
            if(++hz50_counter >= 50){
                hz50_counter = 0;
                trigger_50Hz();
            }
            if(++second_counter >= 1000){
                second_counter = 0;
                trigger_second();
                if(++minute_counter >= 60){
                    minute_counter = 0;
                    trigger_minute();
                }
            }
        }

    }


    return 0;
}

void trigger_50Hz(void){
    static u32 toggle;
    if(++toggle == 2){
        toggle = 0;
        io_update("led3");
    }
}

void trigger_second(void){
    static u32 toggle;
    if(++toggle % 2){
    } else {
    }
    
    io_update("PA0");
    io_update("led4");
    io_update("1wire");
}

void trigger_minute(void){
}



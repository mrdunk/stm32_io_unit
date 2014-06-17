stm32_io_unit
=============

WORK IN PROGRESS.

STM32 dev board as dumb USB IO extender.

Firmware to turn the STMVLDiscovery into a USB IO extender capable of being controlled and queried by the ./st-flash binary (https://github.com/texane/stlink).


Rather than connect a USB to Serial converter to the dev board, we read data directly from memory using the programmer.
To do this we need to put the data in a known location in memory. As we do not intend to use maloc (or any other dynamic memory alocation) it is safe to use th linkers "_sheap" variable to assign a starting address for date we need to read later.



Instructions to build and work out start of data array for stlink to read:
make TOOLCHAIN_PATH=~/Working/EmbeddedArm/gcc-arm-none-eabi-4_8-2014q1/bin/ -f ./Makefile.Demo && /home/duncan/Working/EmbeddedArm/gcc-arm-none-eabi-4_8-2014q1/bin/arm-none-eabi-readelf -a ./Demo.elf  | grep _heap | head -n1 |awk '{ print $5; }' > heap_start && echo done

Instructions to flash:
$ ./st-flash write v1 ~/Working/git/stm32_io_unit/Demo.bin 0x8000000

Instructions to read directly from memory:
$ ./st-flash read /dev/stlinkv1_2 out.1 0x`cat ~/Working/git/stm32_io_unit/heap_start` 80 && hexdump -v out.1

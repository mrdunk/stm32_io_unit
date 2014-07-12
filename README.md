stm32_io_unit
=============

WORK IN PROGRESS.

STM32 dev board as dumb USB IO extender.

Firmware to turn the STMVLDiscovery into a USB IO extender capable of being controlled and queried by the ./st-flash binary (https://github.com/texane/stlink).


Rather than connect a USB to Serial converter to the dev board, we read data directly from memory using the programmer.
To do this we need to query the linker output to see where it has stored variables in memory:
    $ /home/duncan/Working/EmbeddedArm/gcc-arm-none-eabi-4_8-2014q1/bin/arm-none-eabi-readelf -a ./Demo.elf | grep container



Build the firmware:
    $ make TOOLCHAIN_PATH=~/Working/EmbeddedArm/gcc-arm-none-eabi-4_8-2014q1/bin/ 

Flash to STM32:
    $ ~/Working/git/stlink.git/st-flash write v1 ./Demo.bin 0x8000000

Instructions to read directly from memory:
    containers:
        Get addresses with:
            $ CONTAINERS=0x$(/home/duncan/Working/EmbeddedArm/gcc-arm-none-eabi-4_8-2014q1/bin/arm-none-eabi-readelf -a ./Demo.elf |grep containers |awk '{ print $2; }')
            $ echo $CONTAINERS

        Read from that address with:
            $ ~/Working/git/stlink.git/st-flash read v1 /tmp/out.1 $CONTAINERS 0x150

        Format according to struct Port_Container:
            $ hexdump -v -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 3/4 " %08x""\n"' /tmp/out.1

        From here, the 1st 32bit number is the pointer to the data array with the first u32 of that array containg the size.
            $ DATA=0x$(hexdump -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 3/4 " %08x""\n"' /tmp/out.1 | grep 1wire | awk '{ print $6; }')
            $ echo $DATA

        The first 32 bits of the data aray contain the array size:
            $ ~/Working/git/stlink.git/st-flash read v1 /tmp/out.1 $DATA 4
            $ SIZE=0x$(hexdump -e '1/4 "%08x""\n"' /tmp/out.1)
            $ echo $SIZE

        and the same address +4 will get the full array:
            $ let DATA=$DATA+4
            $ ~/Working/git/stlink.git/st-flash read v1 /tmp/out.1 $DATA $SIZE
            $ hexdump -e '1/4 " %08x""\n"' /tmp/out.1

#!/bin/bash

TARGETNAME=Demo
STFLASH=/bin/st-flash
STM32IO=/etc/stm32_io/

if [ $# != 1 ] && [ $# != 2 ]; then
    echo "Usage:"
    echo "    $0  <variable_to_search_for>  <any_output_data>"
    exit
fi

CONTAINER_NAME="$1"
OUTPUT_DATA="$2"

if [ ! -e $STM32IO$TARGETNAME.elf.txt ]; then
    echo "$STM32IO$TARGETNAME.elf.txt does not exist."
    exit
fi

# Get memory address of containers array.
P_CONTAINERS=0x$(grep containers $STM32IO$TARGETNAME.elf.txt |awk '{ print $2; }')
if [ $DEBUG ]; then
    echo "P_CONTAINERS = $P_CONTAINERS"
fi

# Dump the whole array of containers from memory into file.
$STFLASH read v1 /tmp/stm32_received.containers $P_CONTAINERS 0x150 &> /dev/null
if [ $DEBUG ]; then
    # The following doesn't work with busybox's implementation of hexdump:
    #     hexdump -v -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 4/3 " %08x""\n"' /tmp/stm32_received.containers
    hexdump -v -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 12/1 " %02x""\n"' /tmp/stm32_received.containers
fi

# Select only the container we are interesed in ($CONTAINER_NAME) and 
# get memory address of data array for this container.
P_DATA=0x$(hexdump -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 12/1 " %02x""\n"' /tmp/stm32_received.containers | grep $CONTAINER_NAME | awk '{ print $9$8$7$6; }')
CONTAINER_TYPE=$(hexdump -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 12/1 " %02x""\n"' /tmp/stm32_received.containers | grep $CONTAINER_NAME | awk '{ print $2; }')
if [ $DEBUG ]; then
    echo "P_DATA = $P_DATA"
    echo "CONTAINER_TYPE = $CONTAINER_TYPE"
fi

# Get first u32 data array. This value contains the size of the rest of the array.
$STFLASH read v1 /tmp/stm32_received.data_size $P_DATA 4 &> /dev/null
if [ $DEBUG ]; then
    hexdump -v -e '4/1 " %02x""\n"' /tmp/stm32_received.data_size | awk '{ print $4$3$2$1; }'
fi
DATA_SIZE=0x$(hexdump -v -e '4/1 " %02x""\n"' /tmp/stm32_received.data_size | awk '{ print $4$3$2$1; }')
if [ $DEBUG ]; then
    echo "DATA_SIZE = $DATA_SIZE"
fi

# Now fetch the data.
let P_DATA_4=$P_DATA+4; P_DATA_4=0x$(echo "obase=16; $P_DATA_4" | bc)
$STFLASH read v1 /tmp/stm32_received.data $P_DATA_4 $DATA_SIZE &> /dev/null
if [ $DEBUG ]; then
    echo "P_DATA_4 = $P_DATA_4"
    hexdump -C /tmp/stm32_received.data
fi

# Next bit dependant on what cointainer type.
if [ "$CONTAINER_TYPE" == "007" ]; then
    # type 7 = SERIAL_1_WIRE

    # Fetch the data.
    let P_DATA_4=$P_DATA+4; P_DATA_4=0x$(echo "obase=16; $P_DATA_4" | bc)
    $STFLASH read v1 /tmp/stm32_received.data $P_DATA_4 $DATA_SIZE &> /dev/null
    if [ $DEBUG ]; then
         echo "P_DATA_4 = $P_DATA_4"
         hexdump -C /tmp/stm32_received.data
    fi

    # Convert HEX into ASCII.
    hexdump -e '16/1 " %02x""\n"' /tmp/stm32_received.data | awk '{ print $7$6$5$4$3$2"\t"$1"\t"$10$9; }' > /tmp/stm32_received.sensor
    rm -f /tmp/stm32_received.staging
    while read LINE
        do
            OUT=0x$(echo $LINE | awk '{ print $3 }')
            let OUT_INT=$OUT/16
            let OUT_1DP=$(($OUT%16))*10/16
            echo "$(echo $LINE | awk '{ print $1 }'): $OUT_INT.$OUT_1DP" >> /tmp/stm32_received.staging
    done < /tmp/stm32_received.sensor
    mv /tmp/stm32_received.staging /tmp/stm32_received.final.$CONTAINER_NAME
    cat /tmp/stm32_received.final.$CONTAINER_NAME
elif [[ "$CONTAINER_TYPE" == "004" || "$CONTAINER_TYPE" == "005" || "$CONTAINER_TYPE" == "006" ]]; then
    # One of the input types.
    echo "input"
elif [[ "$CONTAINER_TYPE" == "000" || "$CONTAINER_TYPE" == "001" ]]; then
    # One of the output types.
    echo "output"
    echo $OUTPUT_DATA
    if [ -n "$OUTPUT_DATA" ]; then
        echo "  sending..."
        COUNT_ITEMS=0
        for ITEM in $OUTPUT_DATA; do
            let COUNT_ITEMS=$COUNT_ITEMS+1
            #echo $ITEM $COUNT_ITEMS
        done
        let DATA_SIZE=$DATA_SIZE    # Convert Hex format to Decimal.
        if [[ $COUNT_ITEMS != $DATA_SIZE ]]; then
            echo "Input data wrong size."
            echo "Expected $DATA_SIZE bytes."
            echo "Received $COUNT_ITEMS."
            exit
        fi
        SEND_DATA="00: "
        for ITEM in $OUTPUT_DATA; do
            let ITEM=$ITEM    # Convert Hex format to Decimal if required.
            SEND_DATA=$SEND_DATA$(printf " %02x" $ITEM)   # Convert back to hex
        done

        # Pad data out to 32 bit boundary
        if [ $(( $COUNT_ITEMS % 4 )) != 0 ]; then
            echo "XXX $(( 4 - $COUNT_ITEMS % 4 ))"
            for PADDING in $(seq $(( 4 - $COUNT_ITEMS % 4 ))); do
                echo $PADDING
                SEND_DATA=$SEND_DATA" 00"
            done
        fi

        echo $SEND_DATA
        echo $SEND_DATA | xxd -r > /tmp/stm32_to_send
        $STFLASH write v1 /tmp/stm32_to_send $P_DATA_4 &> /dev/null
    else
        echo "  reading..."
        # Fetch the data.
        let P_DATA_4=$P_DATA+4; P_DATA_4=0x$(echo "obase=16; $P_DATA_4" | bc)
        $STFLASH read v1 /tmp/stm32_received.data $P_DATA_4 $DATA_SIZE &> /dev/null
        if [ $DEBUG ]; then
            echo "P_DATA_4 = $P_DATA_4"
            hexdump -C /tmp/stm32_received.data
        fi
    fi
else
    # Some other type.

    # Fetch the data.
    let P_DATA_4=$P_DATA+4; P_DATA_4=0x$(echo "obase=16; $P_DATA_4" | bc)
    $STFLASH read v1 /tmp/stm32_received.data $P_DATA_4 $DATA_SIZE &> /dev/null
    if [ $DEBUG ]; then
        echo "P_DATA_4 = $P_DATA_4"
        hexdump -C /tmp/stm32_received.data
    fi

    # Convert HEX into ASCII.
    rm -f /tmp/stm32_received.staging
    hexdump -e '16/1 " %02x""\n"' /tmp/stm32_received.data > /tmp/stm32_received.staging
    mv /tmp/stm32_received.staging /tmp/stm32_received.final.$CONTAINER_NAME
fi


#!/bin/sh

source output_json.sh

TARGETNAME=Demo
STFLASH=/bin/st-flash
STM32IO=/etc/stm32_io/
DBHOST=192.168.192.254

log(){
    if [ $DEBUG ]; then
        echo "$1"
    fi
}

# Calculate size and start of data referenced by pointer.
get_data_dimensions(){
    P_DATA=$1          # Pointer to start of memory containing data.
    local __DATA_SIZE=$2 # Variable to contain the name of the variable to store the data in.
    local __DATA_START=$3 # Variable to contain the name of the pointer to the start of the data.

    # Get first u32 data array. This value contains the size of the rest of the array.
    $STFLASH read v1 /tmp/stm32_received.data_size $P_DATA 4 &> /dev/null
    local __TMP_DATA_SIZE=0x$(hexdump -v -e '4/1 " %02x""\n"' /tmp/stm32_received.data_size | awk '{ print $4$3$2$1; }')
    
    let local __TMP_P_DATA_START=$P_DATA+4;
    __TMP_P_DATA_START=0x$(echo "obase=16; $__TMP_P_DATA_START" | bc)

    eval $__DATA_SIZE="$__TMP_DATA_SIZE"
    eval $__DATA_START="$__TMP_P_DATA_START"

    log "__TMP_DATA_SIZE = $__TMP_DATA_SIZE"
    log "__TMP_P_DATA_START = $__TMP_P_DATA_START"
}

# Read data directly from memory.
fetch_data(){
    P_DATA=$1   # Pointer to start of memory containing data.
    FILENAME=$2 # Filename to save data in.

    get_data_dimensions $P_DATA DATA_SIZE P_DATA_START
    log "P_DATA_START = $P_DATA_START"

    # Now fetch the data.
    $STFLASH read v1 $FILENAME $P_DATA_START $DATA_SIZE &> /dev/null

    if [ $DEBUG ]; then
        echo "Data in $FILENAME:"
        hexdump -C $FILENAME
    fi
}


if [ $# != 1 ] && [ $# != 2 ]; then
    echo "Usage:"
    echo "    $0  <variable_to_search_for>  <any_output_data>"
    exit
fi

HOSTNAME=$(hostname) 2> /dev/null
if [ $? -ne 0 ]; then
    HOSTNAME=$(grep hostname /etc/config/system | cut -d "'" -f 2)
    if [ $? -ne 0 ]; then
        HOSTNAME="unknown"
    fi
fi

CONTAINER_NAME="$1"
OUTPUT_DATA="$2"

if [ ! -e $STM32IO$TARGETNAME.elf.txt ]; then
    echo "$STM32IO$TARGETNAME.elf.txt does not exist."
    exit
fi

# Get memory address of containers array.
P_CONTAINERS=0x$(grep containers $STM32IO$TARGETNAME.elf.txt |awk '{ print $2; }')
log "P_CONTAINERS = $P_CONTAINERS"

# Dump the whole array of containers from memory into file.
$STFLASH read v1 /tmp/stm32_received.containers $P_CONTAINERS 0x150 &> /dev/null
# The following doesn't work with busybox's implementation of hexdump:
#     hexdump -v -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 4/3 " %08x""\n"' /tmp/stm32_received.containers | log
if [ $DEBUG ]; then
    hexdump -v -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 12/1 " %02x""\n"' /tmp/stm32_received.containers
fi

# Select only the container we are interesed in ($CONTAINER_NAME) and 
# get memory address of data array for this container.
P_DATA=0x$(hexdump -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 12/1 " %02x""\n"' /tmp/stm32_received.containers | grep $CONTAINER_NAME | awk '{ print $9$8$7$6; }')
CONTAINER_TYPE=$(hexdump -e '12/1 "%_p""\t" 2/1 " %03d" 2/1 " %00x" 12/1 " %02x""\n"' /tmp/stm32_received.containers | grep $CONTAINER_NAME | awk '{ print $2; }')
log "P_DATA = $P_DATA"
log "CONTAINER_TYPE = $CONTAINER_TYPE"

## Get first u32 data array. This value contains the size of the rest of the array.
#$STFLASH read v1 /tmp/stm32_received.data_size $P_DATA 4 &> /dev/null
#DATA_SIZE=0x$(hexdump -v -e '4/1 " %02x""\n"' /tmp/stm32_received.data_size | awk '{ print $4$3$2$1; }')
#log "DATA_SIZE = $DATA_SIZE"


# Next bit dependant on what cointainer type.
if [ "$CONTAINER_TYPE" == "007" ]; then
    log "IO type: 1wire"

    fetch_data $P_DATA /tmp/stm32_received.data

    # Convert HEX into ASCII.
    hexdump -e '16/1 " %02x""\n"' /tmp/stm32_received.data | awk '{ print $7$6$5$4$3$2"\t"$1"\t"$10$9; }' > /tmp/stm32_received.sensor
    open_JSON /tmp/stm32_$CONTAINER_NAME.json
    while read LINE
        do
            OUT=0x$(echo $LINE | awk '{ print $3 }')
            let OUT_INT=$OUT/16
            let OUT_1DP=$(($OUT%16))*10/16

            KEY="$(echo $LINE | awk '{ print $1 }')"
            VAL="$OUT_INT.$OUT_1DP"

            compile_JSON /tmp/stm32_$CONTAINER_NAME.json $HOSTNAME $CONTAINER_NAME $KEY $VAL

    done < /tmp/stm32_received.sensor
    close_JSON /tmp/stm32_$CONTAINER_NAME.json
    curl -X POST -d @/tmp/stm32_$CONTAINER_NAME.json http://$DBHOST:1080/1.0/event/put

elif [[ "$CONTAINER_TYPE" == "004" || "$CONTAINER_TYPE" == "005" || "$CONTAINER_TYPE" == "006" ]]; then
    # One of the input types.
    log "IO type: input"
    # TODO
elif [[ "$CONTAINER_TYPE" == "000" || "$CONTAINER_TYPE" == "001" ]]; then
    # One of the output types.
    log "IO type: output"
    if [ -n "$OUTPUT_DATA" ]; then
        log "  sending..."
        COUNT_ITEMS=0
        for ITEM in $OUTPUT_DATA; do
            let COUNT_ITEMS=$COUNT_ITEMS+1
            #echo $ITEM $COUNT_ITEMS
        done

        get_data_dimensions $P_DATA DATA_SIZE P_DATA_START

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
            for PADDING in $(seq $(( 4 - $COUNT_ITEMS % 4 ))); do
                SEND_DATA=$SEND_DATA" 00"
            done
        fi

        log "SEND_DATA = $SEND_DATA"
        echo $SEND_DATA | xxd -r > /tmp/stm32_to_send
        $STFLASH write v1 /tmp/stm32_to_send $P_DATA_START &> /dev/null
    else
        log "  reading..."
        fetch_data $P_DATA /tmp/stm32_received.data
    fi
else
    log "IO type: other"

    fetch_data $P_DATA /tmp/stm32_received.data

    # Convert HEX into ASCII.
    rm -f /tmp/stm32_received.staging
    hexdump -e '16/1 " %02x""\n"' /tmp/stm32_received.data > /tmp/stm32_received.staging
    mv /tmp/stm32_received.staging /tmp/stm32_received.final.$CONTAINER_NAME
fi


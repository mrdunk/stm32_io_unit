#!/bin/sh

source output_json.sh
source hostname.sh

DHCPFILE=/var/dhcp.leases
OUTFILE=/tmp/active_dhcp_clients.json
HOSTNAME=$(get_hostname)
DBHOST=192.168.192.254

open_JSON $OUTFILE
while read LINE; do
    MAC=$(echo $LINE | cut -d" " -f2);
    IP=$(echo $LINE | cut -d" " -f3);
    echo $MAC $IP
    ping -q -c1 -w1 $IP
    #arping -c2 -w1 -f -I br-lan 192.168.192.49
    if [ $? -eq 0 ]; then
        # Sucessfully pinged $IP
        compile_JSON $OUTFILE $HOSTNAME "net_clients" $MAC $IP;
    fi
done < $DHCPFILE
close_JSON $OUTFILE

curl -X POST -d @$OUTFILE http://$DBHOST:1080/1.0/event/put

#!/bin/ash
#
# Executed by udev through /etc/udev/rules.d/50-core.rules
# 
#
serial=`cat /sys/${PHYSDEVPATH}/../../serial`
manufacturer=`cat /sys/${PHYSDEVPATH}/../../manufacturer`
product=`cat /sys/${PHYSDEVPATH}/../../product`
vendor_id=`cat /sys/${PHYSDEVPATH}/../../idVendor`
product_id=`cat /sys/${PHYSDEVPATH}/../../idProduct`
cat << _EOF_ >> /dev/ttyusb_fifo &
${ACTION}
${TTY}
${manufacturer}
${product}
${vendor_id}
${product_id}
${serial}
_EOF_

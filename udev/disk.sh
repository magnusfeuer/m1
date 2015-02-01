#!/bin/ash
#
# Executed by udev through /etc/udev/rules.d/50-core.rules
#
#!/bin/ash
#
# Executed by udev through /etc/udev/rules.d/50-core.rules
#
if [ "$ACTION" = "add" ]
then
        mount  $DISK /mnt
	echo "/mnt" > /dev/packet_fifo
fi

if [ "$ACTION" = "remove" ]
then
        umount  /mnt 
fi


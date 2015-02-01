#!/bin/sh
#
# All rights reserved. Reproduction, modification, use or disclosure
# to third parties without express authority is forbidden.
# Copyright Magden LLC, California, USA, 2004, 2005, 2006, 2007.
#
# Create a USB factory stick.
# When booted from this USB memory stick, the m1 will retrieve a complete setup
# and install everything in an encrypted and nice way.
#
#

export TARGET=jway
export KERNEL_VERSION=2.6.25.4

if [ $# -lt 7 ]
then
    echo "Usage $0 disk part_nr stick-ip-address trusted-db-ip-address"
    echo "disk -                  The master device to install on, with no partition number. (/dev/sda for example)."
    echo "part_nr -               The finished part number of the M.1. See trusted.part table"
    echo "stick-ip-address -      The IP address of the M.1 booting the created stick. 192.168.0.20"
    echo "trusted-db-ip-address - The IP address of database server hosting the magdenus_trusted database. 192.168.0.170"
    echo "os id -                 The version of the os packfile to use in the database"
    echo "m1_bin id -             The version of the m1_bin packfile to use in the database."
    echo "m1_core id      -       The version of the m1_core packfile to use in the database."
    exit 255
fi

export M1_DISK=$1
export PART_NR=$2
export M1_IP=$3
export DB_IP=$4
export OS_ID=$5
export M1_ID=$6
export CORE_ID=$7
if [ "`id -u`" != "0" ]
then
    echo "You have to be root to execute $0"
    exit 255
fi


LOG=create_factory.log

echo "$0 "'date' > $LOG
echo "--- Log file can be found in create_master.log"
echo "--- Using kernel version ${KERNEL_VERSION}-${TARGET}"
echo "--- Using disk $M1_DISK"

sync
sync
sync
sync
umount $M1_DISK > /dev/null 2>&1
umount ${M1_DISK}1 > /dev/null 2>&1
umount $M1_DISK > /dev/null 2>&1
umount ${M1_DISK}1 > /dev/null 2>&1

# Reset old directory.
rm -rf /mnt/master > /dev/null  
rm -rf /mnt/loopback > /dev/null  
mkdir /mnt/master 
mkdir /mnt/loopback > /dev/null  2>&1
mkdir /mnt/master > /dev/null  2>&1

#
# Check if we are creating a new disk
#
#
# Setup the partition and install syslinux boot strap.
#
PATH=$PATH:$PWD/master_tools

echo "--- Creating FAT root file system on USB stick"
dd if=/dev/zero of=${M1_DISK} bs=512 count=1 >> $LOG  2>&1
master_tools/mkdosfs -I ${M1_DISK} >> $LOG  2>&1
$PWD/master_tools/syslinux ${M1_DISK} >> $LOG  2>&1

umount $M1_DISK  >> $LOG  2>&1
umount ${M1_DISK}1  >> $LOG  2>&1
umount $M1_DISK  >> $LOG  2>&1
umount ${M1_DISK}1  >> $LOG  2>&1

mount ${M1_DISK} /mnt/master  >> $LOG  2>&1

# Install linux images.
if [ ! -f kernel/linux-${KERNEL_VERSION}-${TARGET}/vmlinux ]
then
    echo "--- No kernel has been built."
    echo "--- Do:"
    echo "--- cd kernel; make; make install"
    echo "--- and try again."
    exit 255
fi

echo "--- Creating initrd file system"
dd if=/dev/zero of=initrd.tmp count=20000 bs=1024  >> $LOG  2>&1
sync
sync
sync
# Mount it
losetup /dev/loop0 initrd.tmp   >> $LOG  2>&1
mke2fs /dev/loop0  >> $LOG 2>&1 
/sbin/tune2fs -c0 -i0 /dev/loop0  >> $LOG 2>&1
mount /dev/loop0 /mnt/loopback  >> $LOG 

# Copy linuxrc script, which does the install
cp  factory_linuxrc /mnt/loopback/linuxrc
chmod 777 /mnt/loopback/linuxrc

echo "--- Installing linux images."

#
# Copy bzImage and initrd to /mnt
#
cp kernel/linux-${KERNEL_VERSION}-${TARGET}/arch/i386/boot/bzImage /mnt/master/LINUX

#Install linux modules
echo "--- Installing modules and other kernel components in initrd."
cp -a images-${KERNEL_VERSION}-${TARGET}/* /mnt/loopback/

# 
# Install root file system on USB stick.
#
echo "--- Installing master file system skeleton image into initrd."
MASTER_SKELETON=$PWD/master_skeleton.tgz
(cd /mnt/loopback; tar xzpf $MASTER_SKELETON)

echo "--- Populating root file system with busybox build into initrd"
BUSYBOX_PATH=$PWD/busybox/busybox_image.tgz
(cd /mnt/loopback; tar xpf $BUSYBOX_PATH)

#
# Install station ID and IP address to use in linuxrc root
#
echo $PART_NR > /mnt/loopback/part_nr.txt
echo $M1_IP > /mnt/loopback/ip_address.txt
echo $DB_IP > /mnt/loopback/db_address.txt
echo $OS_ID > /mnt/loopback/os.txt
echo $M1_ID > /mnt/loopback/m1.txt
echo $CORE_ID > /mnt/loopback/core.txt
#
# Copy the nvram BIOS settings to the stick.
#
cp bios_settings.img /mnt/loopback/bios_settings.img

#
# Add specific executables we need.
#
for file in add_m1 db2fs keytool
do
    cp ./bin/${file} /mnt/loopback/bin
done

#
# copy demo and magden.pub key which is installed in m1e binary by default.
# Will be used by patchkey to patch binary with new device key.
#


echo "--- Creating master grub used to install bootstrap on target."
mkdir -p /mnt/loopback/boot/grub
cp grub/* /mnt/loopback/boot/grub
mkdir /mnt/loopback/fat
umount /mnt/loopback  >> $LOG 
losetup -d /dev/loop0  >> $LOG 

echo "--- Copying initrd to USB stick."
gzip -c initrd.tmp > /mnt/master/INITRD

echo "--- Setting up syslinux.cfg boot loader config."
cat <<EOF > /mnt/master/syslinux.cfg
DEFAULT LINUX
APPEND initrd=INITRD ramdisk_size=20000 video=viafb:mode=640x480,bpp=32,active_dev=TV+CRT,tv_dedotcrawl=1 quiet
PROMPT 0
EOF
rm initrd.tmp

echo "--- Synching and unmounting."
sync
umount /mnt/master

echo "Done."

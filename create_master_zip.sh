#!/bin/sh
#
# All rights reserved. Reproduction, modification, use or disclosure
# to third parties without express authority is forbidden.
# Copyright Magden LLC, California, USA, 2004, 2005, 2006, 2007.
#
# Create a master USB memory stick that is bootable by an m1 bootloader.
# When booted from this USB memory stick, an M1 computer will have its system re-installed.
#

export TARGET=jway
#export KERNEL_VERSION=2.6.21.5
#export KERNEL_VERSION=2.6.23.14
#export KERNEL_VERSION=2.6.24
export KERNEL_VERSION=2.6.25.4


if [ $# -lt 1 ]
then
    echo "Usage $0 serial"
    echo "Disk is the master device to install on, with no partition number. (/dev/sda for example)."
    echo "The skin, plugins, dds and launch files under out will be installed."
    exit 255
fi

export M1_SERIAL=$1
ZIP_FILE=$PWD/master-${M1_SERIAL}.zip


if [ "`id -u`" != "0" ]
then
    echo "You have to be root to execute $0"
    exit 255
fi


LOG=create_master_zip.log

echo "$0 "'date' > $LOG
echo "--- Log file can be found in create_master.log"
echo "--- Using kernel version ${KERNEL_VERSION}-${TARGET}"
echo "--- Will create zip file $ZIP_FILE"



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


# Install linux images.
if [ ! -f kernel/linux-${KERNEL_VERSION}-${TARGET}/vmlinux ]
then
    echo "--- No kernel has been built."
    echo "--- Do:"
    echo "--- cd kernel; make; make install"
    echo "--- and try again."
    exit 255
fi

echo "--- Create initrd file system"
dd if=/dev/zero of=initrd.tmp count=7500 bs=1024  >> $LOG  2>&1
sync
sync
sync
# Mount it
losetup /dev/loop0 initrd.tmp   >> $LOG  2>&1
mke2fs /dev/loop0  >> $LOG 2>&1 
/sbin/tune2fs -c0 -i0 /dev/loop0  >> $LOG 2>&1
mount /dev/loop0 /mnt/loopback  >> $LOG 

# Copy linuxrc script, which does the install
cp  master_linuxrc /mnt/loopback/linuxrc
chmod 777 /mnt/loopback/linuxrc

#
# Copy bzImage and initrd to /mnt
#
cp kernel/linux-${KERNEL_VERSION}-${TARGET}/arch/i386/boot/bzImage /mnt/master/LINUX

#Install linux modules
cp -a images-${KERNEL_VERSION}-${TARGET}/* /mnt/loopback/

# 
# Install root file system on USB stick.
#
MASTER_SKELETON=$PWD/master_skeleton.tgz
(cd /mnt/loopback; tar xzpf $MASTER_SKELETON)

BUSYBOX_PATH=$PWD/busybox/busybox_image.tgz
(cd /mnt/loopback; tar xpf $BUSYBOX_PATH)

# Add packfile unpacker app.
cp ./bin/pf2fs /mnt/loopback/bin

echo "--- Creating master grub used to install bootstrap on target."
mkdir -p /mnt/loopback/boot/grub
cp grub/* /mnt/loopback/boot/grub
mkdir /mnt/loopback/fat
echo $M1_SERIAL > /mnt/loopback/serial.txt
# Umount loopbackdisk
umount /mnt/loopback  >> $LOG 
losetup -d /dev/loop0  >> $LOG 

echo "--- Copying initrd to USB stick."
gzip -c initrd.tmp > /mnt/master/INITRD


echo "--- Generating device specific binary."
./bin/db2pf -i m1@magden-auto.com/m1_bin/1.0.0 -s ${M1_SERIAL} -P -o /mnt/master/m1bin.pfl

echo "--- Generating device specific binary."
./bin/db2pf -i m1@magden-auto.com/os/1.0.0  -o /mnt/master/os.pfl

echo "--- Generating magden encrypted core."
./bin/db2pf -i m1@magden-auto.com/m1_core/1.0.0 -k magden -S .m1 -o /mnt/master/m1core.pfl

echo "--- Zipping it up to $ZIP_FILE"

(cd /mnt/master; zip $ZIP_FILE *)
echo "--- Done."

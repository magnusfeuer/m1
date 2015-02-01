#!/bin/sh
#
# All rights reserved. Reproduction, modification, use or disclosure
# to third parties without express authority is forbidden.
# Copyright Magden LLC, California, USA, 2004, 2005, 2006, 2007.
#
# Create a self extractable image that can be dragged and dropped to an USB stick.
# When an M1 with a functional boot loader (and nothing else required) is booted
# It will read the linux and initrd from the USB stick and install OS and all packfiles.
#

. ./Version.inc
export TARGET=jway
#export KERNEL_VERSION=2.6.21.5
#export KERNEL_VERSION=2.6.23.14
#export KERNEL_VERSION=2.6.24
export KERNEL_VERSION=2.6.25.4
export ZIPDIR=ziptmp

if [ $# -lt 1 ]
then
    echo "Usage $0 output-zip-file"
    echo "The skin, plugins, dds and launch files under packfiles will be installed."
    exit 255
fi

export ZIPFILE=$1

if [ "`id -u`" != "0" ]
then
    echo "You have to be root to execute $0"
    exit 255
fi


LOG=create_zip.log

echo "$0 "'date' > $LOG
echo "--- Log file can be found in create_master.log"
echo "--- Using kernel version ${KERNEL_VERSION}-${TARGET}"
echo "--- Output zip file is $ZIPFILE"

echo "--- Building a new m1e binary packfile"
(cd .core; make pf)


# Reset old directory.

rm -rf /mnt/loopback > /dev/null  
mkdir /mnt/loopback > /dev/null 2>&1
rm -rf ${ZIPDIR} > /dev/null 
mkdir ${ZIPDIR} > /dev/null  2>&1

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

echo "--- Creating initrd file system"
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

echo "--- Installing linux images."

#
# Copy bzImage and initrd to /mnt
#
cp kernel/linux-${KERNEL_VERSION}-${TARGET}/arch/i386/boot/bzImage ${ZIPDIR}/linux

#Install linux modules
echo "--- Installing modules and other kernel components in initrd."
cp -a images-${KERNEL_VERSION}-${TARGET}/* /mnt/loopback/

# 
# Install root file system in initrd
#
echo "--- Installing master file system skeleton image into initrd."
MASTER_SKELETON=$PWD/master_skeleton.tgz
(cd /mnt/loopback; tar xzpf $MASTER_SKELETON)

echo "--- Populating root file system with busybox build into initrd"
BUSYBOX_PATH=$PWD/busybox/busybox_image.tgz
(cd /mnt/loopback; tar xpf $BUSYBOX_PATH)
# Add packfile unpacker app.
cp ./bin/pf2fs /mnt/loopback/bin
echo "--- Creating master grub used to install bootstrap on target."
mkdir -p /mnt/loopback/boot/grub
cp grub/* /mnt/loopback/boot/grub
mkdir /mnt/loopback/fat

# Unmount loopback disk
umount /mnt/loopback  >> $LOG 
losetup -d /dev/loop0  >> $LOG 

echo "--- Copying initrd to zip target."
gzip -c initrd.tmp > ${ZIPDIR}/initrd

sh create_os_packfile.sh

echo "--- Copying all pack files to zip target."
cp packfiles/*.pfl ${ZIPDIR}

echo "--- Building zip file at ${ZIPFILE}"
(cd ${ZIPDIR}; zip -r ../${ZIPFILE} *)
echo "Done."

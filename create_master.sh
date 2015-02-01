#!/bin/sh
#
# All rights reserved. Reproduction, modification, use or disclosure
# to third parties without express authority is forbidden.
# Copyright Magden LLC, California, USA, 2004, 2005, 2006, 2007.
#
# Create a master USB memory stick disk
# When booted from this USB memory stick, an M1 computer will have all its hardware installed.
#

. ./Version.inc
export TARGET=jway
#export KERNEL_VERSION=2.6.21.5
#export KERNEL_VERSION=2.6.23.14
#export KERNEL_VERSION=2.6.24
export KERNEL_VERSION=2.6.25.4


if [ $# -lt 1 ]
then
    echo "Usage $0 disk"
    echo "Disk is the master device to install on, with no partition number. (/dev/sda for example)."
    echo "The skin, plugins, dds and launch files under out will be installed."
    exit 255
fi

export M1_DISK=$1

if [ "$M1_DISK" = "/dev/sda" ] 
then
	echo "You don't want to do that...."
	exit 255
fi

if [ "`id -u`" != "0" ]
then
    echo "You have to be root to execute $0"
    exit 255
fi


LOG=create_master.log

echo "$0 "'date' > $LOG
echo "--- Log file can be found in create_master.log"
echo "--- Using kernel version ${KERNEL_VERSION}-${TARGET}"
echo "--- Using disk $M1_DISK"

echo "--- Building a new m1e binary packfile"
(cd core; make pf)

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
if [ ! -f $BUSYBOX_PATH ] 
then
    (cd busybox; make)
fi

(cd /mnt/loopback; tar xpf $BUSYBOX_PATH)
# Add packfile unpacker app.
cp ./bin/pf2fs /mnt/loopback/bin
echo "--- Creating master grub used to install bootstrap on target."
mkdir -p /mnt/loopback/boot/grub
cp grub/* /mnt/loopback/boot/grub
mkdir /mnt/loopback/fat

# Umount loopbackdisk
umount /mnt/loopback  >> $LOG 
losetup -d /dev/loop0  >> $LOG 

echo "--- Copying initrd to USB stick."
gzip -c initrd.tmp > /mnt/master/INITRD

sh create_os_packfile.sh ./packfiles/os.pfl

echo "--- Copying all packfiles to usb stick.."
#(cd packfiles; tar czf /mnt/master/pfiles.tgz *.pfl)
cp packfiles/*.pfl /mnt/master/

echo "--- Setting up syslinux.cfg boot loader config."
cat <<EOF > /mnt/master/syslinux.cfg
DEFAULT LINUX
APPEND initrd=INITRD ramdisk_size=7500 video=viafb:mode=640x480,bpp=32,active_dev=TV+CRT,tv_dedotcrawl=1 quiet
PROMPT 0
EOF
rm initrd.tmp

#chmod 755 /mnt/master/sbin/init
#rm -f /mnt/master/sbin/init.*
echo "--- Synching and unmounting."
sync
umount /mnt/master

echo "Done."

#
# OS packfile
#

export TARGET=jway
# export KERNEL_VERSION=2.6.21.5
# export KERNEL_VERSION=2.6.23.14
#export KERNEL_VERSION=2.6.24
export KERNEL_VERSION=2.6.25.4

. ./Version.inc

if [ $# -lt 1 ]
then
    echo "Usage $0 out-packfile"
    exit 255
fi

BUSYBOX_PATH=$PWD/busybox/busybox_image.tgz


if [ ! -f $BUSYBOX_PATH ] 
then
    (cd busybox; make)
fi

echo "--- Creating a target file system tree."
rm -rf target_fs > /dev/null 
mkdir target_fs
mkdir target_fs/boot 

echo "---   Copying target file system skeleton to target file system"
ROOTFS_SKELETON=$PWD/rootfs_skeleton.tgz
(cd target_fs; tar xzpf $ROOTFS_SKELETON)

echo "---   Copying target file system busybox to target file system"
(cd target_fs; tar xpf $BUSYBOX_PATH)


echo "--- Populating root file system with udev system"
(cw=$PWD;cd udev; make install UDEV_ROOT=$cw/target_fs)

echo "---   Copying init launch script to target file system"
cp rootfs_init target_fs/sbin/init
chmod 755 target_fs/sbin/init

echo "---   Copying kernel to target file system."
cp kernel/linux-${KERNEL_VERSION}-${TARGET}/.config target_fs/boot/${KERNEL_VERSION}.config
cp kernel/linux-${KERNEL_VERSION}-${TARGET}/arch/i386/boot/bzImage target_fs/boot/bzImage-${KERNEL_VERSION}

echo "---   Copying kernel modules to target file system."
cp -a images-${KERNEL_VERSION}-${TARGET}/* target_fs

echo "---   Installing boot loader files in target file system."
mkdir -p target_fs/boot/grub
cp grub/*  target_fs/boot/grub
echo "(hd0) /dev/hda" > target_fs/boot/grub/device.map

cp target_fs/boot/grub/grub.conf target_fs/boot/grub/menu.lst # To make suse grub happy

echo "--- Packing up target file system."
target=$PWD/$1
(cd target_fs; ../bin/fs2pf -o$target -im1@magden-auto.com/os/$VERSION . )

rm -rf target_fs


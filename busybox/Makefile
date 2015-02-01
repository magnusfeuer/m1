#
# Makefile to build busybox.
# 

BUSYBOX_VERSION=1.4.1
#GCC_PATH=$(shell pwd)/../i686-pc-linux-gnu/gcc-3.4.4/bin/

busybox_image.tgz: busybox-$(BUSYBOX_VERSION)/_install/bin/busybox
	@echo "-- Building image"
	(cd busybox-$(BUSYBOX_VERSION)/_install; tar czf ../../busybox_image.tgz .)

busybox-$(BUSYBOX_VERSION)/_install/bin/busybox: busybox-$(BUSYBOX_VERSION)/README
	@echo "-- Building busybox."
#	(cd busybox-$(BUSYBOX_VERSION); make CROSS_COMPILE=$(GCC_PATH); make install CROSS_COMPILE=$(GCC_PATH))
	(cd busybox-$(BUSYBOX_VERSION); make ; make install)


busybox-$(BUSYBOX_VERSION)/README:
	@echo "-- Unpacking busybox."
	@tar jxf busybox-$(BUSYBOX_VERSION).tar.bz2
	@cp busybox-$(BUSYBOX_VERSION).config busybox-$(BUSYBOX_VERSION)/.config
	@cp busybox-$(BUSYBOX_VERSION).Makefile busybox-$(BUSYBOX_VERSION)/Makefile



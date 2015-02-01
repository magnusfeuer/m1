#
# Makefile for doing the udev thing.
#

UDEV_VER=118

# Builder
udev-$(UDEV_VER)/udevd: udev-$(UDEV_VER)/README
	@echo
	@echo "------ Building udev version $(UDEV_VER)"
	(cd udev-$(UDEV_VER); make)
	(cd udev-$(UDEV_VER); strip test-udev udevd udevadm)

# Unpacker
udev-$(UDEV_VER)/README:
	@echo
	@echo "------ Unpacking udev version $(UDEV_VER)"
	tar xzf udev-$(UDEV_VER).tar.gz

install: udev-$(UDEV_VER)/udevd
	@echo
	@echo "------ Installing udev version $(UDEV_VER) in root $(UDEV_ROOT)"
	@-mkdir -p $(UDEV_ROOT)/lib/udev/devices
#       Unpack defaulte devices into devices directory
	@(cw=$(shell pwd); cd $(UDEV_ROOT)/lib/udev/devices; tar xzf $$cw/default_devices.tgz)
	@-mkdir -p $(UDEV_ROOT)/etc/udev/rules.d
#	Copy config
	@cp udev.conf $(UDEV_ROOT)/etc/udev/
#       Copy core rules
	@cp 50-core.rules $(UDEV_ROOT)/etc/udev/rules.d
#	Copy disk.sh and tty.sh
	@cp disk.sh $(UDEV_ROOT)/etc/udev/
	@chmod 755 $(UDEV_ROOT)/etc/udev/disk.sh
	@cp tty.sh $(UDEV_ROOT)/etc/udev/
	@chmod 755 $(UDEV_ROOT)/etc/udev/tty.sh
# 	Copy binaries
	@-mkdir $(UDEV_ROOT)/sbin > /dev/null 2>&1
	@(cd udev-$(UDEV_VER); cp test-udev udevd udevadm $(UDEV_ROOT)/sbin)

clean:
	rm -rf udev-$(UDEV_VER)

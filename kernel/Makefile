#
# Master make to build a kernel suitable for the PCM-9373 board.
#
# CC=../m1/tools/i686-pc-linux-gnu/gcc-3.4.4/bin/gcc-3.4
#KERNEL_VERSION=2.6.21.5
#KERNEL_VERSION=2.6.23.14
#KERNEL_VERSION=2.6.24
KERNEL_VERSION=2.6.25.4
KERNEL_SUFFIX=jway
KERNEL=linux-$(KERNEL_VERSION)-$(KERNEL_SUFFIX)
IMAGE_PATH?=../images-$(KERNEL_VERSION)-$(KERNEL_SUFFIX)
KERNEL_TARGET=$(KERNEL)/arch/i386/boot/bzImage
IMAGE_TARGET=$(IMAGE_PATH)/lib/modules/$(KERNEL_VERSION)-$(KERNEL_SUFFIX)/modules.dep

.PHONY: install

all: $(KERNEL)/README $(KERNEL_TARGET) 
	@echo "---- Done with kernel build"

install: $(IMAGE_TARGET)

$(IMAGE_TARGET): $(IMAGE_PATH) $(KERNEL_TARGET)
	rm -rf $(IMAGE_PATH)
#	(cd $(KERNEL); make CC=../$(CC)  INSTALL_MOD_PATH=../$(IMAGE_PATH) modules_install)
	(cd $(KERNEL); make INSTALL_MOD_PATH=../$(IMAGE_PATH) modules_install)
#	/sbin/depmod -F$(KERNEL)/System.map -e -b $(IMAGE_PATH) $(KERNEL_VERSION)-$(KERNEL_SUFFIX)
	/sbin/depmod -F$(KERNEL)/System.map -e -b $(IMAGE_PATH) $(KERNEL_VERSION)

$(IMAGE_PATH):
	@echo "---- Directory $(IMAGE_PATH) does not exist. Creating it."
	mkdir -p $(IMAGE_PATH)

$(KERNEL_TARGET):
	@echo "---- Compiling the kernel. Please wait." 
#	(cd $(KERNEL); make CC=../$(CC) bzImage modules)
	(cd $(KERNEL); make bzImage modules)



$(KERNEL)/README: # If this file is not here, we don't have a kernel source.
	@echo "---- Unpacking the $(KERNEL_VERSION) kernel. This will take a while."
	@tar xjf ./linux-$(KERNEL_VERSION).tar.bz2
	@mv ./linux-$(KERNEL_VERSION) ./$(KERNEL)
	@echo "---- Applying magden patch to get VESA framebuf magden boot logo."
	@(cd $(KERNEL); patch -p1 < ../$(KERNEL).patch)
	@echo "Doing mrproper"
#	(cd $(KERNEL); make CC=../$(CC) mrproper)
	@(cd $(KERNEL); make  mrproper)
	@echo "---- Copying customized config to $(KERNEL)/.config"
	@cp $(KERNEL).config $(KERNEL)/.config
	@echo "---- Cleaning up and making dependencies"
#	(cd $(KERNEL); make CC=../$(CC) clean)
	@(cd $(KERNEL); make clean)


$(CC):  # Not used right now since we use native compiler
	@echo "No gcc 3.4.4 available."
	@echo "Please checkout m1/tools/i686-pc-linux-gnu/gcc-3.4.4 so that the dist directory that"
	@echo "this Makefile resides under is under the same m1 directory as tool. Like this"
	@echo "cd build_directory/m1 (distl directory is located here)."
	@echo "svn co svn+ssh://user@server/repository_path/m1/tools/i686-pc-linux-gnu/gcc-3.4.4 tools/i686-pc-linux-gnu/gcc-3.4.4"
	exit 255

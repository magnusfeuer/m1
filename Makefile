#
# All rights reserved. Reproduction, modification, use or disclosure
# to third parties without express authority is forbidden.
# Copyright Magden LLC, California, USA, 2004, 2005, 2006, 2007.
#

include Makefile.inc

#
# Default app to checkout and build. 
# Override with make APP=xxx. Choose something from app
#
#  Run the end result (in debug mode) as:
#
APP ?= jway/roll_demo


#
# Leave the following path definitions alone. 
# They just specify where in the local tree to checkout and build.
#
PLUGIN_PATH=./app/plugins
PLUGIN_ABS_PATH=$(BUILD_ROOT)/$(PLUGIN_PATH)

APP_PATH=./app
APP_ABS_PATH=$(BUILD_ROOT)/$(APP_PATH)

CORE_PATH=./core
CORE_ABS_PATH=$(BUILD_ROOT)/$(CORE_PATH)


#
# For now we will use native compilers.
#
CC=gcc
CXX=g++ 

#
# Target whose absence is used to trigger a checkout of:
# m1/core
# m1/tools/ddscomp (To compile bitmaps)
# $(COMPILER)
#
CORE_CLONE_TRIGGER=./core/.git/HEAD
APP_CLONE_TRIGGER=./app/.git/HEAD


# .PHONY: kernel check_error update core
.PHONY: o check_error update core

master_all: check_invocation core $(APP)

packfile: core plugins $(PAPP) 

db: core plugins $(DAPP) 

# all_kernel: master_all kernel

check_invocation:
ifeq ($(APP), nil)
ifeq ($(PAPP), nil)
	$(error Usage: $(MAKE) APP="target_platform/skin1 target_platform/skin2 ...")
endif
endif

core:  $(CORE_CLONE_TRIGGER)  
	@echo
	@echo "--- Building core with external libs and tools."
	@(cd $(CORE_PATH);$(MAKE))

plugins:  $(CORE_CLONE_TRIGGER)
	@echo
	@echo "--- Building plugins."
	@(cd $(PLUGIN_PATH);$(MAKE))

# kernel: 
# 	@echo
# 	@echo "--- Building kernel."
# 	(cd kernel; $(MAKE) all install)
# 	@echo
# 	@echo "--- Building busybox"
# 	(cd busybox; $(MAKE))
# 	@echo
# 	@echo "--- Building udev"
# 	(cd udev; $(MAKE))

update:
	@echo
	@echo "--- Updating core."
	(cd m1; git pull)

m1e_packfile:
	(cd $(CORE_PATH); make packfile)

os_packfile:
	$(SHELL) create_os_packfile.sh

packfile: $(PAPP)

m1e_db: 
	(cd $(CORE_PATH); make dpf)

db: $(DAPP)

$(APP):	$(APP_CLONE_TRIGGER)

$(PAPP): $(APP_CLONE_TRIGGER) $(patsubst %,$(APP_ABS_ROOT)/%, $(PAPP))
	@echo
	@echo "--- Building skin packfile $(@)"
	@-rm -rf ptmp
	@-mkdir -p $(BUILD_ROOT)/ptmp > /dev/null 2>&1
	(cd $(abspath $(APP_ABS_ROOT))/$(@)/; $(MAKE) TARGET_DIR=$(BUILD_ROOT)/ptmp)
	(cd $(abspath $(APP_ABS_ROOT))/$(@)/; $(MAKE) pfile TARGET_DIR=$(BUILD_ROOT)/ptmp)


$(DAPP): $(APP_CLONE_TRIGGER) $(patsubst %,$(APP_ABS_ROOT)/%, $(DAPP))
	@echo
	@echo "--- Building DB packfile $(@)"
	@-rm -rf ptmp
	@-mkdir -p $(BUILD_ROOT)/ptmp > /dev/null 2>&1
	(cd $(abspath $(APP_ABS_ROOT))/$(@)/; $(MAKE) TARGET_DIR=$(BUILD_ROOT)/ptmp)
	(cd $(abspath $(APP_ABS_ROOT))/$(@)/; $(MAKE) dfile TARGET_DIR=$(BUILD_ROOT)/ptmp)



$(CORE_CLONE_TRIGGER):
	@echo
	@echo "--- Checking out m1 core."
	git clone -b $(CORE_GIT_BRANCH) $(CORE_GIT_REPO) core

$(APP_CLONE_TRIGGER):
	@echo
	@echo "--- Checking out m1 app."
	git clone -b $(APP_GIT_BRANCH) $(APP_GIT_REPO) app

include Makefile.rules

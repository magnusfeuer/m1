#
# All rights reserved. Reproduction, modification, use or disclosure
# to third parties without express authority is forbidden.
# Copyright Magden LLC, California, USA, 2004, 2005, 2006, 2007.
#

#
# Master include file used by app under m1/app/[platform]/[app]/[version]
# Also used by the Makefile in this root directory.
#

SHELL=/bin/bash
export SHELL

BUILD_ROOT ?= $(PWD)
export BUILD_ROOT

#
# git(hub) server
#
GIT_SERVER ?= git@github.com:magnusfeuer
export GIT_SERVER

#
# Versions for different components (app etc)
#
include $(BUILD_ROOT)/Version.inc

export CORE_VERSION
export APP_VERSION

M1_TARGET_ARCH ?= jway
export M1_TARGET_ARCH

#
# Target framebuffer descriptor
# rgb, bgr, argb, bgra etc. 
# Will be passed on to ~/m1/app/src/[source_packet]/[version]/build_dds.sh
#
TARGET_FRAMEBUFFER ?= bgra
export TARGET_FRAMEBUFFER

#
# M1 core prefix
#
M1_CORE_ROOT=core
export M1_CORE_ROOT
#
# M1 core absolute path
#
M1_CORE_ABS_ROOT=$(BUILD_ROOT)/$(M1_CORE_ROOT)
export M1_CORE_ABS_ROOT
#
# M1 core include
#
#M1_CORE_INCLUDE=$(M1_CORE_ABS_ROOT)/$(CORE_VERSION)/include
#export M1_CORE_INCLUDE

#
# Epic include
#
#EPIC_INCLUDE=$(M1_CORE_ABS_ROOT)/$(CORE_VERSION)/epic/include
#export EPIC_INCLUDE

#
# Extern tool include
#
#EXTERN_INCLUDE=$(M1_CORE_ABS_ROOT)/$(CORE_VERSION)/extern/include
#export EXTERN_INCLUDE

#
# Application root, containing all Mpl apps.
#
APP_ROOT=app
export APP_ROOT

#
# Absolute path to app root dir in file system
#
APP_ABS_ROOT=$(BUILD_ROOT)/$(APP_ROOT)
export APP_ABS_ROOT

#
# plugin (C++ drivers to integrate various hardware) root 
#
PLUGIN_ROOT=$(APP_ROOT)/plugins
export PLUGIN_ROOT

#
# Absolute path to plugin root in file system
#
PLUGIN_ABS_ROOT=$(BUILD_ROOT)/$(PLUGIN_ROOT)
export PLUGIN_ABS_ROOT

#
# Where do we want the result_m1, result_dds and result_launch directories
# to be created. This local value will be overridden if this Makefile
# is invoked by another make
#
TARGET_DIR?=$(PWD)/out
export TARGET_DIR


#
# Font root
#
FONT_ROOT=$(APP_ROOT)/src/fonts
export FONT_ROOT

#
# Absolute path to app root dir in file system
#
FONT_ABS_ROOT=$(APP_ABS_ROOT)/src/fonts
export FONT_ABS_ROOT

#
# BINDIR
#
BINDIR?=$(BUILD_ROOT)/bin
export BINDIR

#
# PACKDIR
# Packfiles are the distribution format, like deb or rpm files,
# used by m1. M1 boxes can be upgraded using USB sticks with
# packfiles on them.
#
PACKDIR?=$(BUILD_ROOT)/packfiles
export PACKDIR


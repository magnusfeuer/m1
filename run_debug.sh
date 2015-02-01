#!/bin/sh
#
# Script to start m1 (debug mode)
#

M1_PATH=$PWD

export M1_DATABASE=/tmp/m1.db
export M1_LIB_PATH=$M1_PATH/out/plugin
export M1_BIN_PATH=$M1_PATH/bin
export M1_FONT_PATH=$PWD/out/dds
export M1_CONSULT_DEVICE=/dev/tty.usbserial
export M1_DEBUG_CLS="-cApexDevice" # "-cConsultDevice -cTimeout"
export M1_APEX_DEVICE="sova_ej"
(cd out; $M1_PATH/bin/m1e.debug -DW $M1_DEBUG_CLS `cat launch/[0-9][0-9]-*`)

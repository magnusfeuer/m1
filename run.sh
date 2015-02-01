#!/bin/sh
#
# Script to start m1 (release mode)
#

M1_PATH=$PWD
#M1_PATH=$M1_WORK

export M1_DATABASE=/tmp/m1.db
export M1_LIB_PATH=$M1_PATH/out/plugin
export M1_FONT_PATH=$PWD/out/dds

# ../m1/core/trunk/bin/m1e
(cd out; $M1_PATH/bin/m1e `cat launch/[0-9][0-9]-*`)

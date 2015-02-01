#!/bin/sh
#
# Script to start m1
#
M1_PATH=$PWD
#M1_PATH=$M1_WORK

export M1_DATABASE=/tmp/m1.db
export M1_LIB_PATH=$M1_PATH/out/plugin
export M1_FONT_PATH=$PWD/out/dds

(cd out; valgrind --log-file="valgrind_log" $M1_PATH/bin/m1e.debug  `cat launch/[0-9][0-9]-*`)
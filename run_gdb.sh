#!/bin/sh
#
# Script to start m1
#
EMACS=/usr/bin/emacs

M1_PATH=$PWD
#M1_PATH=$M1_WORK

M1_LIB_PATH=$M1_PATH/out/plugin
M1_BIN=$M1_PATH/bin
M1_SRC=$M1_PATH/m1/core/trunk/src
M1_OUT=$PWD/out
M1_DEBUG_CLS="-cConsultDevice -cTimeout"

function command
{
    echo "(insert-string \"$1\") (comint-send-input)";
}

cmd0=`command "break main"`
cmd11=`command "set environment M1_DATABASE /tmp/m1.db"`
cmd12=`command "set environment M1_LIB_PATH $M1_LIB_PATH"`
cmd13=`command "set environment M1_FONT_PATH $PWD/out/dds"`
cmd14=`command "set environment M1_CONSULT_DEVICE /dev/tty.usbserial"`
cmd2=`command "directory $M1_SRC"`
cmd3=`command "cd $M1_OUT"`
opts=$M1_DEBUG_CLS

launch=`cat out/launch/[0-9][0-9]-*`
launch_list=`echo $launch`

cmd4=`command "run $opts $launch_list"`

echo "cmd11 = $cmd11";
echo "cmd12 = $cmd12";
echo "cmd13 = $cmd13";
echo "cmd14 = $cmd14";
echo "cmd2 = $cmd2";
echo "cmd3 = $cmd3";


$EMACS --eval "(progn (gdb \"gdb $M1_BIN/m1e.debug\") $cmd0 $cmd11 $cmd12 $cmd13$cmd14 $cmd2 $cmd3 $cmd4)"

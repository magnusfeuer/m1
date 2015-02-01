#!/bin/bash
#  Update script broken out from make.
#
echo "Update: $1 SKINS=$2 MAKE=$3"
if [  -d $1 ]; then
    cd $1;  
    svn update; 
    $3 update  
else 
    $3 SKINS=$2
fi


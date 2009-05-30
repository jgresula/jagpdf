#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


SCRIPT_LOCATION=`dirname $0`;
cd $SCRIPT_LOCATION/jagbase.build
INI=bitten_cmake_smoke.ini
LOGFILE="../../logs/$INI.`date +%Y-%m`.log"
./bitten_slave.sh --log=$LOGFILE $INI


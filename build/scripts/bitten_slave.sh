#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# build slave initial work
# 
# $ svn checkout svn+jagbuildssh://SVNUser@jarda-home/trunk/jagbase jagbase
# $ cd jagbase/build/scripts
# $ ./create_build_dir.sh --config=debug ../../../jagbase.debug

#set -x

function usage()
{
    echo "
Usage:
$0 [options] <build-cfg.ini>

options
 --log=LOGFILE
"
}



function lock()
{
    if [ -f $LOCKFILE ]; then
        locked_pid=`cat $LOCKFILE`
        if [ "0" != "`ps -a | grep $locked_pid | wc -l`" ]; then
            echo 'Locked, exiting'
            exit 1
        else
            echo 'Locked, but the locking process is not alive. Taking over the lock.'
            echo $$ > $LOCKFILE
        fi
    else
        echo $$ > $LOCKFILE
    fi
}

#
# variables
#
BITTEN_SLAVE=bitten-slave
BITTEN_MASTER=http://192.168.1.100/trac/builds

#
# options parsing
#
LOGGING_REX="--log=(.*)"
BITTEN_SLAVE_REX="--slave=(.*)"

set -- $*;
while true
do
    if [[ $1 =~ $LOGGING_REX ]]; then
        LOGFILE=${BASH_REMATCH[1]}; shift;
    elif [[ $1 =~ $BITTEN_SLAVE_REX ]]; then
        BITTEN_SLAVE=${BASH_REMATCH[1]}; shift;
    elif [[ $1 =~ --.* ]]; then 
        echo "unknown option: $1"
        usage;
        exit 1
    else
        break;
    fi
done

BUILD_CONFIG=$1 #bitten_cmake_smoke.ini
if [ -z "$BUILD_CONFIG" ]; then
    echo "Need bitten build .ini file."
    usage;
    exit 1
fi

LOCKFILE=/var/lock/$BUILD_CONFIG
LOGFILE=${LOGFILE:=$BUILD_CONFIG.`date +%Y`.log}
lock

"$BITTEN_SLAVE" --verbose\
 --log=$LOGFILE\
 --keep-files\
 --config=$BUILD_CONFIG\
 --build-dir=\
 --work-dir=.\
 --single\
 --no-loop\
 $BITTEN_MASTER

rm -rf $LOCKFILE

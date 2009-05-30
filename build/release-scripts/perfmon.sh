#!/bin/bash
#
# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


POLL_INTERVAL=5

function usage()
{
    cat <<EOF
usage: perfmon.sh [options] pid

options:
 --interval sec      time between polls (default ${POLL_INTERVAL})


For specified processed outputs the following to stdout:

VirtualMemory NumThreads

EOF
}


while true
do
    if [ "$1" == "--interval" ]; then
        shift; POLL_INTERVAL=$1; shift
    else
        break;
    fi
done

PID=$1

if [ -z "$PID" ]; then
    usage
    exit 2
fi


if [ ! -d /proc/$PID ]; then
    echo "process pid=$PID does not exist"
    exit 1
fi

while true
do
    VM=`cat /proc/$PID/status | grep VmSize | grep -oE "[0-9]+"`
    THREADS=`cat /proc/$PID/status | grep Threads: | grep -oE "[0-9]+"`
    echo $VM $THREADS
    sleep $POLL_INTERVAL
done
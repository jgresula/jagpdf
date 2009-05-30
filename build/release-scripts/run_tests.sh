#!/bin/bash
#
# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

set -e
#set -x


# archives.tar.bz2
# archives/
# apitest/ ... suite
# build/   ... build dir

function normpath()
{
    if [[ "`uname`" =~ "CYGWIN" ]]; then
        cygpath -w $1
    else
        echo $1
    fi
}

SCRIPT_DIR=`dirname $0`
SCRIPT_DIR=`cd $SCRIPT_DIR && pwd`
SCRIPT_DIR=`normpath $SCRIPT_DIR`
HOSTNAME=`hostname`

function usage()
{
    cat <<EOF
usage: run_tests.sh 

options:
 <none yet>

EOF
}


function do_test()
{
    DIST_DIR=`cd $1 && pwd`
    DIST_DIR=`normpath $DIST_DIR`
    shift
    SRC_DIR=`cd apitest && pwd`
    SRC_DIR=`normpath $SRC_DIR`
    rm -rf ./build/*
    mkdir -p ./build
    cd ./build
    cmake -G "Unix Makefiles" \
        -DJAG_INSTALL_PREFIX=$DIST_DIR \
        $@ \
        "$SRC_DIR"
    make apitests
    cd -
}

function unpack_package()
{
    if [ -f $1.tar.bz2 ]; then
        tar -xjf $1.tar.bz2
    elif [ -f $1.zip ]; then
        unzip -q -u $1.zip
    else
        echo "unknown package stem $1"
        exit 1
    fi
}

function prepare_package()
{
    STEM="$1"
    if [ ! -d "$STEM" ]; then
        unpack_package "$STEM"
    fi
}

if [ "`uname`" == "Linux" ]; then
    CFG_PLATFORM=linux.x86
else
    CFG_PLATFORM=win32.x86
fi

CFG_VERSION=1.3.0

echo Unpacking ...

prepare_package jagpdf-$CFG_VERSION.$CFG_PLATFORM.java
prepare_package jagpdf-$CFG_VERSION.$CFG_PLATFORM.c_cpp
prepare_package jagpdf-$CFG_VERSION.$CFG_PLATFORM.py25
prepare_package jagpdf-$CFG_VERSION.$CFG_PLATFORM.py26
prepare_package jagpdf-$CFG_VERSION.$CFG_PLATFORM.py24

if [ ! -d jagpdf-$CFG_VERSION.all ]; then
    # unpack source packages and check that src + test = all
    prepare_package jagpdf-$CFG_VERSION.src.all
    mv jagpdf-$CFG_VERSION{,.all}
    prepare_package jagpdf-$CFG_VERSION.src
    prepare_package jagpdf-$CFG_VERSION.src.apitest
    diff --recursive jagpdf-$CFG_VERSION{,.all}
fi

# create/update a local copy of apitests
mkdir -p ./apitest
cp --update -R jagpdf-$CFG_VERSION/code/test/apitest/* ./apitest/

#set -x

source $SCRIPT_DIR/test_$HOSTNAME.cfg




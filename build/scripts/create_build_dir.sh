#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


#set -x

PWD=$(pwd)
SCRIPT_LOCATION=`dirname $0`;
SCRIPT_LOCATION=`cd $SCRIPT_LOCATION && pwd`;
SRC_DIR=`cd $SCRIPT_LOCATION/../../../jagbase && pwd`;

function usage()
{
    echo "
Usage:
$0 [options] build-dir

Options:
 --config=Release,Debug
 --python=M.m
 -DVARIABLE=VALUE
"
}

if [ ! -f ~/.jagbase.cfg ]; then
    echo "need ~/.jagbase.cfg"
    exit 1
fi

BUILD_CONFIG=Debug
PYTHON_VERSION=
TOOLSET=
CMAKE_VARIABLES=

#
# options parsing
# 
PYTHON_REX="--python=(.*)"
set -- $*;
while true
do
    if [[ $1 =~ --config=[Dd]ebug ]]; then
        BUILD_CONFIG=Debug; shift;
    elif [[ $1 =~ --config=[Re]elease ]]; then
        BUILD_CONFIG=Release; shift;
    elif [[ $1 =~ $PYTHON_REX ]]; then
        PYTHON_VERSION=python-${BASH_REMATCH[1]}; shift;
    elif [[ $1 =~ -D.* ]]; then
        CMAKE_VARIABLES="$CMAKE_VARIABLES $1"; shift
    elif [[ $1 =~ --.* ]]; then 
        echo "unknown option: $1"
        usage;
        exit 1
    else
        break;
    fi
done

BUILD_DIR=$1
if [ -z "$BUILD_DIR" ]; then
    echo "Need build directory.";
    usage;
    exit 2;
fi
mkdir -p $BUILD_DIR

# load the site specific configuration file
source ~/.jagbase.cfg $PYTHON_VERSION $TOOLSET


# populate CACHE_ARGS with variables retrived from the configuration file
CACHE_ARGS=
function cache_arg()
{
    if [ -n "$2" ]; then
        # when $1 is quoted then cmake does not recognize it
        CACHE_ARGS="$CACHE_ARGS -D $1=$2 "
    fi
}

function capitalize()
{
    echo `echo "${1:0:1}" | tr [a-z] [A-Z]`${1:1}
}


cache_arg "PYTHON_DEBUG_LIBRARY:FILEPATH" "PYTHON_DEBUG_LIBRARY-NOTFOUND"
cache_arg "PYTHON_EXECUTABLE:FILEPATH" "$JAG_PYTHON_EXECUTABLE"
cache_arg "PYTHON_INCLUDE_PATH:PATH" "$JAG_PYTHON_INCLUDE_PATH"
cache_arg "PYTHON_LIBRARY:FILEPATH" "$JAG_PYTHON_LIBRARY"
cache_arg "CMAKE_CXX_COMPILER:FILEPATH" "$JAG_CXX_COMPILER"
cache_arg "CMAKE_C_COMPILER:FILEPATH" "$JAG_C_COMPILER"
cache_arg "BOOST_ROOT:PATH" "$JAG_BOOST_ROOT"
cache_arg "JAVA_HOME:FILEPATH" "$JAG_JAVA_HOME"

if [[ `uname` =~ CYGWIN.* ]]; then
    SRC_DIR=`cygpath --windows $SRC_DIR`
    # for some reason, cmake needs capitalized drive letter, otherwise it fails
    # like this: <http://www.mail-archive.com/cmake@cmake.org/msg07695.html>
    SRC_DIR=`capitalize $SRC_DIR`
    PYTHON_INSTALL_DIR=`cygpath --windows $BUILD_DIR/distribution/lib`
    CMAKE_INSTALL_PREFIX=`cygpath --windows $BUILD_DIR/distribution`
else
    PYTHON_INSTALL_DIR="$BUILD_DIR/distribution/lib"
    CMAKE_INSTALL_PREFIX="$BUILD_DIR/distribution"
fi


cd "$BUILD_DIR"
cmake -G "Unix Makefiles" \
    $CACHE_ARGS \
    -DALL_IN_ONE=ON \
    -D "CMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX" \
    -D "PYTHON_INSTALL_DIR=$PYTHON_INSTALL_DIR" \
    -D "CMAKE_BUILD_TYPE:STRING=$BUILD_CONFIG" \
    $CMAKE_VARIABLES \
    "$SRC_DIR"








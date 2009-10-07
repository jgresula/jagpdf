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

# source the sw configuration for this machine
# 
source ~/.jagbase.cfg

function normpath()
{
    if [[ "`uname`" =~ "CYGWIN" ]]; then
        cygpath -w "$1"
    else
        echo $1
    fi
}

SCRIPT_DIR="`dirname $0`"
SCRIPT_DIR="`cd $SCRIPT_DIR && pwd`"
SCRIPT_DIR="`normpath \"$SCRIPT_DIR\"`"
HOSTNAME=`hostname`

function usage()
{
    cat <<EOF
usage: run_tests.sh [remote-machines]

options:
 <none yet>

EOF
}

if [ -n "$1" ]; then
    #
    # run tests on remote machines
    #
    MACHINE=$1

    echo "$0"
    if [ "$0" -nt "jagpdf-tests.tgz" ]; then
        echo "jagpdf-tests.tgz is out of date"
        echo "run \"make_release pack-test\""
        exit 1
    fi

    read -s -p "password: " PWD
    echo

    for MACHINE in $@; do
        remote_system=`ssh $MACHINE uname -a`
        echo -------------------------------------------------------------------
        echo $remote_system
        echo -------------------------------------------------------------------
        if [[ "$remote_system" =~ ^CYGWIN ]]; then
            # google for 'C1902 cygwin'
            SSH_ARGS="-o PubkeyAuthentication=no"
            # work around the problem with whithespace in directory names
            RDIR=/cygdrive/c/home/jarda/tmp/release_test
            # sshpass reads pwd from stdin
            SSHPASS=sshpass
        else
            SSH_ARGS=""
            RDIR=tmp/release_test
            SSHPASS=
        fi
        
        ssh $MACHINE "rm -rf $RDIR ; mkdir -p $RDIR"
        files=`find release.out/binaries/ -maxdepth 1 -type f`
        scp jagpdf-tests.tgz $files $MACHINE:$RDIR
        yes $PWD | $SSHPASS ssh -t $SSH_ARGS $MACHINE "cd $RDIR && tar -xzf jagpdf-tests.tgz && ./run_tests.sh"
    done
    exit 0
fi


#
# checks that no jagpdf .deb package is currently installed
#
function no_installed_deb_allowed()
{
    set +e
    npackages=`aptitude search jagpdf | grep -e '^i' | wc -l`
    set -e
    if [ "$npackages" != "0" ]; then
        echo 'Found these jagpdf packages:'
        aptitude search jagpdf | grep -e '^i'
        echo 'Remove them before using this script.'
        exit 1
    fi
}

#
# creates a symbolic link $2/lib -> $1
# 
function create_lib_link()
{
    TARGET=$1
    LINK=$2

    if [ ! -h $LINK/lib ]; then
        mkdir $LINK
        ln -s $TARGET $LINK/lib
    fi
}

#
# test jagpdf for c/c++ installed from .deb
# 
function deb_test_cpp()
{
    install_deb jagpdf-$CFG_VERSION.$CFG_PLATFORM.c_cpp.deb
    do_test /usr
    remove_deb libjagpdf
}

#
# test jagpdf for python installed from .deb
# it takes python M.m version (e.g. 2.5)
# 
function deb_test_python()
{
    M_m=$1
    Mm=${M_m//./}

    install_deb jagpdf-$CFG_VERSION.$CFG_PLATFORM.py$Mm.deb

    if [ ! -h jagpdf-python$M_m/lib ]; then
        mkdir jagpdf-python$M_m
        ln -s /var/lib/python-support/python$M_m jagpdf-python$M_m/lib
    fi

    create_lib_link /var/lib/python-support/python$M_m jagpdf-python$M_m

    do_test \
        '<system>' \
        -DPYTHON_EXECUTABLE=/usr/bin/python$M_m \
        -DJAGPDF_PY_SYSTEM=ON

    remove_deb python$M_m-jagpdf
}

#
# tests jagpdf for java installed from .deb
# it takes a list of paths to java home
# 
function deb_test_java()
{
    JAVA_HOME=$1

    install_deb jagpdf-$CFG_VERSION.$CFG_PLATFORM.java.deb
    create_lib_link /usr/share/java jagpdf-java
    
    for java_dir in $@; do
        ( export LD_LIBRARY_PATH=/usr/lib/jni ;
            do_test jagpdf-java -DCMAKE_PREFIX_PATH=$java_dir )
    done
    
    remove_deb java-jagpdf
}

# install from sources and test result, the arguments (if any) are passed
# cmake
function install_from_source()
{
    JAGPDF_SOURCE_DIR=jagpdf-$CFG_VERSION
    JAGPDF_BUILD_DIR=`cd $JAGPDF_SOURCE_DIR && pwd`.build
    JAGPDF_DIST_DIR=$JAGPDF_BUILD_DIR/distribution
    PYENV=pyenv

    rm -rf $JAGPDF_BUILD_DIR
    mkdir -p $JAGPDF_BUILD_DIR
    cd  $JAGPDF_BUILD_DIR

    rm -rf $PYENV
    $VIRTUALENV $PYENV
    source $PYENV/bin/activate

    env BOOST_ROOT=$JAG_BOOST_ROOT cmake \
        -DJAVA_HOME=$JAG_JAVA_HOME \
        -DCMAKE_INSTALL_PREFIX=$JAGPDF_DIST_DIR \
        -DPYTHON_EXECUTABLE=`which python` \
        $* \
        ../$JAGPDF_SOURCE_DIR

    make $MAKE_ARGS
    make unit-tests
    make apitests
    make install
    cd -

    do_test $JAGPDF_DIST_DIR -DCMAKE_PREFIX_PATH=$JAG_JAVA_HOME

    # deactivate virtualenv
    deactivate
}

# tests distutils for the all-in-one versions
function test_setup_py()
{
    python_interpreter=$1
    Mm=$2
    ( # execute in a new shell due to virtenv
        ENV_DIR=env$Mm
        $python_interpreter $VIRTUALENV $ENV_DIR
        source $ENV_DIR/bin/activate
        PYEXEC=`which python`
        cd jagpdf-$CFG_VERSION.$CFG_PLATFORM.py$Mm
        $PYEXEC setup.py install
        cd -
            
        do_test \
            "<system>" \
            -DJAGPDF_PY_SYSTEM=ON \
            -DPYTHON_EXECUTABLE=$PYEXEC
            
            
        deactivate
        rm -rf $ENV_DIR
        )
}



#
# configures and runs apitests
# 
function do_test()
{
    if [ "$1" != "<system>" ]; then 
        DIST_DIR=`cd $1 && pwd`
        DIST_DIR="`normpath \"$DIST_DIR\"`"
        DIST_DIR_ARG="-DJAG_INSTALL_PREFIX=$DIST_DIR"
    else
        DIST_DIR_ARG="-DJAG_INSTALL_PREFIX=/this/dir/does/not/exist"
    fi
    shift

    SRC_DIR=`cd apitest && pwd`
    SRC_DIR="`normpath \"$SRC_DIR\"`"
    rm -rf ./build/*
    mkdir -p ./build
    cd ./build
    cmake -G "Unix Makefiles" "$DIST_DIR_ARG" \
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
    if [[ "`uname -m`" =~ 'x86_64' ]]; then
        CFG_PLATFORM=linux.amd64
    else
        CFG_PLATFORM=linux.x86
    fi
else
    CFG_PLATFORM=win32.x86
fi

function install_deb()
{
    sudo dpkg -i $1
}

function remove_deb()
{
    sudo dpkg -r $1
}

CFG_VERSION=1.5.0


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


source "$SCRIPT_DIR/test_$HOSTNAME.cfg"





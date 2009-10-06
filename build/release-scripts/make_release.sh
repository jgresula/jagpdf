#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#



set -e # exit immediatelly on error
#set -x

# directory structure created by the *init* command
# 
#   jagbase
#   jagbase.debug
#   jagbase.release
#   jagbase.doc
#   jagpdf-www
#   jagpdf-www.build
#   binaries
#   public_html
# 
# command *code*
#  - builds all binary targets and places them to binaries/
# 
# command *doc*
#  - builds complete documentation
#    archive -> binaries/
#    build -> jagbase.debug/distribution/doc
#  - actions
#    $ cd jagbase.doc
#    $ make doc-from-sources
#    $ make dist-doc
#    $ <tidy>
#    $ make PACKAGE_doc
#    $ <copy archive to binaries/
# 
# command *web*
#  - builds web to jagpdf-www.build/html
#  - <tidy>
# 
# command *check*
#  - the same as now
# 
# deployment
#  $ rm -rf public_html/*
#  $ <copy content of relevant directories> to public_html
#  $ <create .sha .md5 for relevant files>
#  $ <copy either to a local server or publish it> [--remote]
# 
#  command *deploy-web*
#   $ cp -r jagpdf-www.build/html/* public_html
# 
#  command *deploy*
#   $ cp <rest> public_html
#   $ create .sha .md5
# 
# important folders:
#  binaries/  - result of make_release.sh code doc (.bzip2, .zip)
#             - results from other platforms should be copied here
#             - .sha, .md5 are created by the deploy command
#                 
#  jagbase.debug/distribution/doc/
#  jagpdf-www.build/html/
# 
# TBD: unpack archives and test them

#
# constants
# 
SVN_BASE_URL="svn://192.168.1.100"
GIT_WWW_REPO=~/repos/jagpdf-www.git
ROOT_DIR=.

#
# configurable variables
# 
# commands
CMD_INIT=
CMD_BUILD_CODE=
CMD_CHECK=
CMD_BUILD_DOC=
CMD_BUILD_WEB=
CMD_DEPLOY_WEB=
CMD_DEPLOY_LIB=
CMD_REMOTE_DEPLOY=
CMD_TEST_PKG=
CMD_PACK_TEST=
# options
SVN_BRANCH_URL="trunk/jagbase"
SVN_REVISION=HEAD
DO_HTML_CHECK=1
START_LOCAL_SERVER=
CHECK_BASE_CSS=1
MEMCHECK_SUFFIX="-memcheck"
CHECK_CODE_TXT=1
HOSTNAME=`hostname`

#
# parse command line
#
function usage()
{
    cat <<EOF
usage: make_release.sh [options] commands

commands:
 init       checkouts code and initializes the directory structure
 code       builds source code and creates archives with binaries
 doc        builds documentation and creates an archive
 web        builds web
 test-pkg   tests distributed packages
 pack-test  packs test scripts
 deploy-web deploys web files
 deploy-lib deploys library files
 deploy-all deploys library and web files

common options
 --no-html-check      do not check html files
 --local-server       start local http server after deploying
 --no-base-css-check  do not check base.css
 --no-stop-on-error   continue even if an error occurs

options for command init
 --branch BRANCH  source code branch [trunk/jagbase]
 --revision REV   source code revision [HEAD]

options for command code
 --no-txt-check   do not spellcheck text files
 --no-memcheck    do not run valgrind


EOF
}

if [ -z "$1" ]; then
    usage
    exit 2
fi

while true
do
    if [ "$1" == "init" ]; then
        CMD_INIT=1; shift
    elif [ "$1" == "code" ]; then
        CMD_BUILD_CODE=1; shift
    elif [ "$1" == "doc" ]; then
        CMD_BUILD_DOC=1; shift
    elif [ "$1" == "web" ]; then
        CMD_BUILD_WEB=1; shift
    elif [ "$1" == "deploy-web" ]; then
        CMD_DEPLOY_WEB=1; shift
    elif [ "$1" == "deploy-lib" ]; then
        CMD_DEPLOY_LIB=1; shift
    elif [ "$1" == "deploy-all" ]; then
        CMD_DEPLOY_LIB=1; CMD_DEPLOY_WEB=1; shift
    elif [ "$1" == "test-pkg" ]; then
        CMD_TEST_PKG=1; shift
    elif [ "$1" == "pack-test" ]; then
        CMD_PACK_TEST=1; shift
    elif [ "$1" == "--revision" ]; then
        shift; SVN_REVISION=$1; shift
    elif [ "$1" == "--branch" ]; then
        shift; SVN_BRANCH_URL=$1; shift
    elif [ "$1" == "--no-html-check" ]; then
        DO_HTML_CHECK= ; shift;
    elif [ "$1" == "--local-server" ]; then
        START_LOCAL_SERVER=1 ; shift;
    elif [ "$1" == "--no-base-css-check" ]; then
        CHECK_BASE_CSS=; shift;
    elif [ "$1" == "--no-memcheck" ]; then
        MEMCHECK_SUFFIX=; shift;
    elif [ "$1" == "--no-txt-check" ]; then
        CHECK_CODE_TXT=; shift;
    elif [ "$1" == "--no-stop-on-error" ]; then
        set +e; shift
    else
        break;
    fi
done


SVN_JAGBASE_URL="$SVN_BASE_URL/$SVN_BRANCH_URL"
SVN_REVISION_FLAG="--revision $SVN_REVISION"

TOP=`cd $ROOT_DIR && pwd`
SRC_DIR="$TOP/jagbase"
BUILD_DIR="$TOP/jagbase.build"
BUILD_DIR_RELEASE="$BUILD_DIR.GA"
PACKAGES_DIR="$TOP/release.out/binaries"
PKG_TEST_DIR="$TOP/release.test"
PUBLIC_HTML_DIR="$TOP/release.out/public_html/www"
WEB_SOURCE_DIR="$TOP/jagpdf-www"
WEB_BUILD_DIR="$WEB_SOURCE_DIR.build"
DOC_BUILD_DIR="$BUILD_DIR.doc"
if [ "`uname -n`" == "mamut" ]; then
    # on platforms other than linux only building sources is allowed
    IS_RELEASE_PLATFORM=1
fi

source ~/.jagbase.cfg

# ------------------------------------------------------------
#           build dir configuration
# ------------------------------------------------------------

#
# configures debug/release build directories, required python version is passed
# as the first argument
#
function create_build_dir()
{
    rm -rf $BUILD_DIR_RELEASE
    cd $SRC_DIR/build/scripts
    ./create_build_dir.sh --config=Release --python=$1 $BUILD_DIR_RELEASE
    cd -
}


# configures the build directory for a .deb build
function prepare_deb_build()
{
    configure_python $1

    rm -rf $BUILD_DIR_RELEASE
    mkdir $BUILD_DIR_RELEASE
    cd $BUILD_DIR_RELEASE
    cmake \
        -DBOOST_ROOT=$JAG_BOOST_ROOT \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=distribution \
        -DPYTHON_INSTALL_DIR=distribution/lib \
        -DJAVA_HOME=$JAG_JAVA_HOME \
        -DPYTHON_EXECUTABLE:FILEPATH=$JAG_PYTHON_EXECUTABLE \
        -DPYTHON_INCLUDE_PATH=$JAG_PYTHON_INCLUDE_PATH \
        -DPYTHON_LIBRARY:FILEPATH=$JAG_PYTHON_LIBRARY \
        ../jagbase
    cd -
}

# ------------------------------------------------------------
#                    C/C++ build
#                    
function build_c_base()
{
    cd $BUILD_DIR_RELEASE
    make rebuild_cache
    make $MAKE_ARGS dist-c
    make unit-tests
    make apitests-cpp$MEMCHECK_SUFFIX apitests-c$MEMCHECK_SUFFIX
    cd -
}

function build_c()
{
    build_c_base $@
    cd $BUILD_DIR_RELEASE
    make check-jagpdf-binaries
    make PACKAGE_jagpdf
    cd -
}

# ------------------------------------------------------------
#                    Python build
#                    
function build_python_base()
{
    cd $BUILD_DIR_RELEASE
    make rebuild_cache
    make $MAKE_ARGS dist-py
    make apitests-py$MEMCHECK_SUFFIX
    cd -
}

function build_python()
{
    build_python_base $@
    cd $BUILD_DIR_RELEASE
    make -s check-pyjagpdf-binaries
    make -s PACKAGE_pyjagpdf
    cd -
}

# ------------------------------------------------------------
#                    Java build

function build_java_base()
{
    cd $BUILD_DIR_RELEASE
    make $MAKE_ARGS dist-java
    make apitests-java
    cd -
}

function build_java()
{
    build_java_base $@
    cd $BUILD_DIR_RELEASE
    make -s check-jagpdf-java-binaries
    make -s PACKAGE_jagpdf_java
    cd -
}



HAS_CYGPATH=`which cygpath` || echo
function svn_file_path()
{
    if [ -z "$HAS_CYGPATH" ]; then
        echo $1
    else
        cygpath --windows "$1"
    fi
}

function copy_packages()
{
    find "$1" \( -name '*.deb' -o -name '*.zip' -o -name '*.bz2' \) ! -regex '.*CPack.*' -exec cp {} $PACKAGES_DIR \;
}

function strip-binaries()
{
    find "$1" -type f -name '*.so' -exec strip {} \;
}


# ---------------------------------------------------------------------------
#                                 main
#   

#
# initialization - CMD_INIT
#
# Checkout sources and configure for documentation. Configuring for
# release/debug builds is done per request.
#
# In linux checkout web and configure it.
# 
if [ -n "$CMD_INIT" ]; then
    svn checkout $SVN_REVISION_FLAG $SVN_JAGBASE_URL `svn_file_path "$SRC_DIR"`
    mkdir -p $PACKAGES_DIR
    if [ -n "$IS_RELEASE_PLATFORM" ]; then
        # configure for documentation
        rm -rf $DOC_BUILD_DIR
        cd $SRC_DIR/build/scripts
        ./create_build_dir.sh --config=Release -DDOCUMENTATION_ONLY=ON `svn_file_path "$DOC_BUILD_DIR"`
        cd -
        # web sources and configuration
        git clone $GIT_WWW_REPO $WEB_SOURCE_DIR
        rm -rf $WEB_BUILD_DIR
        mkdir $WEB_BUILD_DIR
        cd $WEB_BUILD_DIR
        cmake $WEB_SOURCE_DIR
        cd -
        # create other directories
        mkdir -p $PUBLIC_HTML_DIR
    fi
fi

#
#
#
if [ -n "$CMD_PACK_TEST" ]; then
    cd $TOP/jagbase/build/release-scripts
    tar -cvzf $TOP/jagpdf-tests.tgz run_tests.sh test_*.cfg test_*.bat
    cd -
fi

#
#
#
if [ -n "$CMD_BUILD_CODE" ]; then
    if [ -n "$IS_RELEASE_PLATFORM" ] && [ -n "$CHECK_CODE_TXT" ]; then
        # spellcheck *.txt files in the root directory
        $SRC_DIR/build/release-scripts/spellcheck.sh $SRC_DIR/*.txt
    fi

    source "$TOP/jagbase/build/release-scripts/build.$HOSTNAME"
    do_build
fi

if [ -n "$CMD_TEST_PKG" ]; then
    mkdir -p $PKG_TEST_DIR
    cp $PACKAGES_DIR/* $PKG_TEST_DIR
    cd $PKG_TEST_DIR
    $SRC_DIR/build/release-scripts/run_tests.sh
    cd -
fi

# stop here for platforms other than linux
if [ -z "$IS_RELEASE_PLATFORM" ]; then
    exit 0
fi

#
#
#
if [ -n "$CMD_BUILD_DOC" ]; then
    cd $DOC_BUILD_DIR
    make -s doc-from-sources
    make -s dist-doc
    if [ -n "$DO_HTML_CHECK" ]; then
        $SRC_DIR/build/release-scripts/postproc_doc.sh distribution/doc
    fi
    make -s pack-doc
    copy_packages $DOC_BUILD_DIR
    cd -
fi


#
#
#
if [ -n "$CMD_BUILD_WEB" ]; then
    cd $WEB_BUILD_DIR
    make -s dist-html
    if [ -n "$DO_HTML_CHECK" ]; then
        $SRC_DIR/build/release-scripts/spellcheck.sh $SRC_DIR/*.txt
        $SRC_DIR/build/release-scripts/postproc_doc.sh html
    fi
    cd -
fi

rm -rf $PUBLIC_HTML_DIR/*


# ensure that website base.css is synchronized with docs base.css
if [ -n "$CMD_DEPLOY_LIB" ] && \
    [ -n "$CMD_DEPLOY_WEB" ] && \
    [ -n "$CHECK_BASE_CSS" ]; then
    
    WEB_CSS=./jagpdf-www/base.css
    DOC_CSS=./jagbase/doc/quickbook/base.css
    if [ $WEB_CSS -nt $DOC_CSS ] && [ "`diff $WEB_CSS $DOC_CSS | wc -l`" != "0" ]; then
        
        echo "WARNING: Website base.css is newer than docs doc.css."
        read -p "  Is it ok? (yes/no|--no-base-css-check) "
        [ "$REPLY" != 'yes' ] && echo "Exiting." ; exit 1
    fi
fi


#
#
#
if [ -n "$CMD_DEPLOY_WEB" ]; then
    cp -r $WEB_BUILD_DIR/html/* $WEB_BUILD_DIR/html/.??* $PUBLIC_HTML_DIR
fi

function copy_to_downloads()
{
    if [ -z "$VERSION" ]; then
        echo "version not set"
        exit 1
    fi
    DEST=$PUBLIC_HTML_DIR/downloads/releases/$VERSION/$1
    mkdir -p $DEST
    shift
    pushd $PACKAGES_DIR/  > /dev/null
    cp $@ $DEST
    cd $DEST
    find . \( -name '*.bz2' -o -name '*.zip' -o -name '*.deb' \) -print \
        | while read f; do md5sum "$f" > "$f.md5"; done
    popd  > /dev/null
}

if [ -n "$CMD_DEPLOY_LIB" ]; then
    # copy documentation
    cp -r $DOC_BUILD_DIR/distribution/doc/* $PUBLIC_HTML_DIR
    
    fname=`ls -1 $PACKAGES_DIR/*.deb | head -n 1`
    if [[ "$fname" =~ jagpdf-([0-9]+\.[0-9]+\.[0-9]+)\. ]]; then
        VERSION="${BASH_REMATCH[1]}"
    else
        echo "can't detect version from file nam"
    fi

    # copy archives, create sums
    rm -rf $PUBLIC_HTML_DIR/downloads
    copy_to_downloads deb *.linux.*.deb
    copy_to_downloads linux *.linux.*.bz2
    copy_to_downloads windows *.win*.zip
    copy_to_downloads src *.src.* *.doc.*
fi


if [ -n "$START_LOCAL_SERVER" ]; then
    if [ -n "$CMD_DEPLOY_WEB" ] || [ -n "$CMD_DEPLOY_LIB" ]; then
        MY_IP=`/sbin/ifconfig eth0 | grep 'inet addr' | cut -d ':' -f 2 | cut -d ' ' -f 1`
        echo "------- preview -------"
        echo "http://$MY_IP:8000"
        echo "-----------------------"
        cd $PUBLIC_HTML_DIR
        python -c "import SimpleHTTPServer;SimpleHTTPServer.test()"
    fi
fi




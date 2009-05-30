#!/bin/bash

# Copyright (c) 2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file 
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

#set -x
set -f  # disable globbing

IGNORE_LIST="\
.*~
.*\.pyc
.*\.icc
.*\.icm
.*\.reg
.*\.png
.*\.qbk
.*\.svg
.*\.dia
.*\.xml
.*\.html?
.*/\.svn/.*
.*/\.git.*
./build/scripts/bitten_.*.ini.in
.*/test/apitest/resources/.*
.*/test/apitest/reference-pdf/.*
.*/tools/external/python/pygccxml/.*
.*/include/external/.*
\./external/.*
"

NO_LICENSE_LIST="
.*/md5\.cpp
.*/md5\.h
.*/scopeguard\.h
.*/doxyfile-template
.*/subversion_types.txt
./INSTALL.txt
./CREDITS.txt
./LICENSE.txt
.*/README
./doc/doxygen/Doxyfile
./doc/quickbook/boostbook_catalog.xml.in
./build/release-scripts/htmlfilt.conf
./build/release-scripts/jarda-home-build.bat
./build/cmake/FindICU.cmake
"

SOURCE_LIST="
.*\.c
.*\.cpp
.*\.h
.*\.h.in
.*\.hpp
.*\.i
.*\.i.in
.*\.java
.*\.py
"

function form_find_list
{
    echo -n "("
    for pattern in $1; do
        echo -n " -regex $pattern -o"
    done
    echo " -false )"
}

FIND_IGNORE=`form_find_list "$IGNORE_LIST"`
NO_LICENSE=`form_find_list "$NO_LICENSE_LIST"`

MAX_LINE=120
OPT_FIX_TABS=
OPT_FIX_TRAILING_SPACE=
OPT_REPORT_LONG_LINES=

function usage()
{
    cat <<EOF
usage: source_check.sh [options]

checks source code in the current directory

options
 --fix-tabs             replace tabs with spaces
 --fix-trailing-space   replace tabs with spaces
 --report-long-lines    report files with lines longer than $MAX_LINE
 --help                 this screen

EOF
}


# FIND_SOURCE=`form_find_list "$SOURCE_LIST"`
# find . -type f -not $FIND_IGNORE $FIND_SOURCE -exec sed -i "s/( /(/g;s/ )/)/g" {} \;
# exit 1


while true
do
    if [ "$1" == "--fix-tabs" ]; then
        OPT_FIX_TABS=1; shift
    elif [ "$1" == "--fix-trailing-space" ]; then
        OPT_FIX_TRAILING_SPACE=1; shift
    elif [ "$1" == "--report-long-lines" ]; then
        OPT_REPORT_LONG_LINES=1; shift
    elif [ "$1" == "--help" ]; then
        usage; exit 2
    else
        break
    fi
done


echo "== Files without license:"
find . -type f -not $FIND_IGNORE -not $NO_LICENSE -print0 | xargs -0 grep --files-without-match "MIT license"


# tabs
if [ -z "$OPT_FIX_TABS" ]; then
    echo "== Files with tabs (--fix-tabs):"
    find . -type f -not $FIND_IGNORE -print0 | xargs -0 grep --files-with-match $'\t'
else
    echo "== Fixing tabs"
    find . -type f -not $FIND_IGNORE -exec sed -i "s/\t/    /g" {} \;

fi

# trailing space
if [ -z "$OPT_FIX_TRAILING_SPACE" ]; then
    echo "== Files with trailing space (--fix-trailing-space):"
    find . -type f -not $FIND_IGNORE -print0 | xargs -0 grep --files-with-match -E '\s+$'
else
    echo "== Fixing trailing space"
    find . -type f -not $FIND_IGNORE -exec sed -i "s/[ \t]*$//g" {} \;

fi


# long lines
if [ -n "$OPT_REPORT_LONG_LINES" ]; then
    echo "== Files with lines longer than $MAX_LINE:"
    find . -type f -not $FIND_IGNORE -print0 | xargs -0 grep --files-with-match -E "^.{$MAX_LINE,}$"
fi





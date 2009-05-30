#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


#set -x
set -e

DIRNAME=$1
THIS_DIR=`dirname $0`
THIS_DIR=`cd $THIS_DIR && pwd`
TMP_FILE=`mktemp /tmp/postproc.XXXXXXXX`
trap "rm $TMP_FILE" INT TERM EXIT
EXIT_CODE=0

function no_lines_allowed()
{
    if [ "$1" != "0" ]; then
        cat $TMP_FILE
        EXIT_CODE=1
    fi
}

TIDY_OPTS="--write-back 1 --drop-proprietary-attributes 1 --output-xhtml 1 --gnu-emacs 1 --char-encoding utf8 -quiet --wrap 0"
find $1 -name '*.htm' -exec echo {} \; -exec tidy $TIDY_OPTS {} \;


echo '---- spellchecking ------------------------------------------------------'
$THIS_DIR/spellcheck.sh $1


echo '---- unresolved quickbook code templates --------------------------------'
no_lines_allowed `find $1 -name '*.htm' -print0 \
                 | xargs -0 grep -e '\[code_' \
                 | tee $TMP_FILE \
                 | wc -l`


echo '---- files that are not XHTML Strict --------------------------------'
no_lines_allowed `find $1 -name '*.htm' \
    -exec grep -L '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict' {} \; \
    | tee $TMP_FILE \
    | wc -l`


exit $EXIT_CODE
#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


#set -x

FOUND=/tmp/spelcheck.$$
RESULT=0
THIS_DIR=`dirname $0`
THIS_DIR=`cd $THIS_DIR && pwd`

#
#
#
function check
{
    case `basename $1` in
        CMakeLists.txt | \
            robots.txt ) 
        # do not check these
        return
        ;;
    esac

    $THIS_DIR/htmlfilt.py < $1 \
        | tee /tmp/spellcheck.log \
        | ispell -l -h \
        | sort -u \
        > $FOUND

    if [ -s $FOUND ]; then
        echo -------- $1 -----------
        cat $FOUND
        rm -f $FOUND
        RESULT=1
    fi
}


for f in $@; do
    if [ -f $f ]; then
        check $f
    elif [ -d $f ]; then
        for found in `find $f \( -name '*.txt' -or -name '*.htm' \)`; do
            check $found
        done
    fi
done

exit $RESULT

#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


help='
How to use this script:
1. go to the directory with the generated files
2. $ /path/to/cmppdf.sh file.pdf

Note that differences in non-printable characters are not visible as
such characters are discarded.

In case there are many different files after apitests do the following:
1. copy the output of the build to a file (e.g. /tmp/build.cmp)
2. go to the directory with the generated files
3. $ /path/to/cmppdf.sh /tmp/build.cmp
'

pwd=`pwd`
ref_pdfs=${pwd/jagbase*/jagbase\/code\/test\/apitest\/reference-pdf\/}

if [ "$1" == "" ]; then
    echo "$help"
    exit 2
fi


#
# Do actual comparison.
#
function do_compare
{
    basename=`basename $1`
    new=$1
    old=`find $ref_pdfs -name $basename`

    new_tmp=/tmp/$basename.new.$$
    old_tmp=/tmp/$basename.old.$$


    if grep -e FlateDecode $new > /dev/null ; then
        # uncompress content streams
        pdftk $new output $new_tmp uncompress
        pdftk $old output $old_tmp uncompress
    else
        cat $new > $new_tmp
        cat $old > $old_tmp
    fi

    #diff -u --text $old_tmp $new_tmp | colordiff | less -R
    wdiff -n $old_tmp $new_tmp | cat --show-nonprinting | colordiff | less -R

    rm $new_tmp $old_tmp
}


#
# Invoke comparison.
#
if [ "${1%%*.pdf}" == "" ]; then
    do_compare $1
else
    FILES=`cat $1 | grep FAILED | grep -o '[^ ]*\.pdf'`
    for pdf in $FILES;
    do
        echo -n "Show $pdf diff? [Y/n/q]"
        REPLY=bad
        while [ "$REPLY" = "bad" ]; do
            read -n 1 -s
            case "$REPLY" in
                n ) ;;
                q ) exit 0 ;;
                y | Y | "" ) do_compare $pdf ;;
                * ) REPLY=bad ;;
            esac
        done
        echo
    done
fi



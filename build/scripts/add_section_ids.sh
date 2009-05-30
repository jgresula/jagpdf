#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

set -e

# iterates over all .htm files listed in manifest $1 and add the 'id'
# attribute

if [[ ! "$1" =~ "manifest" ]] ; then
    echo "expected manifest file"
    exit 1
fi

HTML_DIR=`dirname $1`
cat $1 | while read htm;
do
    BASENAME=`basename $htm`
    ID=section_`echo $BASENAME | sed "s/\.htm//g"`
    # add $ID to the first occurence of <div class="section"...>
    sed -i "0,/<div \(class=\"section\"[^>]*\)>/s//<div \1 id=\"$ID\">/" "$HTML_DIR/$htm"
done

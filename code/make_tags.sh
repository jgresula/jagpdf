#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#



ALL_FILES=/tmp/ctags_all_files-$$

find . -name '*.cpp' > $ALL_FILES
find . -name '*.h' >> $ALL_FILES

cat $ALL_FILES | ctags -e -f ~/JAGBASE-TAGS -L -

rm $ALL_FILES



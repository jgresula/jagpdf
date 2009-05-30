#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


#
# Serches for messages that are not referenced from source code.
#
# Tip - messages have to be sorted in reverse order, otherwise we get
# false positives if one message is a prefix of another.
#

DEF_MESSAGES=/tmp/def-jmsg.$$

find .. -name '*.jmsg' -type f -print0 \
    | xargs -0 grep -hE '^[0-9]+' \
    | cut -d ' ' -f 2 \
    | while read line; do echo "msg_$line"; done \
    | sort -ru \
    > $DEF_MESSAGES

find .. \( -name '*.h' -o -name '*.cpp' \) -type f -print0 \
    | xargs -0 grep -ohf $DEF_MESSAGES \
    | sort -ru \
    | diff -u - $DEF_MESSAGES \
    | grep '^+'


rm $DEF_MESSAGES

exit 1
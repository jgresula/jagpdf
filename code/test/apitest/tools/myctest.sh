#!/bin/bash

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# myctest.sh [memcheck] ${CMAKE_TEST_COMMAND} <ctest args>

OUTPUT=/tmp/myctest.sh.$$

# retain only the last 15 log files, the sed command skips the first 15 lines
rm -f `ls -1t /tmp/myctest.sh* 2>/dev/null | sed -n '16,${p}'`

function quit()
{
    #rm -f $OUTPUT
    if [ -n "$2" ]; then
        echo "$2"
    fi
    exit $1
}

if [ "$1" != "memcheck" ]; then
    # standard cmake invocation
    cmd=$1; shift
    "$cmd" $@
else
    echo "* Running tests with memcheck."
    echo "* The output is sent to $OUTPUT"
    echo "* This may take some time ..."
    CTEST="$2"
    shift 2
    $CTEST -V --quiet --output-log $OUTPUT -T memcheck $@
    EXIT_CODE=$? #{PIPESTATUS[0]}
    # check that we can parse CTest output and that it exists
    if [ "0" == "`sed -n '/^-- Processing memory/,$p' $OUTPUT | wc -l`" ]; then
        quit 1 "Cannot find valgrind result in the CTest output"
    fi
    # fail test is based on the number of unexpected lines in CTest output
    # the sed commands prints output from regext to the end of the file
    NUM_ERRORS=`sed -n '/^-- Processing memory/,$p' $OUTPUT | grep -vE "^-- Processing|^Memory checking|^Potential Memory" | wc -l`
    if [ "0" != $NUM_ERRORS ]; then
        quit 1 "Memory errors were found."
    fi
    quit $EXIT_CODE
fi




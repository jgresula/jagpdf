#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import sys
import os
import re
import shutil

# Takes a file with extension .test, replaces the extension with
# .output and searches this file for errors. If some error is found
# then 'failed' is sent to stderr and stdout contains the reason


def failed( msg=None ):
    if msg:
        print msg
    print >>sys.stderr, 'failed'
    sys.exit(1)


def move_test_file( stem, suffix ):
    src = stem + suffix
    if os.path.isfile(src):
        shutil.move(src,  stem + suffix + ".moved" )


if len(sys.argv)!=2:
    failed( "an argument required" )

if not os.path.isfile(sys.argv[1]):
    failed( "file %s does not exist" % sys.argv[1] )  # the test itself failed

stem, ext = os.path.splitext( sys.argv[1] )
test_out = stem + ".output"

if not os.path.isfile(test_out):
    failed( "test output %s does not exist" % test_out )

valrex = re.compile( '^==[0-9]+==' )
lines = []
for line in open(test_out):
    if valrex.search( line ):
        lines.append( line.strip() )

if lines:
    move_test_file( stem, '.test' )
    move_test_file( stem, '.run' )
    move_test_file( stem, '.output' )
    failed( '\n' + test_out + ':\n' + '\n'.join( lines ) )

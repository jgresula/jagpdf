#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import sys
import os

# reads the content of the passed file and if it contains failed then
# it deletes the file and exits with an error exit code

def failed( msg=None ):
    if msg:
        print msg
    sys.exit(1)


if len(sys.argv)!=2:
    failed( "an argument required" )

for line in open(sys.argv[1]):
    if 'failed' in line:
        os.unlink(sys.argv[1])
        failed( "\n[jag test checker]: one or more tests failed\n" )
    if 'Traceback (most recent call last)' in line:
        failed("\n[jag test checker]: checking script failed!!:\n" + open(sys.argv[1]).read())
        os.unlink(sys.argv[1])


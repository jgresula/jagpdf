#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import fnmatch

pch_file = 'precompiled.h'
dirs_to_watch = [ 'src/resources',
                  'src/pdflib',
                  'src/core' ]

for dirname in [ os.path.abspath(os.path.join( '..', d )) for d in dirs_to_watch ]:
    assert os.path.isdir(dirname)
    for dirpath, dirnames, fnames in os.walk( dirname ):
        files = [ os.path.join( dirpath, f ) for f in fnmatch.filter(fnames, '*.cpp') ]
        for f in files:
            content = open(f).read()
            if pch_file not in content:
                file(f,"wt").write( "#include <%s>\n" % pch_file + content )
                #print f, '- no pch'


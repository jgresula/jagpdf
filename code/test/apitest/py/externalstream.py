#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import jagpdf
import jag.testlib as testlib
import sys
import os

def test_main(argv=None):
    if None == argv:
        argv = sys.argv
    out_file = os.path.abspath(os.path.join(argv[1], 'basic_extstream.pdf'))
    cfg = testlib.test_config()
    doc = jagpdf.create_stream(testlib.FileStreamOut(out_file), cfg)
    doc.page_start(5.9*72, 3.5*72)
    writer = doc.page().canvas()
    doc.page_end()
    doc.finalize()
    doc = None

if __name__ == "__main__":
    test_main()


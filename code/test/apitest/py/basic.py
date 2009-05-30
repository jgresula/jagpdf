#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import sys
import os
import jag.testlib as testlib

def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'basic.pdf')

    doc.page_start(5.9*72, 3.5*72)
    writer = doc.page().canvas()
    doc.page_end()

    doc.finalize()

if __name__ == '__main__':
    test_main()

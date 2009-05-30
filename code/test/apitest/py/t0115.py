# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

#!/usr/bin/env python
# $Id$
import jagpdf
import sys
import os
import jag.testlib as testlib


def test_main(argv=None):
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    doc.page_start(597.6, 848.68)
    canvas = doc.page().canvas()
    #
    gradient = doc.function_2_load("domain=0, 1; c0=0; c1=1")
    p = doc.shading_pattern_load("axial; coords=0, 0, 400, 400",
                                 jagpdf.CS_DEVICE_GRAY,
                                 gradient)
    # cannot set pattern because the pattern color space has not been set
    testlib.must_throw(canvas.pattern, "f", p)
    #
    doc.page_end()
    doc.finalize()

    
if __name__ == '__main__':
    test_main()

    

      


        

    
        


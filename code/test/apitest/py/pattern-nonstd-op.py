#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import os
import sys

def test_main(argv=None):
    #raw_input('.......')
    if argv==None:
        argv=sys.argv
    out_file = os.path.abspath(os.path.join(argv[1], 'pattern-nonstd-op.pdf'))
    cfg = testlib.test_config()
    doc = jagpdf.create_stream(testlib.FileStreamOut(out_file), cfg)

    patt_canvas = doc.canvas_create()
    testlib.must_throw(doc.tiling_pattern_load,
                       "step=0, 0",
                       patt_canvas) # empty canvas
    patt_canvas.move_to(5, 5)
    patt_canvas.line_to(15, 15)
    patt_canvas.path_paint("fs")
    testlib.must_throw(doc.tiling_pattern_load,
                       "matrix=1, 0, 0, 1, 0, 0",
                       patt_canvas) # no step
    patt_id = doc.tiling_pattern_load("step=20, 20", patt_canvas)

    # use pattern
    doc.page_start(5.9*72, 3.5*72)
    writer = doc.page().canvas()
    writer.color_space_pattern_uncolored("f", jagpdf.CS_DEVICE_GRAY)
    writer.pattern("f", patt_id, .5)
    writer.rectangle(36, 36, 144, 144)
    writer.path_paint("sf")

    #try to write to pattern that is already registered
    patt_canvas.rectangle(7, 7, 6, 6)
    patt_canvas.path_paint("fs")
    writer.rectangle(36+144+4, 36, 144, 144)
    writer.path_paint("sf")

    doc.page_end()

    # !!!! suprising - try to write to pattern that is already outputted
    patt_canvas.circle(10, 10, 5)
    patt_canvas.path_paint("s")

    # !!!! this is wild -> causes abort
    #patt_canvas.paint("fs")

    doc.finalize()
    #open("patt.pdf", "wb").write(stream.content())

if __name__ == "__main__":
    test_main()


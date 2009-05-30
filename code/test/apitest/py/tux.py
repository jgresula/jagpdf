#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import os
import jag.testlib as testlib
import jag.svg as svg

def test_main(argv=None):
    tux_svg = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/tux.svg')
    cfg = jagpdf.create_profile()
    cfg.set('doc.compressed', '1')
    doc = testlib.create_test_doc(argv, 'tux.pdf', cfg)
    media = 450, 500
    doc.page_start(*media)
    canvas = doc.page().canvas()
    canvas.translate(50, media[1]-50)
    canvas.scale(1.0, -1.0)
    svg.paint_to_canvas(canvas, tux_svg)
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()



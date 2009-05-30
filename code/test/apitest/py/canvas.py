#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib

def test_main(argv=None):
    doc = testlib.create_test_doc(argv, "canvas.pdf")
    fnt = testlib.EasyFont(doc)
    # create a canvas
    # canvas = doc.create_canvas()
    # canvas.rectangle(36, 36, 72, 72)
    # canvas.path_paint("s")
    for i in range(5):
        doc.page_start(144, 144)
        #    doc.page().add_canvas(0, 0, canvas)
        pg_canvas = doc.page().canvas()
        pg_canvas.text_font(fnt())
        pg_canvas.text(72, 72, "page %i" % i)
        doc.page_end()
    doc.finalize()

if __name__ == '__main__':
    test_main()


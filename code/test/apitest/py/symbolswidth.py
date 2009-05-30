#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import jag.testlib as testlib

def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'symbolswidth.pdf')
    sym_font = doc.font_load("standard; name=ZapfDingbats; size=15")
    doc.page_start(500, 100)
    canvas = doc.page().canvas()
    s = "".join(chr(i) for i in range(33,67))
    w = sym_font.advance(s)
    canvas.text_font(sym_font)
    canvas.color("s", 0.5)
    canvas.rectangle(10, 10, w, 80)
    canvas.path_paint("s")
    canvas.text(10, 30, s)
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()


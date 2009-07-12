#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib

# AY and YA seems to have the largest kerning

def out_text(canvas, font, x, y, txt):
    canvas.text_font(font)
    canvas.text(x, y, txt)
    canvas.rectangle(x, y-5, font.advance(txt), 20)
    canvas.path_paint('s')
    

def test_main(argv=None):
    profile = testlib.test_config()
    profile.set("text.kerning", "1")
    doc = testlib.create_test_doc(argv, 'kerning2.pdf', profile)
    font = testlib.EasyFontTTF(doc)
    font_core = testlib.EasyFont(doc)
    pheight = 70
    doc.page_start(7*72, pheight)
    canvas = doc.page().canvas()


    txt = "AYAYAYAYAYAYAYA YAYAYAYAYAYAYAYAYA YAYAYAYAYAYAYAY AYAYAYAYAY"
    out_text(canvas, font(12), 10, 20, txt)
    out_text(canvas, font_core(12), 10, 45, txt)


    doc.page_end()
    doc.finalize()


if __name__ == '__main__':
    test_main()


#!/usr/bin/env python

# Copyright (c) 2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file 
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

import jag.testlib as testlib
import os
import jagpdf

fspec = 'size=12;file=${JAG_TEST_RESOURCES_DIR}/fonts/DejaVuSans.ttf'

def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'glyph_index.pdf')
    font = doc.font_load(os.path.expandvars(fspec))
    font.glyph_width(74)
    font.glyph_width(0xfffe)
    doc.page_start(300, 200)
    canvas = doc.page().canvas()
    canvas.text_font(font)
    glyphs = [74, 79, 92, 83, 75, 86]
    canvas.text_glyphs(20, 20, glyphs)
    canvas.text_glyphs(20, 40, glyphs, [-130.0, -130.0], [2, 3])
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()

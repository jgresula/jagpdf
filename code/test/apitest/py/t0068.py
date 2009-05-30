#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib


def cannot_output_font(easy_font, enc=None):
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    fnt = easy_font(doc, enc)
    doc.page_start(*testlib.paperA5)
    canvas = doc.page().canvas()
    canvas.text_font(fnt())
    canvas.state_save()
    canvas.state_restore()
    doc.page_end()
    testlib.must_throw_ex("font", doc.finalize)

def test_main(argv=None):
    cannot_output_font(testlib.EasyFont)
    cannot_output_font(testlib.EasyFontTTF, 'windows-1252')
    cannot_output_font(testlib.EasyFontTTF, 'utf-8')

if __name__ == "__main__":
    test_main()


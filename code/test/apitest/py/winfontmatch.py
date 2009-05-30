#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import sys
import jag.testlib as testlib

# tests TrueType system font selection

def test_main(argv=None):
    if 'win32' not in sys.platform:
        return

    doc = testlib.create_test_doc(argv, 'fontmatch-windows-only.pdf')
    doc.page_start(600, 600)
    canvas = doc.page().canvas()

    utf8 = 'P\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88 \xc3\xbap\xc4\x9bl \xc4\x8f\xc3\xa1belsk\xc3\xa9 \xc3\xb3dy.'

    # utf-8
    font = doc.font_load("name=Verdana; size=14; italic; enc=utf-8")
    canvas.text_font(font)
    canvas.text(20, 70, "Verdana Italic UTF-8: " + utf8)
    # unicode
    uni = utf8.decode('utf-8')
    canvas.text(20, 100, "Verdana Italic Unicode: " + uni)
    # cp 1250
    text_1250 = unicode(utf8, 'utf8').encode('cp1250')
    font = doc.font_load("name=Verdana; size=14; italic; enc=windows-1250")
    canvas.text_font(font)
    canvas.text(20, 130, "Verdana Italic cp-1250: " + text_1250)
    # font mapping requires encoding; the error message should
    # express this
    testlib.must_throw_ex('requires an encoding',
                          doc.font_load,
                          "name=Verdana; size=14; italic")

    doc.page_end()
    doc.finalize()


if __name__ == "__main__":
    test_main()


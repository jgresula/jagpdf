#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib


def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'defaultfont2.pdf')
    doc.page_start(200, 36)
    doc.page().canvas().text(10, 10, 'written in the default font')
    doc.page_end()

    doc.page_start(200, 48)
    canvas = doc.page().canvas()
    canvas.state_save()
    courier = doc.font_load('standard;name=Courier;size=10')
    canvas.text_font(courier)
    canvas.text(10, 10, 'written in Courier')
    canvas.state_restore()
    doc.page().canvas().text(10, 30, 'written in the default font')
    doc.page_end()

    doc.page_start(200, 36)
    doc.page().canvas().text(10, 10, 'written in the default font')
    doc.page_end()


    doc.finalize()

if __name__ == "__main__":
    test_main()


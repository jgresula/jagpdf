# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import os
import jag.testlib as testlib
import sys



def do_page(writer):
    writer.page_start(3*72, 80)
    page = writer.page().canvas()
    font = testlib.EasyFont(writer)
    page.text_font(font())

    page.color_space("f", jagpdf.CS_DEVICE_RGB)
    page.color("f", .7, 0, 0)
    page.text(20, 20, "This text should be red!")
    page.state_save()
    page.color_space("f", jagpdf.CS_DEVICE_CMYK)
    page.color("f", 1, 1, 0, .45)
    page.text(20, 40, "This text should be dark blue!")
    page.state_restore()
    page.text(20, 60, "This text should be red again!")
    writer.page_end()


def test_main(argv=None):
    doc_writer = testlib.create_test_doc(argv, 'colorstate.pdf')
    do_page(doc_writer)
    doc_writer.finalize()

if __name__ == '__main__':
    test_main()

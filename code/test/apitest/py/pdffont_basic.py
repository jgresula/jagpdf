# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import os
import jag.testlib as testlib

def page_with_simple_font(doc_writer):
    doc_writer.page_start(5*72, 5*72)
    page = doc_writer.page().canvas()
    for i in range(5, 20):
        font = doc_writer.font_load(os.path.expandvars('size=%d; file=${JAG_TEST_RESOURCES_DIR}/fonts/DejaVuSans.ttf' % i))
        page.text_font(font)
        page.text(20, 20+(i-5)*22, "Page with simple font!")
    doc_writer.page_end()

def page_with_composite_font(doc_writer):
    doc_writer.page_start(5*72, 2*72)
    page = doc_writer.page().canvas()
    font = doc_writer.font_load(os.path.expandvars('size=12; file=${JAG_TEST_RESOURCES_DIR}/fonts/DejaVuSans.ttf; enc=windows-1255'))
    page.text_font(font)
    page.text(72, 72, "Page with composite font!")
    doc_writer.page_end()

def test_main(argv=None):
    doc_writer = testlib.create_test_doc(argv, 'pdffont_basic.pdf')
    page_with_simple_font(doc_writer)
    page_with_composite_font(doc_writer)
    doc_writer.finalize()

if __name__ == '__main__':
    test_main()


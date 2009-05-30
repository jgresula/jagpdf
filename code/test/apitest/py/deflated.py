# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import os

def do_basic_page(writer):
    writer.page_start(3*72, 3*72)
    page = writer.page().canvas()
    for i in range(10):
        c = i*3
        page.rectangle(72+c, 72+c, 72-2*c, 72-2*c)
        page.path_paint("s")
    writer.page_end()

def test_main(argv=None):
    cfg = testlib.test_config()
    cfg.set('doc.compressed', "1")
    doc_writer = testlib.create_test_doc(argv, 'deflated.pdf', cfg)
    do_basic_page(doc_writer)
    doc_writer.finalize()

if __name__ == '__main__':
    test_main()

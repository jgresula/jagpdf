# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# produces two pages with a black rectangle
# on the first page the rectangle looks ok
# on the second page looks faded (due to image with a soft mask)

import jagpdf
import jag.testlib as testlib
import os

def do(doc):
    doc.page_start(72, 72)
    page = doc.page().canvas()
    page.rectangle( 10, 10, 52, 10)
    page.path_paint("fs")
    doc.page_end()

    doc.page_start(72, 72)
    page = doc.page().canvas()
    page.rectangle( 10, 10, 52, 10)
    page.path_paint("fs")

    png = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/png_test_suite/BGGN4A16.png')
    img = doc.image_load_file(png, jagpdf.IMAGE_FORMAT_PNG)
    page.image(img, 10, 25)
    doc.page_end()


def test_main(argv=None):
    doc, cfg = testlib.get_legacy_doc(argv, "smask-color.pdf", { 'doc.version' : 5 })
    do(doc)
    doc.finalize()

if __name__ == '__main__':
    test_main()

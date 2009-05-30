# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import os

def do(doc):
    doc.page_start(60, 60)
    page = doc.page().canvas()
    png = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/png_test_suite/BASI6A16.png')
    img = doc.image_load_file(png, jagpdf.IMAGE_FORMAT_PNG)
    page.image(img, 10, 10)
    doc.page_end()


def test_main(argv=None):
    doc, cfg = testlib.get_legacy_doc(argv,
                                       'smask16.pdf',\
                                       { 'doc.version' : 5,\
                                         'images.softmask_16_to_8' : 0})
    do(doc)
    doc.finalize()

if __name__ == '__main__':
    test_main()

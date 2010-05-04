# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jagpdf
import jag.testlib as testlib

def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'jpeg_icc_gray.pdf')
    img = doc.image_load_file(os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/1279100_32.jpg'))
    doc.page_start(600, 600)
    c = doc.page().canvas()
    c.image(img, 10, 10)
    doc.page_end()
    doc.finalize()


if __name__ == '__main__':
    test_main()

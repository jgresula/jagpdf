# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


#test that embeded icc profile is downgraded to device rgb
#when the version is < 1.3
import os
import jag.testlib as testlib
import jagpdf

def do(writer):
    writer.page_start(8.3*72+6, 8.3*72+6)
    page = writer.page().canvas()
    img_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/lena_srgb.jpg')
    page.image(writer.image_load_file(img_file, jagpdf.IMAGE_FORMAT_JPEG), 2, 2)
    writer.page_end()


def test_main(argv=None):
    cfg = testlib.test_config()
    cfg.set('doc.version', "2")
    doc_writer = testlib.create_test_doc(argv, 'img-jpegicc-1_2.pdf', cfg)
    do(doc_writer)
    doc_writer.finalize()

if __name__ == '__main__':
    test_main()

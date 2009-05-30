# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# palette
# iCC chunk
# sRGB chunk
import jagpdf
import jag.testlib as testlib
import os

def do(writer):
    writer.page_start(2*72, 1.5*72)
    page = writer.page().canvas()
    icc_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/cc3399-icc.png')
    srgb_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/cc3399-srgb.png')
    page.image(writer.image_load_file(icc_file, jagpdf.IMAGE_FORMAT_PNG), 10, 30)
    page.image(writer.image_load_file(srgb_file, jagpdf.IMAGE_FORMAT_PNG), 60, 30)
    writer.page_end()


def test_main(argv=None):
    doc_writer = testlib.create_test_doc(argv, 'img-png-srgb.pdf')
    do(doc_writer)
    doc_writer.finalize()

if __name__ == '__main__':
    test_main()

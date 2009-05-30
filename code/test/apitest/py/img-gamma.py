# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import os
import jag.testlib as testlib

g_font = testlib.EasyFont()

def do_basic_page(writer):
    img_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/lena_uncalibrated.jpg')
    for g in [1.0, 1.4, 1.8, 2.2]:
        writer.page_start(5.6*72 + 4, 5.6*72 + 34)
        page = writer.page().canvas()
        img_spec = writer.image_definition()
        img_spec.format(jagpdf.IMAGE_FORMAT_JPEG)
        img_spec.file_name(img_file)
        img_spec.gamma(g)
        img = writer.image_load(img_spec);
        page.image(img, 2, 32)
        page.text_font(g_font(12))
        page.text(10, 10, "gamma: " + str(g))
        writer.page_end()


def do_gamma_preserve(writer):
    img_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/lena_uncalibrated.jpg')
    writer.page_start(5.6*72 + 4, 6.6*72 + 34)
    page = writer.page().canvas()

    page.color_space("fs", jagpdf.CS_DEVICE_RGB)
    page.color("fs", .7, .2, .5)
    page.rectangle(2, 10, 5.6*72, 72)
    page.path_paint("fs")
    img_spec = writer.image_definition()
    img_spec.format(jagpdf.IMAGE_FORMAT_JPEG)
    img_spec.file_name(img_file)
    img_spec.gamma(2.2)
    img = writer.image_load(img_spec);
    page.image(img, 2, 32+74)
#    page.color("fs", .7, .8, .5)
    page.rectangle(46, 28, 4.2*72, 36)
    page.path_paint("fs")
    page.color("fs", .9, .9, .9)
    page.text_font(g_font(8))
    page.text(10, 12, "[preserve gamma:] two rectangles in the same color, one contained in another")
    writer.page_end()

def test_main(argv=None):
    doc_writer = testlib.create_test_doc(argv, 'img-gamma.pdf')
    g_font.set_writer(doc_writer)
    do_gamma_preserve(doc_writer)
    do_basic_page(doc_writer)
    doc_writer.finalize()

if __name__ == '__main__':
    test_main()

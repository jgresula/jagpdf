# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jag.testlib as testlib
import jagpdf

g_font = testlib.EasyFont()

def image_muster(writer, fname, pg_dim, text, dpi=72.0):
    writer.page_start(*pg_dim)
    page = writer.page().canvas()
    img_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/') + fname
    img = writer.image_load_file(img_file, jagpdf.IMAGE_FORMAT_JPEG);
    assert(dpi==img.dpi_x())
    page.image(img, 2, 32)
    page.text_font(g_font(12))
    page.text(10, 10, text)
    writer.page_end()


def do_jfif_96(writer):
    image_muster(writer, 'lena_96dpi.jpg', (4.0*72 + 6, 3.7*72 + 36), "JPEG with dpi set in JFIF (no EXIF)", 96)

def do_exif_little_endian_srgb(writer):
    image_muster(writer, 'lena_be_srgb.jpg', (5.0*72 + 6, 4.3*72 + 36), "JPEG with little endian EXIF, sRGB defined in EXIF")

def do_exif_little_endian(writer):
    image_muster(writer, 'lena_be_uncalibrated.jpg', (5.0*72 + 6, 4.3*72 + 36), "JPEG with little endian EXIF, sRGB NOT defined in EXIF")

def do_jpeg_cmyk(writer):
    image_muster(writer, 'mandrill_cmyk.jpg', (5.8*72 + 8, 4.5*72 + 36), "JPEG CMYK with EXIF")

def do_lenghty_icc(writer):
    image_muster(writer, 'mandrill_cmyk_icc.jpg', (5.8*72 + 8, 4.5*72 + 36), "JPEG CMYK with EXIF, longish ICC profile (Europe ISO Coated FOGRA27)")

def do_simple_icc(writer):
    image_muster(writer, 'lena_srgb.jpg', (5.0*72 + 6, 4.3*72 + 36), "JPEG RGB, ICC profile (sRGB)")


def do_basic_page(writer):
    images = ['lena_nodpi.jpg', 'lena_gray_nodpi.jpg']
    writer.page_start(5.0*72, 8.0*72)
    page = writer.page().canvas()
    y = 32
    for img_file in [os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/') + stem for stem in images]:
        img = writer.image_load_file(img_file, jagpdf.IMAGE_FORMAT_JPEG);
        page.image(img, 20, y)
        assert(92 == img.dpi_x())
        y += 10 + 72.0*img.height()/float(img.dpi_y())
    page.text_font(g_font(10))
    page.text(10, 10, "JPEG - JFIF with no resolution set")
    writer.page_end()

def test_main(argv=None):
    cfg = testlib.test_config()
    cfg.set('images.default_dpi', '92')
    doc_writer = testlib.create_test_doc(argv, 'img-jpeg.pdf',cfg)
    g_font.set_writer(doc_writer)
    do_jfif_96(doc_writer)
    do_exif_little_endian_srgb(doc_writer)
    do_exif_little_endian(doc_writer)
    do_lenghty_icc(doc_writer)
    do_jpeg_cmyk(doc_writer)
    do_simple_icc(doc_writer)
    do_basic_page(doc_writer)
    doc_writer.finalize()


if __name__ == '__main__':
    test_main()

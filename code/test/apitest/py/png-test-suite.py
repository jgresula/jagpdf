# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jag.imagemanip as imagemanip
import jag.testlib as testlib
import jagpdf
import os
import glob
import sys

# Covers PNG test suite -  http://www.libpng.org/pub/png/pngsuite-all-good.html
# Test are performed for various PDF versions.
#
# There are two files tbrn2c08, TBBN1G04 having color key mask set to mid-gray.
# Photoshop and irfanview seems to ignore it whereas gimp2 and firefox apply the mask.
# We behave in the same way as gimp2/firefox.
#
# cdun2c08 is a low resolution image (25dpi). None of the image viewers/applications
# respect it (even Acrobat, but that can be due to some dpi threshold policy). Its
# printsize should be 1.3 inch (value from irfan) - verified that we generate it at that size.

g_font = testlib.EasyFont()
g_png_dir = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/png_test_suite/')



def test_corrupted_files(writer):
    for png in glob.glob(g_png_dir + 'X*.png'):
        testlib.must_throw(writer.image_load_file, png, jagpdf.IMAGE_FORMAT_PNG)


def do_page_generic(writer, height, pattern, title):
    writer.page_start(5*72, height)
    page = writer.page().canvas()
    page.state_save()
    page.color_space("fs", jagpdf.CS_DEVICE_RGB)
    page.color("fs", 176.0/255, 197.0/255, 230.0/255)
    page.rectangle(2, 2, 5*72-4, height-4)
    page.path_paint("fs")
    page.state_restore()

    pngs = glob.glob(g_png_dir + pattern)
    pngs.sort()
#    print '>>>', pngs
    grid = imagemanip.grid_coords(5, 50)
    placer = imagemanip.ImagePlacer(writer, page, 20, 20, 60, 8)
    page.text_font(g_font(8))
    for png in pngs:
        img = writer.image_load_file(png, jagpdf.IMAGE_FORMAT_PNG)
        placer(img, os.path.splitext(os.path.basename(png))[0], *grid.next())
    page.text_font(g_font(14))
    page.text(2, height-20, title)
    writer.page_end()


def do_suite(writer):
    do_page_generic(writer, 3*72, 'BG*.png', 'BACKGROUND')
    do_page_generic(writer, 3.5*72, 'BASN*.png', 'BASIC NON-INTERLACED')
    do_page_generic(writer, 3.5*72, 'BASI*.png', 'BASIC ADAM-7 INTERLACED')
    do_page_generic(writer, 1.7*72, 'Z*.png', 'COMPRESSION LEVEL')
    do_page_generic(writer, 2.5*72, 'O*.png', 'CHUNK ORDERING')
    do_page_generic(writer, 4.1*72, 'C*.png', 'ANCILLARY CHUNKS') #??, chromacity
    do_page_generic(writer, 2.5*72, 'P*.png', 'PALETTES') #?? text color
    do_page_generic(writer, 2.5*72, 'F*.png', 'FILTERING')
    do_page_generic(writer, 5*72, 'G*.png', 'GAMMA')
    do_page_generic(writer, 3.5*72, 'T*.png', 'TRANSPARENCY')
    do_page_generic(writer, 8.6*72, 'S*.png', 'SIZES')

def test_main(argv=None):
    out_files = ["png-test-suite15.pdf",
                  "png-test-suite14.pdf",
                  "png-test-suite13.pdf",
                  "png-test-suite12.pdf"]
    cfg = testlib.test_config()
    for i, ver in [(0,5), (1,4), (2,3), (3,2)]:
        cfg.set('doc.version', str(ver))
        doc_writer = testlib.create_test_doc(argv, out_files[i], cfg)
        g_font.set_writer(doc_writer)
        do_suite(doc_writer)
        test_corrupted_files(doc_writer)
        doc_writer.finalize()

if __name__ == '__main__':
    test_main()




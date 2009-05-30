# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import jagpdf
import jag.testlib as testlib
import os

g_png_dir = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/')

media = 400, 400

def get_pattern(doc):
    pcell = doc.canvas_create()
    pcell.color_space('f', jagpdf.CS_DEVICE_GRAY)
    pcell.color('f', 0)
    pcell.rectangle(0, 0, 10, 10)
    pcell.rectangle(10, 10, 10, 10)
    pcell.path_paint('f')
    return doc.tiling_pattern_load('step=20, 20', pcell)

def image_pos(img):
    img_width = img.width() / img.dpi_x() * 72
    img_height = img.height() / img.dpi_y() * 72
    x = (media[0]-img_width) / 2
    y = (media[1]-img_height) / 2
    return x, y

def test_main(argv=None):
    cfg = jagpdf.create_profile()
    cfg.set("doc.compressed", "1")
    doc = testlib.create_test_doc(argv, 'transparent_cubes.pdf', cfg)
    img = doc.image_load_file(os.path.join(g_png_dir, 'cubes_transparent.png'))
    doc.page_start(*media)
    canvas = doc.page().canvas()
    canvas.color_space_pattern('f')
    canvas.pattern('f', get_pattern(doc))
    canvas.rectangle(0, 0, *media)
    canvas.path_paint('f')
    canvas.image(img, *image_pos(img))
    doc.page_end()
    doc.finalize()

if __name__ == '__main__':
    test_main()

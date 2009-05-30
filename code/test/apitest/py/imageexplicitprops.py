# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jag.testlib as testlib
import jag.imagemanip as imagemanip
import jagpdf

g_image_dir = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images')
g_jpeg_file = os.path.join(g_image_dir, "lena.jpg")
g_png_file = os.path.join(g_image_dir, "lena_alpha.png")
g_font = testlib.EasyFont()


def get_pattern(doc):
    pcell = doc.canvas_create()
    pcell.color_space('f', jagpdf.CS_DEVICE_GRAY)
    pcell.color('f', 0.8)
    pcell.rectangle(0, 0, 10, 10)
    pcell.rectangle(10, 10, 10, 10)
    pcell.path_paint('f')
    return doc.tiling_pattern_load('step=20, 20', pcell)

def paint_background(doc, media):
    canvas = doc.page().canvas()
    canvas.state_save()
    canvas.color_space_pattern('f')
    canvas.pattern('f', get_pattern(doc))
    canvas.rectangle(0, 0, *media)
    canvas.path_paint('f')
    canvas.state_restore()

def do_props_page(doc):
    def reg_spec(doc, meth_str, *args):
        spec = doc.image_definition()
        spec.format(jagpdf.IMAGE_FORMAT_JPEG)
        spec.file_name(g_jpeg_file)
        if meth_str:
            meth = getattr(spec, meth_str)
            meth(*args)
        return doc.image_load(spec)

    media = 9.3*72, 15.7*72
    doc.page_start(*media)
    page = doc.page().canvas()
    page.text_font(g_font(10))
    placer = imagemanip.ImageGrid(doc, (10,10), 2, (4.5*72,4.5*72), 12, 5)

    srgb = doc.color_space_load("srgb")
    placer(reg_spec(doc, None), "original")
    placer(reg_spec(doc, "gamma", 1.8), "gamma 1.8")
    placer(reg_spec(doc, "color_space", srgb), "srgb")
    placer(reg_spec(doc, "decode", [1,0,1,0,1,0]), "decode")
    img_alter = reg_spec(doc, "decode", [.5, 0, .5, 0, .5, 0])
    placer(reg_spec(doc, "alternate_for_printing", img_alter), "alternate for print")
    placer(reg_spec(doc, "dpi", 144, 144), "dpi 144 (the others are 72)")
    doc.page_end()



def do_masks_page(doc):
    def reg_spec(doc, meth_str, *args):
        spec = doc.image_definition()
        spec.format(jagpdf.IMAGE_FORMAT_PNG)
        spec.file_name(g_png_file)
        if meth_str:
            meth = getattr(spec, meth_str)
            meth(*args)
        return doc.image_load(spec)

    media = 9.3*72, 10*72
    doc.page_start(*media)
    paint_background(doc, media)

    page = doc.page().canvas()
    page.text_font(g_font(10))
    placer = imagemanip.ImageGrid(doc, (10,10), 2, (4.5*72,4.5*72), 12, 5)
    placer(reg_spec(doc, None), "original (with alpha channel)")

    mask_dim = 72, 72
    gimg = imagemanip.image(imagemanip.InvertedEllipseC, *mask_dim)
    smspec = doc.define_image_mask()
    smspec.data(imagemanip.pack_bits(gimg, 8, 1, *mask_dim))
    smspec.dimensions(*mask_dim)
    smspec.bit_depth(8)
    smid = doc.register_image_mask(smspec)
    placer(reg_spec(doc, "image_mask", smid), "custom soft mask")
    placer(reg_spec(doc, "color_key_mask", [0,127,0,127,0,255]), "color key mask")
    gimg = imagemanip.image(imagemanip.InvertedEllipseC, *mask_dim)

    smspec = doc.define_image_mask()
    smspec.data(imagemanip.pack_bits(gimg, 1, 1, *mask_dim))
    smspec.dimensions(*mask_dim)
    smspec.bit_depth(1)
    smid = doc.register_image_mask(smspec)
    placer(reg_spec(doc, "image_mask", smid), "custom mask")
    doc.page_end()

def test_main(argv=None):
    doc = testlib.create_test_doc(argv, "imageexplicitprops.pdf")
    g_font.set_writer(doc)
    do_props_page(doc)
    do_masks_page(doc)
    doc.finalize()

if __name__ == '__main__':
    test_main()





# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jag.imagemanip as imagemanip
import jag.testlib as testlib
import jagpdf
import tempfile
import sys

g_font = testlib.EasyFont()
g_temp_files = testlib.TemporaryFiles()
g_img_dim = 72, 72

class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

    def add(self, **kwds):
        self.__dict__.update(kwds)
        return self


def prepare_page(doc, dim):
    doc.page_start(*dim)
    page = doc.page().canvas()
    testlib.do_background(page, (0.6, 0.3, 0.2), dim, 5)
    page.color_space("fs", jagpdf.CS_DEVICE_GRAY)
    page.color("fs", 1.0)
    page.text_font(g_font(6))
    grid = imagemanip.grid_coords(3, 80)
    placer = imagemanip.ImagePlacer(doc, page, 20, 20, 95, 10)
    return page, grid, placer


def default_cfg():
    return Bunch(img_dim=g_img_dim, spec_fn=None, bits_a = [16, 8, 4, 2, 1], dpi=(72,72), from_file=False)

g_img_cache = imagemanip.img_cache()

def get_img_id(doc, image, bpc, nr_channels, cs, p = default_cfg()):
    spec = doc.image_definition()
    spec.dimensions(*p.img_dim)
    spec.bits_per_component(bpc)
    spec.dpi(*p.dpi)
    spec.color_space(cs)
    spec.format(jagpdf.IMAGE_FORMAT_NATIVE)
    img_data = g_img_cache.img_data(image, bpc, nr_channels, *p.img_dim)
    if p.from_file:
        handle, tmp_file = tempfile.mkstemp()
        img_data.tofile(open(tmp_file,'wb'))
        os.close(handle)
        g_temp_files.add(tmp_file)

    if p.from_file:
        spec.file_name(tmp_file)
    else:
        spec.data(img_data)
    desc = "%d levels per channel (%d %s)" % (2**bpc, bpc, bpc > 1 and "bits" or "bit")
    if p.spec_fn:
        desc2 = p.spec_fn(spec)
        if desc2:
            desc = desc2
    return doc.image_load(spec), desc


def do_generic(doc, image, cs, nr_channels, title, p = default_cfg()):
    page_dim = 4.8*72, 3.8*72
    page, grid, placer = prepare_page(doc, page_dim)
    for bpc in p.bits_a:
        img, desc = get_img_id(doc, image, bpc, nr_channels, cs, p)
        placer(img, desc, *grid.next())

    page.text_font(g_font(14))
    page.text(20, page_dim[1] - 30, title)
    doc.page_end()


#odd mask goes through file, even through memory
class SoftMask:
    def set_doc(self, doc):
        self.dim = g_img_dim # the same because of matte
        self.doc = doc
        self.registry = {}
        self.src_img_registry = {}
        self.generic = imagemanip.image(imagemanip.LineX, *self.dim)
        self.test_errors()
        self.req_nr = 0

    def test_errors(self):
        pass

    def id(self, bit_depth, interpolate, matte=None, decode=None):
        key = (bit_depth, decode, interpolate, matte)
        if key in self.registry:
            return self.registry[key]
        else:
            self.req_nr += 1
            spec = self.doc.define_image_mask()
            spec.dimensions(*self.dim)
            if interpolate:
                spec.interpolate(interpolate)
            spec.bit_depth(bit_depth)
            if matte:
                spec.matte(matte)
            if decode:
                spec.decode(*decode)
            if bit_depth in self.src_img_registry:
                mask_data = self.src_img_registry[bit_depth]
            else:
                mask_data = imagemanip.pack_bits(self.generic, bit_depth, 1, *self.dim)
                self.src_img_registry[bit_depth] = mask_data
            if self.req_nr%2:
                handle, tmp_file = tempfile.mkstemp()
                mask_data.tofile(open(tmp_file,'wb'))
                os.close(handle)
                g_temp_files.add(tmp_file)
                spec.file_name(tmp_file)
            else:
                spec.data(mask_data)
            id = self.doc.register_image_mask(spec)
            self.registry[key] = id
            return id

g_softmask = SoftMask()


class soft_mask_fn:
    TEST_BIT_DEPTH, TEST_INTERPOLATE, TEST_DECODE, TEST_MATTE = range(4)

    def __init__(self, test_type, nr_channels):
        self.val = { soft_mask_fn.TEST_BIT_DEPTH : [None,\
                                        (16, 0, None, None),\
                                        (8, 0, None, None),\
                                        (4, 0, None, None),\
                                        (2, 0, None, None),\
                                        (1, 0, None, None)
                                       ],\
                     soft_mask_fn.TEST_INTERPOLATE :  [(8, 0, None, None),\
                                          (8, 1, None, None),\
                                          (4, 0, None, None),\
                                          (4, 1, None, None),\
                                          (2, 0, None, None),\
                                          (2, 1, None, None)],\
                     soft_mask_fn.TEST_DECODE :  [(8, 0, None, (1,0)),\
                                          (8, 0, None, (0,1)),\
                                          (8, 0, None, (0,.5)),\
                                          (8, 0, None, (.5,1)),\
                                          (8, 0, None, (.5,0)),\
                                          (8, 0, None, (1,.5))],\
                     soft_mask_fn.TEST_MATTE :  [None,\
                                        (8, 0, nr_channels*(0,), None),\
                                        (8, 0, nr_channels*(.25,), None),\
                                        (8, 0, nr_channels*(.5,), None),\
                                        (8, 0, nr_channels*(.75,), None),\
                                        (8, 0, nr_channels*(1,), None)]\
                     }[test_type]

    def __call__(self, spec):
        val = self.val.pop()
        if val:
            spec.image_mask(g_softmask.id(*val))
            matte_str = val[2] and str(val[2][0]) or "-"
            return "bits: %d, i: %s, m:%s, d:%s" % (val[0], val[1] and "yes" or "no", matte_str, str(val[3]))
        return "No mask."



LineX_d            = imagemanip.image(imagemanip.LineX, *g_img_dim)
LineY_d            = imagemanip.image(imagemanip.LineY, *g_img_dim)
Rhomboid_d         = imagemanip.image(imagemanip.Rhomboid, *g_img_dim)
InvertedCross_d    = imagemanip.image(imagemanip.InvertedCross, *g_img_dim)
Checkboard_d       = tuple([(x%2 in [0] and y%2 in [0]) and 1 or 0 for x,y in imagemanip.grid_coords(*g_img_dim)])


def do_grayscale(doc):
    do_page_for_cs(doc, Checkboard_d, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE", default_cfg())

def do_rgb(doc):
    channels = [LineX_d, LineY_d, Rhomboid_d]
    generic_image = imagemanip.interleave_channels(*channels)
    do_page_for_cs(doc, generic_image, jagpdf.CS_DEVICE_RGB, 3, "RGB", default_cfg())


def do_cmyk(doc):
    channels = [LineX_d, LineY_d, Rhomboid_d, InvertedCross_d]
    generic_image = imagemanip.interleave_channels(*channels)
    do_page_for_cs(doc, generic_image, jagpdf.CS_DEVICE_CMYK, 4, "CMYK", default_cfg())



def do_page_for_cs(doc, img, cs, nr_channels, desc, cfg):
    for test, test_desc in [(soft_mask_fn.TEST_INTERPOLATE, 'interpolation'),\
                       (soft_mask_fn.TEST_BIT_DEPTH, 'bit depth'),\
                       (soft_mask_fn.TEST_DECODE, 'decode'),\
                       (soft_mask_fn.TEST_MATTE, 'matte')]:
        cfg.add(bits_a = 6*[8], spec_fn = soft_mask_fn(test, nr_channels))
        do_generic(doc, img, cs, nr_channels, desc + " - " + test_desc,  cfg)
    #read the mask from a file
    cfg.add(bits_a = 6*[8], spec_fn = soft_mask_fn(soft_mask_fn.TEST_BIT_DEPTH, nr_channels), from_file=True)
    do_generic(doc, img, cs, nr_channels, desc + " - " + 'bit depth (from a file)',  cfg)




def output_files():
    return

def do_main(argv):
    out_files = ["customsmask15.pdf",
                  "customsmask14.pdf",
                  "customsmask13.pdf"]
    # (2,3,1) - removed
    for index, version, strict in [(1,4,1), (0,5,1), (2,3,0)]:
        if version < 4 and strict:
            checker = testlib.must_throw
            cfg = testlib.test_config()
            cfg.set("doc.version", str(version))
            cfg.set("doc.strict_mode", str(strict))
            cfg.set("images.softmask_16_to_8", str(0))
            doc = jagpdf.create_stream(testlib.NoopStreamOut(), cfg)
        else:
            checker = lambda fn, *args: fn(*args)
        doc, cfg = testlib.get_legacy_doc(argv,
                                           out_files[index],
                                           { 'doc.version' : version,
                                             'doc.strict_mode' : strict,
                                             'images.softmask_16_to_8' : 0 })
        g_font.set_writer(doc)
        g_softmask.set_doc(doc)
        checker(do_grayscale, doc)
        checker(do_rgb, doc)
        checker(do_cmyk, doc)
        doc.finalize()

def test_main(argv=None):
    try:
        do_main(argv)
#        print g_img_cache.stats()
    except:
        g_temp_files.release()
        raise


if __name__ == '__main__':
    test_main()
#    testlib.profile_test(test_main)




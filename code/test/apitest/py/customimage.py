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
import md5

#mask interpolation - it seems the it does not have any effect

class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

    def add(self, **kwds):
        self.__dict__.update(kwds)
        return self


g_font = testlib.EasyFont()
g_img_dim = 72, 72
g_alt_img_dim = 67, 59
g_temp_files = testlib.TemporaryFiles()

def prepare_page(doc, dim):
    doc.page_start(*dim)
    page = doc.page().canvas()
    testlib.do_background(page, (0.6, 0.3, 0.2), dim, 5)
    page.color_space("fs", jagpdf.CS_DEVICE_RGB)
    page.color("fs", 1.0, 1.0, 1.0)
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
    id_ = doc.image_load(spec)
    testlib.must_throw(doc.image_load, spec) # cannot load the same spec twice
    return id_, desc


def do_generic(doc, image, cs, nr_channels, title, p = default_cfg()):
    page_dim = 4.8*72, 3.8*72
    page, grid, placer = prepare_page(doc, page_dim)
    for bpc in p.bits_a:
        img, desc = get_img_id(doc, image, bpc, nr_channels, cs, p)
        placer(img, desc, *grid.next())

    page.text_font(g_font(14))
    page.text(20, page_dim[1] - 30, title)
    doc.page_end()

###########################################################################
LineX_d            = imagemanip.image(imagemanip.LineX, *g_img_dim)
LineY_d            = imagemanip.image(imagemanip.LineY, *g_img_dim)
InvertedEllipseC_d = imagemanip.image(imagemanip.InvertedEllipseC, *g_img_dim)
Rhomboid_d         = imagemanip.image(imagemanip.Rhomboid, *g_img_dim)
Cross_d            = imagemanip.image(imagemanip.Cross, *g_img_dim)
Checkboard_d       = [(x%4 in [0,1] and y%4 in [0,1]) and 1 or 0 for x,y in imagemanip.grid_coords(*g_img_dim)]

LineX_d_alt            = imagemanip.image(imagemanip.LineX, *g_alt_img_dim)
LineY_d_alt            = imagemanip.image(imagemanip.LineY, *g_alt_img_dim)
InvertedEllipseC_d_alt = imagemanip.image(imagemanip.InvertedEllipseC, *g_alt_img_dim)
Rhomboid_d_alt         = imagemanip.image(imagemanip.Rhomboid, *g_alt_img_dim)
Cross_d_alt            = imagemanip.image(imagemanip.Cross, *g_alt_img_dim)


#odd mask goes through file, even through memory
class HardMask:
    def set_doc(self, doc):
        self.dim = 64, 64
        self.doc = doc
        self.registry = {}
        generic = imagemanip.image(imagemanip.InvertedEllipseC, *self.dim)
        self.mask_data = imagemanip.pack_bits(generic, 1, 1, *self.dim)
        self.req_nr = 0
        self.test_errors()

    def test_errors(self):
        spec = self.doc.define_image_mask()
        testlib.must_throw(self.doc.register_image_mask, spec)
        spec.dimensions(*self.dim)
        testlib.must_throw(self.doc.register_image_mask, spec)

    def id(self, interpolate, reverse):
        key = (interpolate, reverse)
        if  key in self.registry:
            return self.registry[key]
        else:
            self.req_nr += 1
            spec = self.doc.define_image_mask()
            spec.dimensions(*self.dim)
            spec.interpolate(interpolate)
            spec.bit_depth(1)
            if self.req_nr%2:
                handle, tmp_file = tempfile.mkstemp()
                self.mask_data.tofile(open(tmp_file,'wb'))
                os.close(handle)
                g_temp_files.add(tmp_file)
                spec.file_name(tmp_file)
            else:
                spec.data(self.mask_data)
            if reverse:
                pass
                #spec.reverse()
            id = self.doc.register_image_mask(spec)
            self.registry[key] = id
            return id

g_hardmask = HardMask()


###########################################################################




class hard_mask_fn:
    def __init__(self):
        self.val = [(0,1), (1,1), (0,0), (1,0)]

    def __call__(self, spec):
        val = self.val.pop()
        spec.image_mask(g_hardmask.id(*val))
        return "interpolate .. %s, reverse %s" % (val[0] and "yes" or "no", val[1] and "yes" or "no")

class gamma_fn:
    def __init__(self):
        self.val = [1.0, 1.4, 1.8, 2.2, 2.6, 3.0]

    def __call__(self, spec):
        val = self.val.pop()
        spec.gamma(val)
        return 'gamma ' + str(val)

class decode_fn:
    def __init__(self, channels):
        self.val = [(0, 1), (1, 0), (0, 0.5), (0.5, 1), (0.25, 0.75), (0.4, 0.6)]
        self.channels = channels

    def __call__(self, spec):
        val = self.val.pop()
        spec.decode(self.channels*val)
        return 'decode ' + ("[%2f, %2f]" % val)

class alternate_fn:
    def __init__(self, doc, img_id):
        self.val = [img_id, None]
        self.supported = doc.version() >= 3

    def __call__(self, spec):
        val = self.val.pop()
        if val:
            if self.supported:
                spec.alternate_for_printing(val)
            return 'alternated'
        else:
            return 'not alternated'


class rendering_intent_fn:
    def __init__(self):
        self.val = [None,\
                     "RI_ABSOLUTE_COLORIMETRIC",\
                     "RI_RELATIVE_COLORIMETRIC",\
                     "RI_SATURATION",\
                     "RI_PERCEPTUAL"]

    def __call__(self, spec):
        val = self.val.pop()
        if val:
            spec.rendering_intent(getattr(jagpdf, val))
            return val[3:]
        else:
            return 'default'



class interpolate_fn:
    def __init__(self):
        self.val = [0, 1]

    def __call__(self, spec):
        val = self.val.pop()
        spec.interpolate(val)
        return val and "interpolated" or "not interpolated"


class color_key_mask8_fn:
    def __init__(self, channels):
        self.val = [None, (0,127), (127,255), (64,192), (96,160), (32,224)]
        self.ch = channels

    def __call__(self, spec):
        val = self.val.pop()
        if val:
            spec.color_key_mask(self.ch*val)
            return "<%.2f, %.2f>" % (val[0]/255.0, val[1]/255.0)
        else:
            return "not-masked"



#    return Bunch(img_dim=g_img_dim, spec_fn=None, bits_a = [16, 8, 4, 2, 1])

def do_grayscale(doc):
    for idata in ["LineX", "Cross", "InvertedEllipseC", "Rhomboid"]:
        do_generic(doc, globals()[idata+'_d'], jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - " + idata)

    cfg = default_cfg().add(from_file=True)
    do_generic(doc, LineX_d, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - LineX (from file)", cfg)

    cfg = default_cfg().add(img_dim = g_alt_img_dim)
    do_generic(doc, LineY_d_alt, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - %dx%d" % g_alt_img_dim, cfg)

    cfg = default_cfg().add(bits_a = 6*[16], spec_fn = gamma_fn())
    do_generic(doc, LineY_d, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - gamma",  cfg)

    cfg = default_cfg().add(dpi=(144,144))
    do_generic(doc, LineY_d, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - 144 dpi",  cfg)

    cfg = default_cfg().add(bits_a = 6*[16], spec_fn = decode_fn(1))
    do_generic(doc, Cross_d, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - decode",  cfg)

    alt_img_id, desc = get_img_id(doc, Cross_d, 16, 1, jagpdf.CS_DEVICE_GRAY)
    cfg = default_cfg().add(bits_a = 2*[16], spec_fn = alternate_fn(doc, alt_img_id))
    do_generic(doc, LineX_d, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - alternate for print",  cfg)

    Cross_d_low = imagemanip.image(imagemanip.Cross, 18, 18)
    cfg = default_cfg().add(bits_a = 2*[16], dpi=(18,18), spec_fn = interpolate_fn(), img_dim = (18,18))
    do_generic(doc, Cross_d_low, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - interpolate",  cfg)

    cfg = default_cfg().add(bits_a = 6*[8], spec_fn = color_key_mask8_fn(1))
    do_generic(doc, LineX_d, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - color key mask",  cfg)

    cfg = default_cfg().add(bits_a = 4*[8], spec_fn = hard_mask_fn())
    do_generic(doc, Cross_d, jagpdf.CS_DEVICE_GRAY, 1, "GRAYSCALE - hard mask",  cfg)


def do_rgb(doc):
    channels = [LineX_d, LineY_d, Rhomboid_d]
    generic_image = imagemanip.interleave_channels(*channels)
    do_generic(doc, generic_image, jagpdf.CS_DEVICE_RGB, 3, "RGB")

    cfg = default_cfg().add(bits_a = 5*[16], spec_fn = rendering_intent_fn())
    channels = [LineX_d, LineY_d, Rhomboid_d]
    generic_image = imagemanip.interleave_channels(*channels)
    do_generic(doc, generic_image, jagpdf.CS_DEVICE_RGB, 3, "RGB - rendering intent", cfg)

    cfg = default_cfg().add(img_dim = g_alt_img_dim)
    channels = [LineX_d_alt, LineY_d_alt, Rhomboid_d_alt]
    generic_image2 = imagemanip.interleave_channels(*channels)
    do_generic(doc, generic_image2, jagpdf.CS_DEVICE_RGB, 3, "RGB - %dx%d" % g_alt_img_dim, cfg)

    cfg = default_cfg().add(bits_a = 6*[8], spec_fn = color_key_mask8_fn(3))
    do_generic(doc, generic_image, jagpdf.CS_DEVICE_RGB, 3, "RGB - color key mask", cfg)

    cfg = default_cfg().add(bits_a = 4*[8], spec_fn = hard_mask_fn())
    do_generic(doc, generic_image, jagpdf.CS_DEVICE_RGB, 3, "RGB - hard mask",  cfg)


def do_cmyk(doc):
    channels = [LineX_d, LineY_d, Rhomboid_d, InvertedEllipseC_d]
    generic_image = imagemanip.interleave_channels(*channels)
    do_generic(doc, generic_image, jagpdf.CS_DEVICE_CMYK, 4, "CMYK")

    cfg = default_cfg().add(img_dim = g_alt_img_dim)
    channels = [LineX_d_alt, LineY_d_alt, Rhomboid_d_alt, InvertedEllipseC_d_alt]
    generic_image2 = imagemanip.interleave_channels(*channels)
    do_generic(doc, generic_image2, jagpdf.CS_DEVICE_CMYK, 4, "CMYK - %dx%d" % g_alt_img_dim, cfg)

    cfg = default_cfg().add(bits_a = 6*[8], spec_fn = color_key_mask8_fn(4))
    do_generic(doc, generic_image, jagpdf.CS_DEVICE_CMYK, 4, "CMYK - color key mask", cfg)

    cfg = default_cfg().add(bits_a = 4*[8], spec_fn = hard_mask_fn())
    do_generic(doc, generic_image, jagpdf.CS_DEVICE_CMYK, 4, "CMYK - hard mask",  cfg)


def do_cielab(doc):
    channels = [g_img_dim[0]*g_img_dim[1]*[0.5], LineX_d, LineY_d]
    image = imagemanip.interleave_channels(*channels)
    do_generic(doc, image, doc.color_space_load('cielab; white=0.9505, 1.089'), 3, "CIE Lab")


def do_indexed(doc):
    palette = [str(v) for v in range(256)]
    cfg = default_cfg().add(bits_a = 1*[8])
    do_generic(doc, LineX_d, doc.color_space_load('gray; palette=' + ','.join(palette)), 1, "Palette", cfg)

def check_errors(doc):
    spec = doc.image_definition()
    testlib.must_throw(doc.image_load, spec)
    spec.data([10,20,30,40])
    testlib.must_throw(doc.image_load, spec)
    spec.dimensions(2, 2)
    testlib.must_throw(doc.image_load, spec)
    spec.bits_per_component(8)
    testlib.must_throw(doc.image_load, spec)
    spec.color_space(jagpdf.CS_DEVICE_GRAY)
    spec.format(jagpdf.IMAGE_FORMAT_NATIVE)
    doc.image_load(spec)
    spec1 = doc.image_definition()

    spec1.format(jagpdf.IMAGE_FORMAT_PNG)
    spec1.file_name("this_file_does_not_exist")
    testlib.must_throw(doc.image_load, spec1)


def do_main(argv=None):
    out_files = ["customimage15.pdf",\
                  "customimage14.pdf",\
                  "customimage13.pdf",\
                  "customimage12.pdf"]
    # (0,2,1) - removed
    for index, version, strict in [(3,2,0), (2,3,1), (1,4,1), (0,5,1)]:
        if strict and version == 2:
            # it seems that this test branch is flawed as the
            # exceptions are raised in different places then
            # originally inteded
            checker = testlib.must_throw
            cfg = testlib.test_config()
            cfg.set("doc.version", str(version))
            cfg.set("doc.strict_mode", str(strict))
            doc = jagpdf.create_as_stream(testlib.NoopStreamOut(), cfg)
        else:
            checker = lambda fn, *args: fn(*args)
            doc, cfg = testlib.get_legacy_doc(argv,
                                              out_files[index],
                                              {'doc.version':version,
                                               'doc.strict_mode':strict})
        g_font.set_writer(doc)
        checker(g_hardmask.set_doc, doc)
        checker(do_grayscale, doc)
        checker(do_rgb, doc)
        checker(do_cmyk, doc)
        checker(do_cielab, doc)
        checker(do_indexed, doc)
        check_errors(doc)
        doc.finalize()

def test_main(argv=None):
    try:
        do_main(argv)
        #print g_img_cache.stats()
    except:
        g_temp_files.release()
        raise


if __name__ == '__main__':
    test_main()
#    testlib.profile_test(test_main)




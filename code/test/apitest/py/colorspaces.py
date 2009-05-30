# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import os
import sys
import jag.testlib as testlib


## ===============================================
## GENERAL
## ===============================================

class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

class Bind:
    def __init__(self, fn, *args):
        self.args = args
        self.fn = fn

    def __call__(self, *args):
        return self.fn(*(self.args + args))

g_font = testlib.EasyFont()


def draw_rect_grid(page, spec, it):
    # spec.x0,y0,w,h,nx,ny,step_x, step_y
    for yc in range(spec.num_y):
        for xc in range(spec.num_x):
            x = spec.x0 + xc*spec.w + xc*spec.step_x
            y = spec.y0 + yc*spec.h + yc*spec.step_y
            it(xc,yc)
            page.rectangle(x, y, spec.w, spec.h)
            page.path_paint("fs")



def grid1(page, rng, x, y):
    page.color("fs", float(x)/rng)


def grid3(page, rng, fix, x, y):
    if fix == 0:
        page.color("fs", float(x)/rng, float(y)/rng, 0.0)
    elif fix == 1:
        page.color("fs", float(x)/rng, 0.0, float(y)/rng)
    elif fix == 2:
        page.color("fs", 0.0, float(x)/rng, float(y)/rng)
    else:
        assert(0)

def grid4(page, rng, fix, x, y):
    if fix == 0:
        page.color("fs", float(x)/rng, float(y)/rng, 0.0, 0.0)
    elif fix == 1:
        page.color("fs", float(x)/rng, 0.0, float(y)/rng, 0.0)
    elif fix == 2:
        page.color("fs", 0.0, float(x)/rng, float(y)/rng, 0.0)
    else:
        assert(0)

## ===============================================
## DEVICE SPACES
## ===============================================

def do_device_spaces(writer):
    writer.page_start(5.9*72, 6.2*72)
    page = writer.page().canvas()
    page.text_font(g_font(12))

    page.text(20, 20, "DeviceGray")
    page.state_save()
    page.color_space("fs", jagpdf.CS_DEVICE_GRAY)
    spec = Bunch(x0=20, y0=40, w=13, h=13, num_x=24, num_y=1, step_x=2, step_y=2)
    draw_rect_grid(page, spec, Bind(grid1,page,spec.num_x))
    page.state_restore()

    page.text(20, 70, "DeviceRGB")
    page.state_save()
    page.color_space("fs", jagpdf.CS_DEVICE_RGB)
    spec = Bunch(x0=20, y0=85, w=7, h=7, num_x=14, num_y=14, step_x=2, step_y=2)
    draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 0))
    spec.x0 = 2.1*72
    draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 1))
    spec.x0 = 3.9*72
    draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 2))
    page.state_restore()

    page.text(20, 230, "DeviceCMYK")
    page.state_save()
    page.color_space("fs", jagpdf.CS_DEVICE_CMYK)
    spec = Bunch(x0=20, y0=250, w=7, h=7, num_x=14, num_y=14, step_x=2, step_y=2)
    draw_rect_grid(page, spec, Bind(grid4, page, spec.num_x, 0))
    spec.x0 = 2.1*72
    draw_rect_grid(page, spec, Bind(grid4, page, spec.num_x, 1))
    spec.x0 = 3.9*72
    draw_rect_grid(page, spec, Bind(grid4, page, spec.num_x, 2))
    page.state_restore()

    page.text_font(g_font(14))
    page.text(20, 410, "Device Color Spaces")
    writer.page_end()



## ===============================================
## LAB SPACES
## ===============================================

def def_calrgb(writer, gamma=None, mtx=""):
    calrgb = "calrgb;white=0.9505, 1.0890"
    if gamma:
        calrgb += ";gamma=%.3f, %.3f, %.3f" % (gamma, gamma, gamma)
    if mtx:
        calrgb += ";matrix=0.4497, 0.2446, 0.0252, 0.3163, 0.6720, 0.1412, 0.1845, 0.0833, 0.9227"
    return writer.color_space_load(calrgb), gamma and gamma or "default", mtx


def lab_grid3(page, rng, L, x, y):
    x0 = ((float(x)/rng)-0.5)*200
    y0 = ((float(y)/rng)-0.5)*200
    page.color("fs", L, x0, y0)


def do_cie_spaces(writer):
    writer.page_start(5.9*72, 11.3*72)
    page = writer.page().canvas()
    page.text_font(g_font(12))

    curr_y = 20

    def def_calgray(gamma=None):
        spec = "calgray; white=0.9505, 1.0890"
        if gamma:
            spec += ';gamma=%.3f' % gamma
        return writer.color_space_load(spec), gamma and gamma or "default"


    for cs_id, gamma in [def_calgray(), def_calgray(2.2)]:
        page.text(20, curr_y, "CalGray - gamma " + str(gamma))
        curr_y += 15
        page.state_save()
        page.color_space("fs", cs_id)
        spec = Bunch(x0=20, y0=curr_y, w=13, h=13, num_x=24, num_y=1, step_x=2, step_y=2)
        draw_rect_grid(page, spec, Bind(grid1,page,spec.num_x))
        page.state_restore()
        curr_y += 35

    page.text(20, curr_y, "CIE Lab")
    page.text(110, curr_y, "L=25")
    page.text(240, curr_y, "L=75")
    page.text(370, curr_y, "L=100")
    curr_y += 15
    cielab = "cielab;white=0.9505, 1.0890"
    cielabid = writer.color_space_load(cielab)
    page.state_save()
    page.color_space("fs", cielabid)
    spec = Bunch(x0=20, y0=curr_y, w=7, h=7, num_x=14, num_y=14, step_x=2, step_y=2)
    draw_rect_grid(page, spec, Bind(lab_grid3, page, spec.num_x, 25))
    spec.x0 = 2.1*72
    draw_rect_grid(page, spec, Bind(lab_grid3, page, spec.num_x, 75))
    spec.x0 = 3.9*72
    draw_rect_grid(page, spec, Bind(lab_grid3, page, spec.num_x, 100))
    page.state_restore()
    curr_y += 145

    for cs_id, gamma, mtx,  in [def_calrgb(writer), def_calrgb(writer,2.2), def_calrgb(writer,1.8, ' - transformed')]:
        page.text(20, curr_y, "CalRGB - gamma " + str(gamma) + str(mtx))
        curr_y += 15
        page.state_save()
        page.color_space("fs", cs_id)
        spec = Bunch(x0=20, y0=curr_y, w=7, h=7, num_x=14, num_y=14, step_x=2, step_y=2)
        draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 0))
        spec.x0 = 2.1*72
        draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 1))
        spec.x0 = 3.9*72
        draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 2))
        page.state_restore()
        curr_y += 140

    curr_y += 20
    page.text_font(g_font(14))
    page.text(20, curr_y, "CIE Color Spaces - white point [0.9505, 0.0, 1.0890]")
    writer.page_end()



## ===============================================
## ICC Based spaces
## ===============================================
g_res_dir = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/icc')
g_icc_files = ['AdobeRGB1998.icc',\
                "sRGB Color Space Profile.icm",\
                'WideGamutRGB.icc',\
                'AppleRGB.icc',\
                'CIERGB.icc',\
                'ColorMatchRGB.icc']
g_iccs = [os.path.join(g_res_dir, icc) for icc in g_icc_files]


def def_iccbased(writer, icc_file):
    spec = "icc; components=3; profile=" + icc_file
    # alternate is ignored
    return writer.color_space_load(spec), os.path.basename(icc_file)

def do_iccbased_spaces(writer):
    writer.page_start(5.9*72, 14*72)
    page = writer.page().canvas()
    page.text_font(g_font(12))

    curr_y = 20
    for cs_id, desc in [def_iccbased(writer,d) for d in g_iccs]:
        page.text(20, curr_y, desc)
        curr_y += 15
        page.state_save()
        page.color_space("fs", cs_id)
        spec = Bunch(x0=20, y0=curr_y, w=7, h=7, num_x=14, num_y=14, step_x=2, step_y=2)
        draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 0))
        spec.x0 = 2.1*72
        draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 1))
        spec.x0 = 3.9*72
        draw_rect_grid(page, spec, Bind(grid3, page, spec.num_x, 2))
        page.state_restore()
        curr_y += 140

    curr_y += 20
    page.text_font(g_font(14))
    page.text(20, curr_y, "ICC Based color spaces")
    writer.page_end()



## ===============================================
## Indexed spaces
## ===============================================

def palette332():
    result = []
    for i in range(256):
        result += int(255 * ((i&7)/7.0)),\
                  int(255 * (((i>>3)&7)/7.0)),\
                  int(255 * (((i>>6)&3)/3.0))
    return result


def palette431():
    result = []
    for i in range(256):
        result += int(255 * ((i&15)/15.0)),\
                  int(255 * (((i>>4)&7)/7.0)),\
                  int(255 * (((i>>7)&1)/1.0))
    return result


def indexed_fn(page, x, y):
    page.color("fs", y*16+x)


def do_indexed_spaces(writer):
    writer.page_start(5.9*72, 7.5*72)
    page = writer.page().canvas()
    page.text_font(g_font(12))
    curr_y = 20
    curr_x = 20
    def def_indexed(csid, pal):
        palette = "by-id; id=%d; palette=%s" % (csid,
                                                ','.join([str(c) for c in pal]))
        return writer.color_space_load(palette)

    calrgb_id = def_calrgb(writer,2.2)[0]
    icc_id = def_iccbased(writer,g_iccs[1])[0] #sRGB
    for cs_id, desc in [(def_indexed(jagpdf.CS_DEVICE_RGB, palette332()), "332 DeviceRGB"),\
                         (def_indexed(jagpdf.CS_DEVICE_RGB, palette431()), "431 DeviceRGB"),\
                         (def_indexed(calrgb_id, palette332()), "332 CallRGB - g2.2"),\
                         (def_indexed(calrgb_id, palette431()), "431 CallRGB - g2.2"),\
                         (def_indexed(icc_id, palette332()), "332 ICC - sRGB"),\
                         (def_indexed(icc_id, palette431()), "431 ICC - sRGB"),\
                        ]:

        page.text(curr_x, curr_y, "Indexed - " + desc)
        page.state_save()
        page.color_space("fs", cs_id)
        spec = Bunch(x0=curr_x, y0=curr_y+15, w=6, h=6, num_x=16, num_y=16, step_x=2, step_y=2)
        draw_rect_grid(page, spec, Bind(indexed_fn, page))
        page.state_restore()
        curr_x += 200
        if curr_x > 350:
            curr_x = 20
            curr_y += 155

    curr_y += 20
    page.text_font(g_font(14))
    page.text(20, curr_y, "Indexed color spaces")
    writer.page_end()


def test_main(argv=None):
    writer = testlib.create_test_doc(argv, 'colorspaces.pdf')
    g_font.set_writer(writer)
    do_iccbased_spaces(writer)
    do_cie_spaces(writer)
    do_device_spaces(writer)
    do_indexed_spaces(writer)
    writer.finalize()

if __name__ == '__main__':
    test_main()

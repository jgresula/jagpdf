# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# comparison of sRGB and appropriate CalRGB
# also test ability to specify color space externally
import jagpdf
import os
import jag.testlib as testlib
g_font = testlib.EasyFont()

def get_image(writer, color_space):
    page = writer.page().canvas()
    img_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/lena_uncalibrated.jpg')
    spec = writer.image_definition()
    spec.format(jagpdf.IMAGE_FORMAT_JPEG)
    spec.color_space(color_space)
    spec.file_name(img_file)
    return writer.image_load(spec);



def calrgb_pars():
    """gama, mtx, white point"""
    g = .45455
    xw, yw = .3127, .329
    xr, yr = .64, .33
    xg, yg = .3, .6
    xb, yb = .15, .06
    R = G = B = 1.0
    z = yw*((xg-xb)*yr - (xr-xb)*yg + (xr-xg)*yb)

    Ya =  yr/R * ((xg-xb)*yw - (xw-xb)*yg + (xw-xg)*yb)/z
    Xa = Ya*xr/yr
    Za = Ya*(((1-xr)/yr)-1)

    Yb = -yg/G * ((xr-xb)*yw - (xw-xb)*yr + (xw-xr)*yb)/z
    Xb = Yb*xg/yg
    Zb = Yb*(((1-xg)/yg)-1)

    Yc =  yb/B * ((xr-xg)*yw - (xw-xg)*yr + (xw-xr)*yg)/z
    Xc = Yc*xb/yb
    Zc = Yc*(((1-xb)/yb)-1)

    Xw = Xa*R + Xb*G + Xc*B
    Yw = Ya*R + Yb*G + Yc*B
    Zw = Za*R + Zb*G + Zc*B
    return 1/g, [Xa,Ya,Za, Xb,Yb,Zb, Xc,Yc,Zc], [Xw, Yw, Zw]


def calrgb_as_srgb(writer):
    "these params do not do what I would expect"
    spec = "calrgb; white=0.9505, 1.089; gamma=2.22222, 2.22222, 2.22222;"+\
           "matrix=0.4124, 0.2126, 0.0193, 0.3576, 0.7152, 0.1192, 0.1805, 0.0722, 0.9505"
    return writer.color_space_load(spec)


def sRGB(writer):
    spec = "icc; components=3; profile=${JAG_TEST_RESOURCES_DIR}/icc/sRGB Color Space Profile.icm"
    spec = os.path.expandvars(spec)
    return writer.color_space_load(spec)


def do(writer):
    ctrl = [(jagpdf.CS_DEVICE_RGB, "device RGB"),\
             (sRGB(writer), "sRGB"),\
             (calrgb_as_srgb(writer), "CalRGB (parametrized as sRGB)")]
    writer.page_start(3*5.6*72+5*4, 5.6*72+32)
    page = writer.page().canvas()
    page.text_font(g_font(12))

    x=6
    y=25
    for cs, txt in ctrl:
        page.image(get_image(writer, cs), x, y)
        page.text(x, y-20, txt)
        x += 5.6*72+4
    writer.page_end()

def test_main(argv=None):
    doc_writer = testlib.create_test_doc(argv, 'img-jpeg-srgb2calrgb.pdf')
    g_font.set_writer(doc_writer)
    do(doc_writer)
    doc_writer.finalize()

if __name__ == '__main__':
    test_main()


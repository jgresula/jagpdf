# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import os
import sys
import jag.testlib as testlib

g_font = testlib.EasyFont()

g_icc_profile_path = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/icc/AdobeRGB1998.icc')



def do_page(writer):
    def color3(page) : page.color("fs", .3, .7, .4)
    def color1i(page) : page.color("fs", 0)
    def register(str_):
        return writer.color_space_load(str_)

    srgb = register("srgb")
    adobergb = register("adobe-rgb")
    devicergb = register("rgb")
    calrgb = register("calrgb; gamma=2.2, 2.2, 2.2; white=0.9505, 1.089; matrix=0.4124, 0.2126, 0.0193, 0.3576, 0.7152, 0.1192, 0.1805, 0.0722, 0.9505")
    indexed_adobergb = register("adobe-rgb; palette=77, 179, 102")
    writer.page_start(6*10+5*72, 20+72+20)
    page = writer.page().canvas()

    x, d, s = 10, 72, 10
    page.text_font(g_font(6))
    page.text(x, 100, "RGB(0.3, 0.7, 0.4)")
    for cs, desc, set_color in [(srgb, "sRGB", color3),\
                                 (calrgb, "CalRGB set as sRGB", color3),\
                                 (adobergb, "Adobe RGB", color3),\
                                 (indexed_adobergb, "indexed Adobe RGB", color1i),\
                                 (devicergb, "device RGB", color3)\
                                ]:
        page.color_space("fs", cs)
        set_color(page)
        page.rectangle(x, 20, d, d)
        page.path_paint("fs")
        page.text(x, 8, desc)
        x += d+s
    writer.page_end()
    # failures
    def bad_spec(str_):
        testlib.must_throw(writer.color_space_load, str_)
    bad_spec("srgb; palette=77.4, 179, 302")
    bad_spec("calrgb; white=0.0, 2.2")
    bad_spec("calrgb; white=1.0, -1.0")
    bad_spec("calrgb; white=1.0, 1.0; black=1.5, 2.2, -0.1")
    bad_spec("calrgb; white=1.0, 1.0; matrix=1.5, 2.2, -0.1, 0.4, 0, 0, 0, 0")
    bad_spec("cielab; white=1.0, 1.0; range=.1, .2, 0.4")
    bad_spec("cielab; white=1.0, 1.0; range=.1, -.2, 0.4, 2")
    bad_spec("icc; components=2; profile=" + g_icc_profile_path)
    bad_spec("icc; components=3; profile=" + g_icc_profile_path + '_make_it_non-existent')

    bad_spec("calgray; white=1.0, 1.0; gamma=0.0")
    bad_spec("calgray; white=1.0, 1.0; gamma=-0.1")
    bad_spec("calrgb; white=1.0, 1.0; gamma=1,1,1,1")
    bad_spec("calrgb; white=1.0, 1.0; gamma=1,0,1")
    bad_spec("calrgb; white=1.0, 1.0; gamma=1,-1.3,1")
    bad_spec("")
    bad_spec("unknown")



def do_failures_v2(writer):
    def bad_spec(str_):
        testlib.must_throw(writer.color_space_load, str_)
    bad_spec("srgb")
    bad_spec("adobe-rgb; palette=77, 179, 102")
    writer.page_start(72, 72)
    page = writer.page().canvas()
    writer.page_end()


def test_main(argv=None):
    cfg = testlib.test_config()

    # normal functionality
    cfg.set('doc.version', "3")
    writer = testlib.create_test_doc(argv,'color-space-str-def.pdf', cfg)
    g_font.set_writer(writer)
    do_page(writer)
    writer.finalize()

    # expected failures
    cfg.set('doc.version', "2")
    writer = jagpdf.create_stream(testlib.NoopStreamOut(), cfg)
    g_font.set_writer(writer)
    do_failures_v2(writer)
    writer.finalize()

if __name__ == '__main__':
    test_main()


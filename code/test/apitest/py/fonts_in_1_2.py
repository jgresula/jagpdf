# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# content:
#  even if the cid is forced then a simple font is used (if an encoding understood by PDF is used)
#  if it is not possible to use a simple font -> exception
import jagpdf
import jag.testlib as testlib
import os
g_font = testlib.EasyFont()


def test_main(argv=None):
    cfg = testlib.test_config()
    cfg.set('doc.version', "2")
    cfg.set('fonts.force_cid', "1")

    writer = testlib.create_test_doc(argv, "fonts_in_1_2.pdf", cfg)
    g_font.set_writer(writer)

    writer.page_start(72, 72)
    page = writer.page().canvas()
    page.text_font(g_font())
    page.text(20, 20, "OK")
    page.text(20, 40, "1.2 - OK")

    fspec = os.path.expandvars('enc=ISO_8859-3:1988; size=10; file=${JAG_TEST_RESOURCES_DIR}/fonts/DejaVuSans.ttf')
    testlib.must_throw(writer.font_load, fspec) #no cid fonts embedding allowed in <1.3

    writer.page_end()
    writer.finalize()

if __name__ == '__main__':
    test_main()


#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import jag.textfmt as textfmt



def test_main(argv=None):
    doc = testlib.create_test_doc(argv, "basictxtfmt.pdf")
    g_fnt_1250 = testlib.EasyFont(doc, 'windows-1250')
    g_fnt_ttf_utf8 = testlib.EasyFontTTF(doc, 'utf-8')
    g_fnt_utf8 = doc.font_load("standard; name=Helvetica; size=12; enc=utf-8")
    txt = testlib.long_text
    utxt = testlib.long_unicode_text
    do_page(doc, utxt, g_fnt_ttf_utf8(12), "ttf pyunicode", align="justify")
    do_page(doc, txt, g_fnt_1250(12), "core cp1250")
    do_page(doc, txt, g_fnt_1250(12), "core cp1250 - justified", align="justify")
    do_page(doc, txt, g_fnt_1250(12), "core cp1250 - right align", align="right")
    do_page(doc, txt, g_fnt_1250(12), "core cp1250 - center align", align="center")
    do_page(doc, txt, g_fnt_1250(12), "core cp1250 - bottomup", direction='bottomup')
    do_page(doc, utxt.encode('windows-1250'), g_fnt_1250(12), "core cp1250 (from unicode)", baseline="top", align="justify")
    do_page(doc, utxt, g_fnt_utf8, "core unicode", baseline="top", align="justify")
    
    doc.finalize()


def do_page(doc, text, fnt, desc='note', **kwds):
    paper = testlib.paperA4
    doc.page_start(*paper)
    canvas = doc.page().canvas()
    # canvas.transform(1, 0, 0, -1, 0, 0)
    # canvas.transform(1, 0, 0, 1, 0, -(paper[1]/1))
    canvas.state_save()
    canvas.color_space("s", jagpdf.CS_DEVICE_GRAY)
    canvas.color("s", 0.8)
    rect = (36, 36, paper[0]-72, paper[1]-72)
    canvas.rectangle(*rect)
#     canvas.move_to(rect[0]-12, rect[0]+rect[3]-12)
#     canvas.line_to(rect[0]-12, rect[0]+rect[3]-12-72)
    canvas.path_paint("s")
    canvas.state_restore()
    textfmt.format_text(rect, text, doc, fnt, **kwds)
    doc.outline().item(desc)
    doc.page_end()


if __name__ == '__main__':
    test_main()


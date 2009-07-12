#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib

# AY and YA seems to have the largest kerning

def test_main(argv=None):
    profile = testlib.test_config()
    profile.set("text.kerning", "1")
    doc = testlib.create_test_doc(argv, 'kerning2.pdf', profile)
    font = testlib.EasyFontTTF(doc)
    font_core = testlib.EasyFont(doc)
    pheight = 70
    doc.page_start(7*72, pheight)
    canvas = doc.page().canvas()


    txt = "AYAYAYAYAYAYAYA YAYAYAYAYAYAYAYAYA YAYAYAYAYAYAYAY AYAYAYAYAY"
    
    canvas.text_font(font(12))
    canvas.text(10, 20, txt)

    canvas.text_font(font_core(12))
    canvas.text(10, 35, txt)

#     # fetch widths
#     finfo = font
#     widths = [finfo.advance(c) for c in txt]
#     txt_width = sum(widths)
# 
#     # draw bounding lines
#     canvas.state_save()
#     canvas.color('s', 0.8)
#     canvas.move_to(12, 0)
#     canvas.line_to(12, pheight)
#     canvas.move_to(12+txt_width, 0)
#     canvas.line_to(12+txt_width, pheight)
#     canvas.path_paint('s')
#     canvas.state_restore()
# 
#     # write whole string - single text object
#     canvas.text_start(12, 12)
#     canvas.text(txt)
#     canvas.text_end()
# 
#     # write single characters - single text object
#     canvas.text_start(12, 24)
#     for c in txt:
#         canvas.text(c)
#     canvas.text_end()
# 
#     # write single characters - text object for each
#     x = 12
#     for i,c in enumerate(txt):
#         canvas.text_start(x, 36)
#         canvas.text(c)
#         canvas.text_end()
#         x += widths[i]

    doc.page_end()
    doc.finalize()


if __name__ == '__main__':
    test_main()


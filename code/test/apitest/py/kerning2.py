#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jagpdf
import jag.testlib as testlib
import jag.textfmt as textfmt


# AY and YA seems to have the largest kerning

def out_text(canvas, font, x, y, txt, boxed=True):
    canvas.text_font(font)
    canvas.text(x, y, txt)
    if boxed:
        canvas.rectangle(x, y-5, font.advance(txt), 20)
        canvas.path_paint('s')

def basic(doc, font, font_core):
    pheight = 100
    doc.page_start(9*72, pheight)
    canvas = doc.page().canvas()
    txt = "AYAYAYAYAYAYAYA Yarmil YAYAYAYAYAYAYAY AYAYAYAYAYA"
    out_text(canvas, font(12), 10, 20, txt)
    out_text(canvas, font_core(12), 10, 45, txt)
    out_text(canvas, font_core(12), 10, 70, "Kerning.", False)
    doc.page_end()

def format(doc, font, align, boxed=True):
    media = 597.6, 848.68
    margin = 20
    rect = margin, margin, media[0]-2*margin, media[1]-2*margin
    doc.page_start(*media)
    canvas = doc.page().canvas()
    if boxed:
        canvas.state_save()
        canvas.color('s', 0.5)
        canvas.rectangle(*rect)
        canvas.path_paint('s')
        canvas.state_restore()
    text_dir = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/text/')
    txt = open(os.path.join(text_dir, 'lipsum.txt')).read()
    #txt = " ".join(100 * ['Quidam'])
    textfmt.format_text(rect, txt, doc, font, align=align, para_spacing=0.5)
    doc.page_end()

def merge(doc, font):
    pheight = 100
    doc.page_start(9*72, pheight)
    canvas = doc.page().canvas()
    txt = "mmmmmAY"
    canvas.text(10, 20, txt, [-200], [1])
    txt = "AYmmmmm"
    canvas.text(10, 45, txt, [-200], [6])
    doc.page_end()
    
    # TBD: glyph adj on beginning + check kerning na konci and vice versa    

def do_doc(argv, docname, profile=None):
    doc = testlib.create_test_doc(argv, docname, profile)
    font = testlib.EasyFontTTF(doc)
    font_core = testlib.EasyFont(doc)
    basic(doc, font, font_core)
    merge(doc, font_core(12))
    merge(doc, font(12))
    format(doc, font_core(12), 'justify')
    format(doc, font_core(12), 'left')
    format(doc, font(12), 'justify')
    format(doc, font(12), 'left')
    doc.finalize()



def test_main(argv=None):
    profile = testlib.test_config()
    profile.set("text.kerning", "1")
    do_doc(argv, 'kerning2.pdf', profile)
    #do_doc(argv, 'kerning2_no.pdf')

if __name__ == '__main__':
    test_main()


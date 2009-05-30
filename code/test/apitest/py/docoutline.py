#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import jagpdf
import sys
import os
import jag.testlib as testlib

#raw_input('attach')

g_font = testlib.EasyFontTTF()
g_page_height = 7*72

ITALIC = 1
BOLD   = 2

def do_page(doc, title, x=None, y=None, style=None, rgb=None, text=None):
    global g_page_height
    outline = doc.outline()
    if None==text:
        text=title
    doc.page_start(5.9*72, g_page_height)
    if None!=style:
        if doc.version() >= 4:
            outline.style(style)
    if None!=rgb:
        if doc.version() >= 4:
            outline.color(*rgb)
    if None==x:
        outline.item(title)
    else:
        outline.item(title, 'mode=XYZ; left=%f; top=%f' % (x, y))
    page = doc.page().canvas()
    page.text_font(g_font())
    page.text(20, g_page_height/2, text)
    doc.page_end()
    g_page_height -= 36



def standard_usage(doc):
    outline = doc.outline()
    do_page(doc, 'P\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88', text="utf16-be bookmark")
    outline.state_save()
    do_page(doc, '2nd page - gray', rgb=(0.5,0.5,0.5))
    outline.state_restore()
    do_page(doc, '3rd page - default style')
    outline.level_down()
    outline.state_save()
    do_page(doc, '3a page - bold red', style=BOLD, rgb=(1.0,0.0,0.0))
    outline.state_save()
    do_page(doc, '3b page - 1/2h bold italic red', 0, g_page_height/2, style=BOLD|ITALIC)
    outline.state_restore()
    outline.level_down()
    do_page(doc, '3b1 page - bold red')
    outline.level_up()
    outline.state_restore()
    do_page(doc, '3c page - default style')
    # the last level_up() is not called intentionally as the
    # implementation is supposed to take care about that


def fault_injection():
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    outline = doc.outline()
    testlib.must_throw(outline.item, "Invalid bookmark")
    outline.state_save()
    doc.page_start(5.9*72, g_page_height)
    testlib.must_throw(outline.level_down)
    testlib.must_throw(outline.level_up)
    doc.page_end()
    outline.state_restore()
    testlib.must_throw(outline.state_restore)
    doc.finalize()


def do_document(argv, cfg, name):
    doc = testlib.create_test_doc(argv, name, cfg)
    g_font.set_writer(doc)
    standard_usage(doc)
    doc.finalize()


def do_invalid_destinations(argv,cfg,name):
    invalid_dests = [\
        "zoom=1.2",
        "mode=nonsense",
        "mode=XYZ;zoom=onan",
        "mode=FitR;left=1;top=1;bottom=1"
       ]
    for d in invalid_dests:
        doc = testlib.create_test_doc(argv, name, cfg)
        doc.page_start(10.0*72, 10.0*72)
        doc.outline().item("~", d)
        doc.page_end()
        testlib.must_throw(doc.finalize)
        doc = None

    syntax_err_dests = ["oom=1.2"]
    doc = testlib.create_test_doc(argv, name, cfg)
    doc.page_start(10.0*72, 10.0*72)
    for d in syntax_err_dests:
        testlib.must_throw(doc.outline().item, "~", d)
    doc.page_end()
    doc.finalize()



def do_generic_bookmarks(argv, cfg, name):
    rl = 72
    rt = 9*72
    rr = 72+144
    doc = testlib.create_test_doc(argv, name, cfg)
    outline = doc.outline()
    doc.page_start(10.0*72, 10.0*72)
    page = doc.page().canvas()
    page.rectangle(rl, 7*72, 144, 144)
    page.path_paint('s')
    page.rectangle(72+36, 7*72+36, 72, 72)
    page.path_paint('s')

    outline.item("Zoom 100%", "mode=XYZ;zoom=1.0")
    outline.item("Zoom 250%", "mode=XYZ;zoom=2.5")
    outline.item("Zoom 25%", "mode=XYZ;zoom=.25")



    outline.item("Rect top-left, retain zoom", "mode=XYZ;left=%lf;top=%lf" % (rl,rt) )



    outline.item("Fit width, position rectangle top", "mode=FitH;top=%lf" % rt)
    outline.item("Fit width, retain y", "mode=FitH")




    outline.item("Fit height, position rectangle right", "mode=FitV;left=%lf" % rr)
    outline.item("Fit height, retain x", "mode=FitV")
    outline.item("Fit inner rectangle",
                     "mode=FitR;left=%lf;top=%lf;bottom=%lf;right=%lf" %\
                     (72+36, 7*72+36+72, 7*72+36, 72+36+72 ))
    outline.item("Fit page", "mode=Fit")
    outline.item("Fit page bbox", "mode=FitB")
    outline.item("Fit bbox width, retain y", "mode=FitBH")



    outline.item("Fit bbox width, top 1/2 rect", "mode=FitBH;top=%lf" % (rt-72))
    outline.item("Fit bbox height, retain x", "mode=FitBV")



    outline.item("Fit bbox height, left 1/2 rect", "mode=FitBV;left=%lf" % (rl+72))
    outline.item("", "mode=XYZ;zoom=1.5")
    outline.item("^^ an empty bookmark that zooms to  150%", "mode=XYZ;zoom=1.0")
    doc.page_end()
    doc.finalize()


def test_main(argv=None):
    cfg = testlib.test_config()
#    cfg.set("doc.trace_level", "5")
#    cfg.set("doc.trace_show_loc", "0")

    do_invalid_destinations(argv, cfg, 'docoutline_invalid_dest.pdf')
    do_generic_bookmarks(argv, cfg, 'docoutline_generic.pdf')
    do_document(argv, cfg, 'docoutline.pdf')

    cfg.set("doc.version", "3")
    do_document(argv, cfg, 'docoutline13.pdf')

    cfg.set("doc.version", "4")
    cfg.set("doc.encryption", "standard")
    cfg.set("info.static_producer", "1")
    cfg.set("doc.static_file_id", "1")
    cfg.set("info.creation_date", "0")

    do_document(argv, cfg, 'docoutline_enc.pdf')

## fault injection
    fault_injection()


if __name__ == "__main__":
    test_main()


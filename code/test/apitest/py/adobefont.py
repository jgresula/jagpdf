#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import os
import sys
import jag.testlib as testlib

g_page_margin = 30
g_pad = 4
g_ptsize = 12
g_boxd = g_ptsize + g_pad

def page_dim():
    legend = g_ptsize+g_pad
    dim = 16*(g_ptsize+g_pad) + legend + 2*(g_page_margin)
    return dim, dim


def do_legend(doc, title):
    page = doc.page().canvas()
    pdim = page_dim()[0]
    fid = doc.font_load("standard;name=Helvetica;size=%d" % g_ptsize)
    page.text_font(fid)
    page.text(10, pdim-10-g_boxd, title)
    page.state_save()
    page.color('fs', .5)
    y = g_page_margin
    for c in "FECDBA9876543210":
        page.text(g_page_margin, y, c+'x')
        y += g_boxd
    x = g_page_margin + g_boxd
    for c in "0123456789ABCDEF":
        page.text(x, pdim-g_page_margin-g_boxd, 'x'+c)
        x += g_boxd
    page.state_restore()


def place_char(page, code):
    left, top = g_page_margin+g_boxd, page_dim()[0]-g_page_margin-2*g_boxd
    x = code % 16
    y = code / 16
    page.text(left+x*g_boxd, top-y*g_boxd, chr(code))


fonts = [\
    "Courier-Bold",\
    "Courier-BoldOblique",\
    "Courier-Oblique",\
    "Courier",\
    "Helvetica-Bold",\
    "Helvetica-BoldOblique",\
    "Helvetica-Oblique",\
    "Helvetica",\
    "Times-Bold",\
    "Times-BoldItalic",\
    "Times-Italic",\
    "Times-Roman",\
    "Symbol",\
    "ZapfDingbats",\
   ]


def page_with_encoding(doc, enc, font):
    doc.page_start(*page_dim())
    page = doc.page().canvas()
    fspecstr = "standard;name=%s;size=%d" % (font, g_ptsize)
    if enc:
        fspecstr = fspecstr + ";enc=" + enc
        do_legend(doc, enc)
    else:
        do_legend(doc, 'built-in')
    fid = doc.font_load(fspecstr)
    page.text_font(fid)
    for i in range(256):
        place_char(page, i)
    doc.page_end()



def pages_with_encoding(doc):
    page_with_encoding(doc, 'windows-1252', 'Helvetica')
    page_with_encoding(doc, 'macintosh', 'Helvetica')
    page_with_encoding(doc, '', 'Helvetica')
    page_with_encoding(doc, '', 'Symbol')
    page_with_encoding(doc, '', 'ZapfDingbats')


def other_8bit_encodings(doc):
    page_with_encoding(doc, 'windows-1250', 'Helvetica')
    page_with_encoding(doc, "windows-1251", 'Helvetica')
    page_with_encoding(doc, "windows-1253", 'Helvetica')
    page_with_encoding(doc, "windows-1254", 'Helvetica')
    page_with_encoding(doc, "windows-1255", 'Helvetica')
    page_with_encoding(doc, "windows-1256", 'Helvetica')
    page_with_encoding(doc, "windows-1257", 'Helvetica')
    page_with_encoding(doc, "windows-1258", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-1:1987", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-2:1987", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-3:1988", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-4:1988", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-5:1988", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-6:1987", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-7:1987", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-8:1988", 'Helvetica')
    page_with_encoding(doc, "ISO_8859-9:1989", 'Helvetica')
    page_with_encoding(doc, "ISO-8859-10", 'Helvetica')
    page_with_encoding(doc, "ISO-8859-11", 'Helvetica')
    page_with_encoding(doc, "ISO-8859-13", 'Helvetica')
    page_with_encoding(doc, "ISO-8859-14", 'Helvetica')
    page_with_encoding(doc, "ISO-8859-15", 'Helvetica')
#    page_with_encoding(doc, "ISO-8859-16", 'Helvetica')



def basic_page(doc):
    specstr = "standard;name=%s;size=%d"
    specs12 = [(f,doc.font_load(specstr % (f,12))) for f in fonts]
    specs8 = [(f,doc.font_load(specstr % (f,8))) for f in fonts]

    doc.page_start(5.9*72, 5.9*72)
    page = doc.page().canvas()
    y = 10
    ystep = 14
    for name, fid in specs8 + specs12:
        page.text_font(fid)
        page.text(20, y, "written in %s" % name)
        y += ystep
    doc.page_end()


def page_with_kun(doc):
    doc.page_start(*page_dim())
    fid = doc.font_load("standard;name=Helvetica;size=8;enc=windows-1250")
    page = doc.page().canvas()
    page.text_font(fid)
    utf8 = 'P\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88 \xc3\xbap\xc4\x9bl \xc4\x8f\xc3\xa1belsk\xc3\xa9 \xc3\xb3dy.'
    s = unicode(utf8, 'utf8').encode('cp1250')
    page.text(20, 20, s)
    doc.page_end()


def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'adobefonts.pdf')
    basic_page(doc)
    pages_with_encoding(doc)
    page_with_kun(doc)
    other_8bit_encodings(doc)
    doc.finalize()


if __name__ == "__main__":
    test_main()



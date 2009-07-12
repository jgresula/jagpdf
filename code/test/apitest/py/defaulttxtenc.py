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

paper = testlib.paperA5
rect = (36, 36, paper[0]-72, paper[1]-72)

def do_page(doc, fnt, txt, label):
    doc.page_start(*paper)
    canvas = doc.page().canvas()
    canvas.rectangle(*rect)
    canvas.path_paint("s")
    textfmt.format_text(rect, txt, doc, fnt(10), align='justify')
    doc.outline().item(label)
    doc.page_end()

def test_main(argv=None):
    ##
    ## default text encoding: utf-8
    ## font encoding:         windows-1250
    ##
    cfg = testlib.test_config()
    cfg.set("text.encoding", "utf-8")
    doc = testlib.create_test_doc(argv, "defaulttxtenc.pdf", cfg)
    #doc = testlib.create_test_doc("/mnt/win/c/Temp/basictxtfmt3.pdf", cfg)

    do_page(
        doc,
        testlib.EasyFont(doc, 'windows-1250'),
        testlib.long_unicode_text,
        "core font 1250, text utf-8")

    do_page(
        doc,
        testlib.EasyFontTTF(doc, 'windows-1250'),
        testlib.long_unicode_text,
        "ttf 1250, text utf-8")

    doc.finalize()


    ##
    ## default text encoding: windows-1250
    ## font encoding: iso-8859-2
    ##
    cfg  = testlib.test_config()
    cfg.set("text.encoding", "windows-1250")
    doc = testlib.create_test_doc(argv, "defaulttxtenc2.pdf", cfg)
    #doc = testlib.create_test_doc("/mnt/win/c/Temp/defaulttxtenc2.pdf", cfg)

    do_page(
        doc,
        testlib.EasyFont(doc, 'iso-8859-2'),
        testlib.long_unicode_text.encode("windows-1250"),
        "core font iso-8859-2, windows-1250")

    do_page(
        doc,
        testlib.EasyFontTTF(doc, 'iso-8859-2'),
        testlib.long_unicode_text.encode("windows-1250"),
        "ttf iso-8859-2, windows-1250")

    doc.finalize()


if __name__ == '__main__':
    test_main()
#     import sys
#     sys.exit(1)

#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

import jag.testlib as testlib
import jagpdf

def check_font_metrics(doc):
    f1 = doc.font_load("standard; name=Helvetica; size=12; enc=utf-8")
    f2 = doc.font_load("standard; name=Helvetica; size=12; enc=iso-8859-2")
    assert f1.is_bold() == f2.is_bold()
    assert f1.is_italic() == f2.is_italic()
    assert f1.size() == f2.size()
    assert f1.family_name() == f2.family_name()
    assert f1.height() == f2.height()
    assert f1.ascender() == f2.ascender()
    assert f1.descender() == f2.descender()
    assert f1.bbox_xmin() == f2.bbox_xmin()
    assert f1.bbox_ymin() == f2.bbox_ymin()
    assert f1.bbox_xmax() == f2.bbox_xmax()
    assert f1.bbox_ymax() == f2.bbox_ymax()


def basic_test(argv):
    lines = ["Jag kan äta glas utan att skada mig.",
             "Rødgrød med mælk og fløde på.",
             "Příliš žluťoučký kůň úpěl ďábelské ódy.",
             "Single encoding."]
    profile = testlib.test_config()
    profile.set("doc.trace_level", "2")
    doc = testlib.create_test_doc(argv, 'adobefont_unicode.pdf', profile)
    fnt = doc.font_load("standard; name=Helvetica; size=12; enc=utf-8")
    doc.page_start(500, 500)
    canvas = doc.page().canvas()
    canvas.text_font(fnt)
    # n texts
    for line, y in zip(lines, [110, 90, 70, 50]):
        canvas.text(50, y, line)
    # single multiline text
    canvas.text_start(50, 150)
    for line in lines:
        canvas.text(line)
        canvas.text_translate_line(0, 15)
    canvas.text_end()
    # all in one line
    fnt_8 = doc.font_load("standard; name=Helvetica; size=8; enc=utf-8")
    canvas.text_font(fnt_8)
    joined = " ".join(lines)
    rect_w = fnt_8.advance(joined)
    # enable the following lines to see BANG
    canvas.state_save()
    canvas.color('s', 0.5)
    canvas.rectangle(10, 250, rect_w, 100)
    canvas.path_paint('s')
    canvas.state_restore()
    canvas.text(10, 300, joined)
    # full justification
    t = unicode(lines[1] + ' ' + lines[2], 'utf-8')
    w = fnt_8.advance(t)
    spaces = [i for i in range(len(t)) if t[i] == u' ']
    nr_spaces = len(spaces)
    padd = -(1000 / fnt_8.size()) * (rect_w - w) / nr_spaces
    positions = [padd for i in range(nr_spaces)]
    canvas.text(10, 320, t, positions, spaces)
    #
    canvas.text(10, 400, "Cyrillic >\xd0\x96\xd1\x86\xd0\xba<")
    #
    check_font_metrics(doc)
    doc.page_end()
    doc.finalize()

def font_reuse(argv):
    czech = u"Příliš žluťoučký kůň úpěl ďábelské ódy."
    danish = u"Rødgrød med mælk og fløde på."
    doc = testlib.create_test_doc(argv, 'adobefont_reusing.pdf')
    fnt_1 = doc.font_load("standard; name=Helvetica; size=12; enc=iso-8859-1")
    fnt_2 = doc.font_load("standard; name=Helvetica; size=12; enc=iso-8859-2")
    fnt_u = doc.font_load("standard; name=Helvetica; size=12; enc=utf-8")
    doc.page_start(500, 500)
    canvas = doc.page().canvas()
    # 8859-2
    canvas.text_font(fnt_2)
    canvas.text(50, 100, czech.encode('iso-8859-2'))
    # 8859-1
    canvas.text_font(fnt_1)
    canvas.text(50, 120, danish.encode('iso-8859-1'))
    # utf-8
    canvas.text_font(fnt_u)
    canvas.text(50, 150, danish)
    canvas.text(50, 170, czech)
    doc.page_end()
    doc.finalize()

def error_injection():
    # reject non-roman alphabet
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    fnt = doc.font_load("standard; name=Helvetica; size=12; enc=utf-8")
    doc.page_start(500, 500)
    canvas = doc.page().canvas()
    canvas.text_font(fnt)
    #testlib.must_throw(canvas.text, 10, 10, "\xd0\x96\xd1\x86\xd0\xba")
    doc.page_end()
    doc.finalize()

def graphics_state(argv):
    # do not send unnecessary set font operators
    profile = testlib.test_config()
    profile.set("doc.compressed", "0")
    doc = testlib.create_test_doc(argv, 'adobefont_grstate.pdf', profile)
    fnt = doc.font_load("standard; name=Helvetica; size=12; enc=utf-8")
    doc.page_start(200, 200)
    canvas = doc.page().canvas()
    canvas.text_font(fnt)
    canvas.text_start(20,50)
    canvas.text('first')
    canvas.text(' second')
    canvas.text_end()
    doc.page_end()
    doc.finalize()

    

def test_main(argv=None):
    basic_test(argv)
    font_reuse(argv)
    error_injection()
    graphics_state(argv)


if __name__ == "__main__":
    test_main()

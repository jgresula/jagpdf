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
    test_faults()
    # 'save state' and 'restore state' operators should not be used within
    # text objects. Regarding the graphics state, only 'General graphics
    # state' operators are allowed.

    doc = testlib.create_test_doc(argv, "textstate.pdf")
    #doc = testlib.create_test_doc("/mnt/win/c/Temp/textstate.pdf")
    fnt = testlib.EasyFont(doc)

    paper = testlib.paperA5
    doc.page_start(*paper)
    canvas = doc.page().canvas()

    fid = fnt(32)
    canvas.text_font(fid)
    linespacing = fid.height()
    canvas.text_start(18, 36)

    def show_text_ex(canvas, text, action, *args):
        action(*args)
        canvas.text(text)
        canvas.text_translate_line(0, linespacing)


    #char spacing
    canvas.text("Character spacing normal")
    canvas.text_translate_line(0, linespacing)
    show_text_ex(canvas, "Character spacing: 1", canvas.text_character_spacing, 1)
    canvas.text_character_spacing(0)

    #horizontal scaling
    canvas.text("Horizontal scaling 100%")
    canvas.text_translate_line(0, linespacing)
    show_text_ex(canvas, "Horizontal scaling 50%", canvas.text_horizontal_scaling, 50)
    show_text_ex(canvas, "Horizontal scaling 110%", canvas.text_horizontal_scaling, 110)
    canvas.text_horizontal_scaling(100)

    # rise
    canvas.text_translate_line(0, linespacing)
    canvas.text("Rise ")
    canvas.text_rise(10)
    canvas.text("10")
    canvas.text_rise(-10)
    canvas.text("-10")

    canvas.text_end()
    doc.page_end()

    # text rendering modes
    doc.page_start(*paper)
    canvas = doc.page().canvas()

    canvas.color_space("s", jagpdf.CS_DEVICE_RGB)
    canvas.color("s", 0.0, 0.0, 1.0)

    canvas.translate(18, 36)
    canvas.text_font(fid)

    def clipped_obj(canvas):
        canvas.state_save()
        canvas.color_space("s", jagpdf.CS_DEVICE_RGB)
        canvas.color("s", 1.0, 0.0, 0.0)
        for i in range(1, int(paper[1]), 3):
            canvas.move_to(0, i)
            canvas.line_to(paper[0], i)
        canvas.path_paint("s")
        canvas.state_restore()

    modes = [("f", "fill", None),
              ("fs", "fill stroke", None),
              ("s", "stroke", None),
              ("i", "invisible", None),
              ("fc", "fill clip", clipped_obj),
              ("fsc", "fill stroke clip", clipped_obj),
              ("c", "clip", clipped_obj),
              ("sc", "stroke clip", clipped_obj)]
    curr_y = 0
    for mode, text, action in modes:
        canvas.state_save()
        canvas.text_start(0, curr_y)
        canvas.text_rendering_mode(mode)
        canvas.text(text)
        canvas.text_end()
        canvas.translate(0, -curr_y)
        if action:
            clipped_obj(canvas)
        canvas.state_restore()
        curr_y += linespacing


    # other tests and fault injection
    testlib.must_throw(canvas.text_rendering_mode, "a")
    testlib.must_throw(canvas.text_rendering_mode, "ic")
    canvas.text_rendering_mode("fFsScC")
    canvas.text_rendering_mode("iI")

    doc.page_end()
    doc.finalize()


def test_faults():
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    doc.page_start(72, 72)
    canvas = doc.page().canvas()
    # cannot use simple text inside a text object
    canvas.text_start(0, 0)
    testlib.must_throw(canvas.text, 10, 10, "No!")
    canvas.text_end()
    canvas.text(10, 10, "Yes")
    doc.page_end()
    doc.finalize()




if __name__ == '__main__':
    test_main()

#!/usr/bin/env python
# $Id$

import jagpdf
import jag.testlib as testlib

pageDim = 500, 650


def do_document(doc, canvas, topdown=False):
    # path
    canvas.move_to(50, 50)
    canvas.line_to(350, 50)
    canvas.line_to(200, 150)
    canvas.path_paint('sc')
    # text
    canvas.move_to(50, 170)
    canvas.line_to(450, 170)
    canvas.path_paint('s')
    fnt = doc.font_load("standard; name=Helvetica; size=40")
    canvas.text_font(fnt)
    if topdown:
        canvas.state_save()
        canvas.scale(1, -1)
    canvas.text(50, 170, 'Never say never!')
    if topdown:
        canvas.state_restore()
    # multiline text
    linespacing = fnt.height()
    canvas.move_to(50, 500)
    canvas.line_to(450, 500)
    canvas.move_to(50, 500 + linespacing)
    canvas.line_to(450, 500 + linespacing)
    canvas.path_paint('s')
    canvas.text_start(50, 500)
    canvas.text("Multiline #1")
    canvas.text_translate_line(0, linespacing)
    canvas.text("Multiline #2")
    canvas.text_end()
    # image
    canvas.move_to(50, 260)
    canvas.line_to(50, 230)    
    canvas.line_to(250, 230)
    canvas.line_to(250, 260)
    canvas.path_paint('s')
    testlib.paint_image('/images/logo.png', doc, 50, 230)
    canvas.state_save()
    canvas.translate(250, 230)
    canvas.scale(0.5, 0.5)
    testlib.paint_image('/images/logo.png', doc, 0, 0)
    canvas.state_restore()
    # shading pattern
    fn = doc.function_2_load("domain=0, 1; c0=0; c1=1")
    sh = doc.shading_pattern_load("axial; coords=300, 300, 400, 400",
                                  jagpdf.CS_DEVICE_GRAY,
                                  fn)
    canvas.state_save()
    canvas.color_space_pattern('f')
    canvas.pattern('f', sh)
    canvas.rectangle(280, 280, 140, 140)
    canvas.path_paint('fs')
    canvas.state_restore()
    canvas.move_to(300, 300)
    canvas.line_to(400, 400)
    canvas.path_paint('s')
    # shading operator
    canvas.state_save()
    sh = doc.shading_pattern_load("axial; coords=10, 10, 90, 90",
                                  jagpdf.CS_DEVICE_GRAY,
                                  fn)
    canvas.translate(100, 300)
    canvas.rectangle(0, 0, 100, 100)
    canvas.path_paint('w')
    canvas.shading_apply(sh)
    canvas.state_restore()
    
    

def do_file(argv, name, profile=None):
    doc = testlib.create_test_doc(argv, name, profile)
    doc.page_start(*pageDim)
    canvas = doc.page().canvas()
    do_document(doc, canvas)
    doc.page_end()
    doc.finalize()
    


def test_main(argv=None):
    do_file(argv, "topdown_off.pdf")
    profile = testlib.test_config()
    profile.set('doc.topdown', "1")
    do_file(argv, "topdown_on.pdf", profile)
    #
    profile = testlib.test_config()
    profile.set('doc.page_layout', 'TwoColumnLeft')
    profile.set('doc.viewer_preferences', 'FitWindow')
    doc = testlib.create_test_doc(argv, "topdown.pdf", profile)
    # classic page
    doc.page_start(*pageDim)
    canvas = doc.page().canvas()
    do_document(doc, canvas)
    doc.page_end()
    # top-down page
    doc.page_start(*pageDim)
    canvas = doc.page().canvas()
    canvas.translate(0, pageDim[1])
    canvas.scale(1, -1)
    do_document(doc, canvas, True)
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()


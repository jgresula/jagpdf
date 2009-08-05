#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#
import jagpdf
import jag.testlib as testlib
import math

pageDim = 500, 650

def create_tiling_pattern_inner(doc):
    fnt = testlib.EasyFont(doc, 'utf-8')
    pc = doc.canvas_create()
    pc.color_space("f", jagpdf.CS_DEVICE_GRAY)
    pc.color("f", 0.0)
    pc.text_font(fnt(8))
    pc.text_start(2, 9)
    pc.text("inner #1")
    pc.text_translate_line(0, 9)
    pc.text("inner #2")
    pc.text_end()
    spec = "step=50, 25"
    return doc.tiling_pattern_load(spec, pc)
    

def create_tiling_pattern2(doc):
    pc = doc.canvas_create()
    pc.color_space("fs", jagpdf.CS_DEVICE_GRAY)
    pc.color("fs", 0.5)
    pc.rectangle(5, 5, 90, 40)
    pc.path_paint("f")
    pc.rectangle(0, 0, 100, 50)
    pc.path_paint("s")
    spec = "step=100, 50"
    tp = create_tiling_pattern_inner(doc)
    pc.color_space_pattern('f')
    pc.pattern('f', tp)
    pc.rectangle(0, 0, 100, 50)
    pc.path_paint("f")
    return doc.tiling_pattern_load(spec, pc)

def create_tiling_pattern(doc, mtx=None):
    pc = doc.canvas_create()
    pc.color_space("fs", jagpdf.CS_DEVICE_GRAY)
    pc.color("fs", 0)
    pc.state_save()
    pc.color("fs", 0.5)
    pc.translate(50, 50)
    pc.rotate(math.pi/4.)
    pc.scale(1, 0.5)
    pc.translate(-50, -50)
    pc.circle(50, 50, 50)
    pc.path_paint("fs")
    pc.state_restore()
    pc.move_to(5, 5)
    pc.line_to(95, 95)
    pc.path_paint("s")
    pc.text_start(5, 10)
    pc.text("text in pattern #1")
    pc.text_translate_line(0, 14)
    pc.text("text in pattern #2")
    pc.text_end()
    testlib.paint_image('/images/logo.png', doc, 5, 40, pc)
    spec = "step=100, 100"
    if mtx:
        mtx_s = ", ".join([str(i) for i in mtx])
        spec = spec + "; matrix=" + mtx_s
    return doc.tiling_pattern_load(spec, pc)

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
    basey = 550
    canvas.move_to(50, basey)
    canvas.line_to(450, basey)
    canvas.path_paint('s')
    canvas.color('s', 0.8)
    canvas.move_to(50, basey + linespacing)
    canvas.line_to(450, basey + linespacing)
    canvas.path_paint('s')
    canvas.color('s', 0.0)
    canvas.text_start(50, basey)
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
    sh = doc.shading_pattern_load("axial; coords=170, 300, 270, 400",
                                  jagpdf.CS_DEVICE_GRAY,
                                  fn)
    canvas.state_save()
    canvas.color_space_pattern('f')
    canvas.pattern('f', sh)
    canvas.rectangle(150, 280, 140, 140)
    canvas.path_paint('fs')
    canvas.state_restore()
    canvas.move_to(170, 300)
    canvas.line_to(270, 400)
    canvas.path_paint('s')
    # scaled shading pattern
    mtx = testlib.Matrix()
    mtx.translate(400, 350)
    mtx.scale(.5, .5)
    mtx.translate(-400, -350)
    mtx_s = ", ".join([str(i) for i in mtx.data()])
    spec_str = "axial; coords=350, 300, 450, 400; matrix=%s" % mtx_s
    sh = doc.shading_pattern_load(spec_str,
                                  jagpdf.CS_DEVICE_GRAY,
                                  fn)
    canvas.state_save()
    canvas.color_space_pattern('f')
    canvas.pattern('f', sh)
    canvas.rectangle(330, 280, 140, 140)
    canvas.path_paint('fs')
    canvas.state_restore()
    canvas.move_to(375, 325)
    canvas.line_to(425, 375)
    canvas.path_paint('s')
    # shading operator
    canvas.state_save()
    sh = doc.shading_pattern_load("axial; coords=10, 10, 90, 90",
                                  jagpdf.CS_DEVICE_GRAY,
                                  fn)
    canvas.translate(25, 300)
    canvas.rectangle(0, 0, 100, 100)
    canvas.path_paint('w')
    canvas.shading_apply(sh)
    canvas.state_restore()
    # tiling pattern
    tp = create_tiling_pattern(doc)
    canvas.color_space_pattern('f')
    canvas.pattern('f', tp)
    canvas.rectangle(0, 400, 100, 100)
    canvas.path_paint("f")
    mtx = testlib.Matrix()
    mtx.translate(50, 50)
    mtx.scale(.5, .5)
    mtx.translate(-50, -50)
    tp_scaled = create_tiling_pattern(doc, mtx.data())
    canvas.pattern('f', tp_scaled)
    canvas.rectangle(125, 425, 100, 100)
    canvas.path_paint("fs")
    # pattern using another pattern
    tp2 = create_tiling_pattern2(doc)
    canvas.pattern('f', tp2)
    canvas.rectangle(300, 450, 100, 50)
    canvas.path_paint("f")
    
    

def do_file(argv, name, profile=None):
    doc = testlib.create_test_doc(argv, name, profile)
    doc.page_start(*pageDim)
    canvas = doc.page().canvas()
    do_document(doc, canvas)
    doc.page_end()
    doc.finalize()
    


def test_main(argv=None):
    #do_file(argv, "topdown_off.pdf")
    profile = testlib.test_config()
    profile.set('doc.topdown', "1")
    do_file(argv, "topdown_on.pdf", profile)
#     #

if __name__ == "__main__":
    test_main()


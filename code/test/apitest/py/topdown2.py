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



def create_shading_pattern(doc):
    fn = doc.function_2_load("domain=0, 1; c0=0; c1=1")
    return doc.shading_pattern_load("axial; coords=100, 100, 400, 400",
                                    jagpdf.CS_DEVICE_GRAY,
                                    fn)

def do_page(doc, dim, tp ,sp):
    doc.page_start(*dim)
    canvas = doc.page().canvas()
    canvas.color_space_pattern('f')
    canvas.pattern('f', sp)
    canvas.rectangle(100, 100, 300, 300)
    canvas.path_paint('fs')
    canvas.pattern('f', tp)
    canvas.rectangle(100, 100, 100, 100)
    canvas.rectangle(100, 300, 100, 100)
    canvas.path_paint('fs')
    doc.page_end()    


def do_document(doc, topdown=False):
    tp = create_tiling_pattern(doc)
    sp = create_shading_pattern(doc)
    do_page(doc, (500, 500), tp, sp)
    do_page(doc, (500, 750), tp, sp)
    
    

def do_file(argv, name, profile=None):
    doc = testlib.create_test_doc(argv, name, profile)
    do_document(doc)
    doc.finalize()


def test_main(argv=None):
    #do_file(argv, "topdown2_off.pdf")
    profile = testlib.test_config()
    profile.set('doc.topdown', "1")
    do_file(argv, "topdown2_on.pdf", profile)
    profile.set('doc.encryption', 'standard')
    profile.set('stdsh.pwd_user', 'user')
    profile.set('stdsh.pwd_owner', 'owner')
    profile.set('info.static_producer', '1')
    profile.set('info.creation_date', '0')
    do_file(argv, "topdown2_on_enc.pdf", profile)


if __name__ == "__main__":
    test_main()


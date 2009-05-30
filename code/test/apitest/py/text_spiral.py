#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import math
import jagpdf
import jag.testlib as testlib

def test_main(argv=None):
    cfg = jagpdf.create_profile()
    cfg.set('doc.compressed', '1')
    doc = testlib.create_test_doc(argv, 'text_spiral.pdf', cfg)
    def get_angle(r, d):
        c = (d / 2.0) / r
        return math.pi - (2 * math.acos(c))
    font = doc.font_load('standard;name=Helvetica;size=28')
    media = 500, 500
    center = [c/2.0 for c in media]
    doc.page_start(*media)
    canvas = doc.page().canvas()
    canvas.text_font(font)
    n = 70
    r = 8
    radius = 180
    scale = 1.0
    color = 0.0
    # cross
    canvas.color('s', 0.9)
    canvas.move_to(0, center[1])
    canvas.line_to(media[0], center[1])
    canvas.move_to(center[0], 0)
    canvas.line_to(center[0], media[1])
    canvas.path_paint('s')
    canvas.color('s', 1.0)
    #text
    txt_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/text/lipsum.txt')
    data = open(txt_file).read()
    di = 0
    for j in range(r):
        for i in range(n):
            canvas.color('f', color)
            canvas.state_save()
            x = center[0] - (font.advance(data[0])/2.0)
            y = center[1] + radius
            canvas.translate(x, y)
            canvas.scale(scale, scale)
            canvas.text(0, 0, data[di])
            canvas.state_restore()
            canvas.translate(*center)
            radius -= 0.3
            scale -= 0.0015
            angle = get_angle(radius, scale * font.advance(data[di]))
            canvas.rotate(-angle)
            canvas.translate(-center[0], -center[1])
            color += 0.0015
            di += 1
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()


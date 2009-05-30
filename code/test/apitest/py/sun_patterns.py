# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import os
import string
import math

media = 400, 400

def function(doc, spot):
    def spot_transform(fn):
        in_1 = "0.5 sub 2 mul exch"
        out_1 = "2 div 0.5 add exch"
        return string.Template("{$in_1 $in_1 $fn $out_1 $out_1}").substitute(locals())
    double_dot = spot_transform('360 mul sin 2 div exch 360 mul sin 2 div add')
    rhomboid = spot_transform('abs exch abs 0.9 mul add 2 div')
    cos_dot = spot_transform('180 mul cos exch 180 mul cos add 2 div')
    return doc.function_4_load("domain=0, 1, 0, 1; range=0.0, 1.0; func=" + locals()[spot])


def do_sun(doc, canvas):
    canvas.color_space_pattern('fs')
#     # cloud
#     fn = function(doc, 'double_dot')
#     sh = doc.shading_pattern_load("function; matrix_fun=300, 0, 0, 300, 50, 50",
#                                   jagpdf.CS_DEVICE_GRAY, fn)
#     canvas.pattern('fs', sh)
#     canvas.rectangle(50, 50, 300, 300)
#     canvas.path_paint('fs')
    # sun
    fn1 = doc.function_2_load("domain=0.0, 1.0; c0=1, 0, 0; c1=1, 1, 0")
    fn2 = doc.function_2_load("domain=0.0, 1.0; c0=1, 1, 0; c1=1, 1, 1")
    fn = doc.function_3_load("domain=0, 1; bounds=0.85; encode=0, 1, 0, 1",
                             [fn1, fn2])
    cx, cy, r, r0, r1 = media[0]/2, media[1]/2, 100, 100, 180
    sh = doc.shading_pattern_load("radial; coords=0, 0, 0, 0 ,0, %d;"
                                  "matrix=1, 0, 0, 1, %d, %d" % (r, cx, cy),
                                  jagpdf.CS_DEVICE_RGB, fn)
    canvas.pattern('fs', sh)
    canvas.circle(cx, cy, r)
    canvas.path_paint('fs')
    # rays
    canvas.line_width(8)
    fn_w = doc.function_2_load("domain=0.0, 1.0; c0=1, 1, 1; c1=1, 1, 0")
    fn_ray = doc.function_3_load("domain=0, 1; bounds=0.3; encode=0, 1, 0, 1",
                             [fn_w, fn2])
    sh = doc.shading_pattern_load("radial; coords=0, 0, %d, 0 ,0, %d;"
                                  "matrix=1, 0, 0, 1, %d, %d" % (r0, r1, cx, cy),
                                  jagpdf.CS_DEVICE_RGB, fn_ray)
    canvas.pattern('fs', sh)
    nr_rays = 20
    step = math.pi * 2 / nr_rays
    angle = 0
    while angle < 2 * math.pi:
        canvas.move_to(r0 * math.cos(angle) + cx, r0 * math.sin(angle) + cy)
        canvas.line_to(r1 * math.cos(angle) + cx, r1 * math.sin(angle) + cy)
        angle += step
    canvas.path_paint('fs')


def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'sun.pdf')
    doc.page_start(*media)
    do_sun(doc, doc.page().canvas())
    doc.page_end()
    doc.finalize()

if __name__ == '__main__':
    test_main()

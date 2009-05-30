# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
# import os
# import string
#
# # shading in rgb: 660099, ffff00  .. fialova, zlute
# #           cmyk: <.33, 1, 0, 0.4>, <0, 0, 1, 0>
#
# dim = 200, 200
#
# def do_rectangle(canvas, sh):
#     canvas.color_space_pattern('fs')
#     canvas.pattern('fs', sh)
#     canvas.rectangle(0, 0, *dim)
#     canvas.path_paint('f')
#
# def axial_basic(doc, canvas):
#     # basic axial shading
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=1.0, 0.0, 0.0; c1=0.0, 0.0, 1.0")
#     sh = doc.shading_pattern_load("axial; coords=50, 50, 150, 150", jagpdf.CS_DEVICE_RGB, fn)
#     do_rectangle(canvas, sh)
#
# def axial_matrix(doc, canvas):
#     # axial shading with changed matrix
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=1.0, 0.0, 0.0; c1=0.0, 0.0, 1.0")
#     sh = doc.shading_pattern_load("axial; coords=50, 50, 150, 150;matrix=1, 0, 0, 1, 50, 0", jagpdf.CS_DEVICE_RGB, fn)
#     do_rectangle(canvas, sh)
#
# def axial_extend(doc, canvas):
#     # axial shading extending beyond the starting point
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=1.0, 0.0, 0.0; c1=0.0, 0.0, 1.0")
#     sh = doc.shading_pattern_load("axial; coords=50, 50, 150, 150; extend=1, 0", jagpdf.CS_DEVICE_RGB, fn)
#     do_rectangle(canvas, sh)
#
# def axial_domain(doc, canvas):
#     # axial shading with changed domain
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=1.0, 0.0, 0.0; c1=0.0, 0.0, 1.0")
#     sh = doc.shading_pattern_load("axial; coords=50, 50, 150, 150; domain=0.25, 0.75", jagpdf.CS_DEVICE_RGB, fn)
#     do_rectangle(canvas, sh)
#
# def radial_basic(doc, canvas):
#     # basic radial shading
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=0.0, 1.0, 0.0; c1=0.0, 0.0, 1.0")
#     sh = doc.shading_pattern_load("radial; coords=100, 100, 0, 100, 100, 80", jagpdf.CS_DEVICE_RGB, fn)
#     do_rectangle(canvas, sh)
#
# def radial_bbox(doc, canvas):
#     # radial shading with bounding box
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=0.0, 1.0, 0.0; c1=0.0, 0.0, 1.0")
#     sh = doc.shading_pattern_load("radial; coords=100, 100, 0, 100, 100, 100;bbox=50, 50, 150, 150", jagpdf.CS_DEVICE_RGB, fn)
#     do_rectangle(canvas, sh)
#
# def radial_background(doc, canvas):
#     # radial shading with background
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=0.0, 1.0, 0.0; c1=0, 0, 1")
#     sh = doc.shading_pattern_load("radial; coords=100, 100, 20, 100, 100, 80;background=0.8, 0.5, 1.0", jagpdf.CS_DEVICE_RGB, fn)
#     do_rectangle(canvas, sh)
#
# def radial_shading_operator(doc, canvas):
#     # radial shading with paint operator
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=0.0, 1.0, 0.0; c1=0.0, 0.0, 1.0")
#     sh = doc.shading_pattern_load("radial; coords=0, 0, 0 ,0, 0, 1", jagpdf.CS_DEVICE_RGB, fn)
#     for x, y in [[50,50], [50,150], [150, 150], [150, 50]]:
#         canvas.state_save()
#         canvas.circle(x, y, 30)
#         canvas.path_paint('w')
#         canvas.transform(30, 0, 0, 30, x, y)
#         canvas.shading_apply(sh)
#         canvas.state_restore()
#
# def axial_stitching(doc, canvas):
#     #009999 - #FF6600
#     fn1 = doc.function_2_load("domain=0.0, 1.0; c0=0.0, 0.6, 0.6; c1=1.0, 0.4, 0.0")
#     #FF6600 - #FFB200
#     fn2 = doc.function_2_load("domain=0.0, 1.0; c0=1.0, 0.4, 0.0; c1=1.0, 0.7, 0.0")
#     #FFB200 - #1919B3
#     fn3 = doc.function_2_load("domain=0.0, 1.0; c0=1.0, 0.7, 0.0; c1=0.1, 0.1, 0.7")
#     fn = doc.function_3_load("domain=0, 1; bounds=0.33, 0.66; encode=0, 1, 0, 1, 0, 1",
#                              [fn1, fn2, fn3])
#     sh = doc.shading_pattern_load("axial; coords=0, 50, 200, 50", jagpdf.CS_DEVICE_RGB, fn)
#     do_rectangle(canvas, sh)
#
#
# def function(doc, spot):
#     def spot_transform(fn):
#         in_1 = "0.5 sub 2 mul exch"
#         out_1 = "2 div 0.5 add exch"
#         return string.Template("{$in_1 $in_1 $fn $out_1 $out_1}").substitute(locals())
#     double_dot = spot_transform('360 mul sin 2 div exch 360 mul sin 2 div add')
#     rhomboid = spot_transform('abs exch abs 0.9 mul add 2 div')
#     cos_dot = spot_transform('180 mul cos exch 180 mul cos add 2 div')
#     return doc.function_4_load("domain=0, 1, 0, 1; range=0.0, 1.0; func=" + locals()[spot])
#
# def function_basic(doc, canvas):
#     # basic function
#     fn = function(doc, 'double_dot')
#     sh = doc.shading_pattern_load("function; matrix_fun=100, 0, 0, 100, 50, 50", jagpdf.CS_DEVICE_GRAY, fn)
#     do_rectangle(canvas, sh)
#
# def function_multi(doc, canvas):
#     # three functions
#     fn1 = function(doc, 'rhomboid')
#     fn2 = function(doc, 'double_dot')
#     fn3 = function(doc, 'cos_dot')
#     sh = doc.shading_pattern_load_n("function; matrix_fun=100, 0, 0, 100, 50, 50", jagpdf.CS_DEVICE_RGB, [fn1, fn2, fn3])
#     do_rectangle(canvas, sh)
#
#
# def two_vertical_shadings(doc, canvas, cs1, cs2, c0, c1, c02=None, c12=None):
#     def c2s(c):
#         return ','.join([str(i) for i in c])
#     c02 = c02 or c0
#     c12 = c12 or c1
#     fn_1 = doc.function_2_load("domain=0.0, 1.0; c0=%s; c1=%s" % (c2s(c0), c2s(c1)))
#     fn_2 = doc.function_2_load("domain=0.0, 1.0; c0=%s; c1=%s" % (c2s(c02), c2s(c12)))
#     sh_1 = doc.shading_pattern_load("axial; coords=0, 0, 0, 200", cs1, fn_1)
#     sh_2 = doc.shading_pattern_load("axial; coords=0, 0, 0, 200", cs2, fn_2)
#     canvas.color_space_pattern('fs')
#     canvas.pattern('fs', sh_1)
#     canvas.rectangle(0, 0, 100, 200)
#     canvas.path_paint('f')
#     canvas.pattern('fs', sh_2)
#     canvas.rectangle(100, 0, 100, 200)
#     canvas.path_paint('f')
#
#
# def axial_space_diff(doc, canvas):
#     two_vertical_shadings(doc, canvas, jagpdf.CS_DEVICE_RGB, jagpdf.CS_DEVICE_CMYK,
#                           [0.4, 0, 0.6], [1, 1, 0],           # rgb colors
#                           [.33, 1, 0, 0.4], [0, 0, 1, 0])   # cmyk colors
#
# def axial_parametrized_spaces(doc, canvas):
#     calrgb = doc.color_space_load("calrgb; gamma=2.2, 2.2, 2.2; white=0.9505, 1.089; matrix=0.4124, 0.2126, 0.0193, 0.3576, 0.7152, 0.1192, 0.1805, 0.0722, 0.9505")
#     adobergb = doc.color_space_load("adobe-rgb")
#     two_vertical_shadings(doc, canvas, calrgb, adobergb, [0.4, 0, 0.6], [1, 1, 0])
#
# def axial_diff2(doc, canvas):
#     srgb = doc.color_space_load("srgb")
#     adobergb = doc.color_space_load("adobe-rgb")
#     two_vertical_shadings(doc, canvas, srgb, adobergb, [0.4, 0, 0.6], [1, 1, 0])
#
#
# def check_errors():
#     doc = jagpdf.create_stream(testlib.NoopStreamOut())
#     doc.page_start(1, 1)
#     fn = function(doc, 'double_dot')
#     sh = doc.shading_pattern_load("function; matrix_fun=100, 0, 0, 100, 50, 50", jagpdf.CS_DEVICE_GRAY, fn)
#     canvas = doc.page().canvas()
#     canvas.color_space_pattern('fs')
#     testlib.must_throw(canvas.pattern, 'fs', sh, 1)
#     canvas.color_space_pattern_uncolored('fs', jagpdf.CS_DEVICE_GRAY)
#     canvas.pattern('fs', sh) # should throw
#     # indexed not allowed pattern spaces
#     indexed_cs = doc.color_space_load("adobe-rgb; palette=77, 179, 102")
#     fn = doc.function_2_load("domain=0.0, 1.0; c0=1.0, 0.0, 0.0; c1=0.0, 0.0, 1.0")
#     testlib.must_throw(doc.shading_pattern_load, "axial; coords=50, 50, 150, 150", indexed_cs, fn)
#     doc.page_end()
#     doc.finalize()


def test_main(argv=None):
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    # function 2
    doc.function_2_load("domain=0.0, 1.0; c0=1; c1=2")
    doc.function_2_load("domain=0.0, 1.0")
    f1 = doc.function_2_load("c0=11; c1=12")
    f2 = doc.function_2_load("exponent=2")
    doc.function_2_load("")
    doc.function_2_load("c1=12")
    testlib.must_throw(doc.function_2_load, "c0=11; c1=12,23")
    testlib.must_throw(doc.function_2_load, "c1=12,23")
    # function 3
    doc.function_3_load("bounds=0.3; encode=0, 1, 0, 1", [f1, f2])
    # function 4

    doc.page_start(10, 10)
    doc.page_end()
    doc.finalize()

if __name__ == '__main__':
    test_main()

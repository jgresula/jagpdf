# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import os


def create_col_pattern(writer, blue):
    #define pattern
    pw = writer.canvas_create()
    pw.color_space("f", jagpdf.CS_DEVICE_RGB)
    pw.color("f", 1, 0, blue)
    pw.rectangle(0, 0, 20, 20)
    pw.path_paint("f")
    pw.color("f", 0, 1, blue)
    pw.circle(20, 20, 15)
    pw.path_paint("f")
    return writer.tiling_pattern_load("step=35, 35", pw)


def create_uncol_pattern(writer, step):
    #define pattern
    pw = writer.canvas_create()
    for y in range(0, 40, step):
        pw.move_to(0, y)
        pw.line_to(40, y)
    pw.path_paint("s")
    return writer.tiling_pattern_load("step=40, 40", pw)


def draw_col_page(writer, pids, pre_action, paint_op):
    writer.page_start(3*72, 2*72)
    page = writer.page().canvas()
    if pre_action:
        pre_action(page)
    page.color_space_pattern(paint_op)
    page.pattern(paint_op, pids[0])
    page.circle(62, 72, 36)
    page.path_paint(paint_op)
    page.pattern(paint_op, pids[1])
    page.circle(154, 72, 36)
    page.path_paint(paint_op)
    writer.page_end()


def draw_uncol_page(writer, pids, pre_action, paint_op):
    for cs, p1, p2 in [(jagpdf.CS_DEVICE_GRAY,\
                         lambda p: page.pattern(paint_op, pids[0], .25),\
                         lambda p: page.pattern(paint_op, pids[1], .75)),\
                       (jagpdf.CS_DEVICE_RGB,\
                         lambda p: page.pattern(paint_op, pids[0], 0, 1, 0),\
                         lambda p: page.pattern(paint_op, pids[1], 1, 0, 0)),\
                       (jagpdf.CS_DEVICE_CMYK,\
                         lambda p: page.pattern(paint_op, pids[0], 1, 1, 0, .5),\
                         lambda p: page.pattern(paint_op, pids[1], 0, .61, .72, 0)),\
                  ]:
        writer.page_start(3*72, 2*72)
        page = writer.page().canvas()
        if pre_action:
            pre_action(page)
        page.color_space_pattern_uncolored(paint_op, cs)
        p1(page)
        page.circle(62, 72, 36)
        page.path_paint(paint_op)
        p2(page)
        page.circle(154, 72, 36)
        page.path_paint(paint_op)
        writer.page_end()


def do_colored_page(writer):
    pids = [create_col_pattern(writer, 0), create_col_pattern(writer, .5)]
    draw_col_page(writer, pids, None, "f")
    draw_col_page(writer, pids, lambda pg: pg.line_width(10), "s")


def do_uncolored_page(writer):
    pids = [create_uncol_pattern(writer, 2), create_uncol_pattern(writer, 4)]
    draw_uncol_page(writer, pids, None, "f")
    draw_uncol_page(writer, pids, lambda pg: pg.line_width(10), "s")

def check_errors():
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    doc.page_start(1, 1)
    canvas = doc.page().canvas()
    colored = create_col_pattern(doc, 1)
    uncolored = create_uncol_pattern(doc, 1)
    #
    canvas.color_space_pattern('fs')
    testlib.must_throw(canvas.pattern, 'fs', colored, 1)
    canvas.pattern('fs', uncolored, 1)     # should throw
    #
    canvas.color_space_pattern_uncolored('fs', jagpdf.CS_DEVICE_CMYK)
    testlib.must_throw(canvas.pattern, 'fs', uncolored)
    canvas.pattern('fs', colored)     # should throw
    doc.page_end()
    doc.finalize()

def check_matrix(argv):
    doc = testlib.create_test_doc(argv, 'tiling_patterns2.pdf')
    # load uncolored pattern
    p = doc.canvas_create()
    p.circle(20, 20, 20)
    p.path_paint('s')
    pid = doc.tiling_pattern_load("step=40,40", p)
    p = doc.canvas_create()
    p.circle(20, 20, 20)
    p.path_paint('fs')
    pid2 = doc.tiling_pattern_load("matrix=1.25, 0, 0, 1.25, 50, 50; step=40,40", p)
    # use it
    doc.page_start(200, 200)
    canvas = doc.page().canvas()
    canvas.color_space_pattern_uncolored('f', jagpdf.CS_DEVICE_GRAY)
    canvas.pattern('f', pid, .5)
    canvas.rectangle(0, 0, 200, 200)
    canvas.path_paint('f')
    canvas.pattern('f', pid2, .5)
    canvas.rectangle(50, 50, 100, 100)
    canvas.path_paint('fs')
    doc.page_end()
    doc.finalize()






def test_main(argv=None):
    doc_writer = testlib.create_test_doc(argv, 'tiling_patterns.pdf')
    do_colored_page(doc_writer)
    do_uncolored_page(doc_writer)
    doc_writer.finalize()
    check_errors()
    check_matrix(argv)

if __name__ == '__main__':
    test_main()

#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import jag.arc as arc
import math

paper = 597.6, 848.68

def lines(canvas, x, y, rx=1.0, ry=1.0, angles=[]):
    """draws a vertical line and optionally lines with given angles"""
    canvas.state_save()
    canvas.color_space('s', jagpdf.CS_DEVICE_GRAY)
    canvas.color('s', 0.8)
    canvas.move_to(0, y)
    canvas.line_to(paper[0], y)
    canvas.path_paint('s')
    if angles:
        canvas.color('s', 0.4)
        for a in angles:
            canvas.move_to(x, y)
            canvas.line_to(x + rx * math.cos(a), y + ry * math.sin(a))
        canvas.path_paint('s')
    canvas.state_restore()

def apply_cmds(canvas, cmds):
    for cmd, args in cmds:
        getattr(canvas, cmd)(*args)

def jagpdf_arc(canvas, cx, cy, rx, ry, start, sweep):
    canvas.arc(cx, cy, rx, ry, start, sweep)

def jagpdf_arc_to(canvas, x1, y1, rx, ry, x_rot, large_arc, sweep_flag):
    canvas.arc_to(x1, y1, rx, ry, x_rot, large_arc, sweep_flag)


def arc_center(doc):
    doc.page_start(*paper)
    canvas = doc.page().canvas()
    canvas.color('s', 0.8)
    canvas.move_to(300, 0)
    canvas.line_to(300, paper[1])
    canvas.path_paint('s')
    canvas.color_space('fs', jagpdf.CS_DEVICE_RGB)
    canvas.color('fs', .0, .0, .0)
    # ellipse
    center = 300, 750
    lines(canvas, *center)
    jagpdf_arc(canvas, center[0], center[1], 200, 70, 0, 2 * math.pi)
    canvas.path_paint('s')
    # 1/2 quadrants
    canvas.state_save()
    center = 300, 550
    step = math.pi / 4.0
    angles = [step * i for i in range(8)]
    lines(canvas, center[0], center[1], 200, 70, angles)
    colors = [[1.0, 0, 0], [0.0, 1.0, 0]]
    for i, a in enumerate(angles):
        canvas.color('s', *colors[i % 2])
        jagpdf_arc(canvas, center[0], center[1], 200, 70, a, step)
        canvas.path_paint('s')
    canvas.state_restore()
    # close
    center = 300, 350
    lines(canvas, *center)
    jagpdf_arc(canvas, center[0], center[1], 200, 70, math.pi / 4.0, math.pi)
    canvas.path_close()
    canvas.path_paint('s')
    # fill
    center = 300, 150
    lines(canvas, *center)
    canvas.color('f', 0.8, 0.8, 0.9)
    jagpdf_arc(canvas, center[0], center[1], 200, 70, math.pi / 4.0, math.pi)
    canvas.path_close()
    canvas.path_paint('sf')
    doc.page_end()

def arc_endpoints(doc):
    def line_bump(y, rx, ry, rotx, large, sweep):
        canvas.move_to(50, y)
        canvas.line_to(280, y)
        jagpdf_arc_to(canvas, 320, y, rx, ry, rotx, large, sweep)
        canvas.line_to(550, y)
        canvas.path_paint('s')
    doc.page_start(*paper)
    canvas = doc.page().canvas()
    canvas.color_space('fs', jagpdf.CS_DEVICE_RGB)
    line_bump(800, 20, 20, 0, 0, 0)
    line_bump(780, 40, 40, 0, 0, 0)
    line_bump(760, 40, 40, 0, 1, 0)
    line_bump(740, 40, 40, 0, 1, 1)
    line_bump(720, 40, 40, 0, 0, 1)
    line_bump(680, 20, 40, 0, 0, 0)
    line_bump(580, 80, 40, math.pi/4.0, 0, 0)
    line_bump(540, 80, 40, math.pi/4.0, 0, 1)
    line_bump(500, 80, 40, math.pi/4.0, 1, 0)
    line_bump(460, 80, 40, math.pi/4.0, 1, 1)
    doc.page_end()


def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'arc.pdf')
    arc_endpoints(doc)
    arc_center(doc)
    doc.finalize()

if __name__ == "__main__":
    test_main()


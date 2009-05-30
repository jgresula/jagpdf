# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# portions of the code taken from http://www.antigrain.com, file
# agg_bezier_arc.cpp

import math

"""Implements the following function for converting elliptical arc to
a sequence of bezier segments.

arc_to_bezier_center()     ... center representation
arc_to_bezier_endopoints() ... endpoints representation

They return an array of commands. Each command is
[<move_to,line_to,bezier_to>, [[coord-pair1], ... [coord-pairN]]]
"""


# ----------------------------------------------------------------------
#                      elliptic arc to bezier
bezier_arc_angle_epsilon = 0.01

def arc_to_bezier_segment(cx, cy, rx, ry, start_angle, sweep_angle):
    """Converts arc to a bezier segment."""
    x0 = math.cos(sweep_angle / 2.0)
    y0 = math.sin(sweep_angle / 2.0)
    tx = (1.0 - x0) * 4.0 / 3.0
    ty = y0 - tx * x0 / y0
    px = [x0, x0 + tx, x0 + tx, x0]
    py = [-y0, -ty, ty, y0]
    sn = math.sin(start_angle + sweep_angle / 2.0)
    cs = math.cos(start_angle + sweep_angle / 2.0)
    curve = 8 * [0]
    for i in xrange(4):
        curve[i * 2]     = cx + rx * (px[i] * cs - py[i] * sn)
        curve[i * 2 + 1] = cy + ry * (px[i] * sn + py[i] * cs)
    return curve

def arc_to_bezier_center(x, y, rx, ry, start_angle, sweep_angle, move_to=True):
    """For given arc center representation returns series of move_to,
    line_to and bezier_to commands."""
    start_angle = start_angle % (2.0 * math.pi)
    sweep_angle = min(sweep_angle, 2.0 * math.pi)
    sweep_angle = max(sweep_angle, -2.0 * math.pi)

    result = []
    if abs(sweep_angle) < 1e-10:
        if move_to:
            result.append(['move_to',
                           [x + rx * math.cos(start_angle),
                            y + ry * math.sin(start_angle)]])
        result.append(['line_to',
                       [x + rx * math.cos(start_angle + sweep_angle),
                        y + ry * sin(start_angle + sweep_angle)]])
        return result
    total_sweep = 0.0
    local_sweep = 0.0
    prev_sweep = 0.0
    num_vertices = 2
    done = False
    segments = []
    while 1:
        if sweep_angle < 0.0:
            prev_sweep  = total_sweep
            local_sweep = -math.pi * 0.5
            total_sweep -= math.pi * 0.5
            if total_sweep <= sweep_angle + bezier_arc_angle_epsilon:
                local_sweep = sweep_angle - prev_sweep
                done = True
        else:
            prev_sweep  = total_sweep
            local_sweep =  math.pi * 0.5
            total_sweep += math.pi * 0.5
            if total_sweep >= sweep_angle - bezier_arc_angle_epsilon:
                local_sweep = sweep_angle - prev_sweep
                done = True
        segments.append(arc_to_bezier_segment(x, y, rx, ry,
                                              start_angle,
                                              local_sweep))
        num_vertices += 6
        start_angle += local_sweep
        if done or num_vertices >= 26:
            break
    result = []
    if move_to:
        result.append(['move_to', [segments[0][0], segments[0][1]]])
    result += [['bezier_to', segment[2:]] for segment in segments]
    return result


def angle_between_vectors(a, b):
    def dot(u, v):
        return u[0]*v[0] + u[1]*v[1]
    la = math.sqrt(dot(a, a))
    lb = math.sqrt(dot(b, b))
    angle = math.acos(dot(a,b)/(la*lb))
    if (a[0]*b[1] - a[1]*b[0]) < 0:
        angle = -angle
    return angle


def arc_to_bezier_endpoints(x0, y0, rx, ry, xasis_rot, large_arc_flag, sweep_flag, x, y, move_to=False):
    """For given arc endpoints representation returns series of
    move_to, line_to and bezier_to commands."""
    # correct radii
    if rx == 0.0 or ry == 0.0:
        return [['line_to', [x, y]]]
    rx, ry = abs(rx), abs(ry)
    # http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
    # F.6.5 Conversion from endpoint to center parameterization
    #
    # step1
    dx2 = (x0-x)/2.0
    dy2 = (y0-y)/2.0
    x1 = math.cos(xasis_rot)*dx2 + math.sin(xasis_rot)*dy2
    y1 = -math.sin(xasis_rot)*dx2 + math.cos(xasis_rot)*dy2
    # step2
    rx2 = rx*rx
    ry2 = ry*ry
    y12 = y1*y1
    x12 = x1*x1
    # correction
    L = x12/rx2 + y12/ry2
    if L > 1:
        L2 = math.sqrt(L)
        rx = L2 * rx
        ry = L2 * ry
        rx2 = rx*rx
        ry2 = ry*ry
    cf = math.sqrt((rx2*ry2 - rx2*y12 - ry2*x12)/(rx2*y12 + ry2*x12))
    cx1 = cf * rx*y1/ry
    cy1 = -cf * ry*x1/rx
    if large_arc_flag == sweep_flag:
        cx1 = -cx1
        cy1 = -cy1
    # step3
    sx2 = (x0+x)/2.0
    sy2 = (y0+y)/2.0
    cx = math.cos(xasis_rot)*cx1 + math.sin(xasis_rot)*cy1 + sx2
    cy = -math.sin(xasis_rot)*cx1 + math.cos(xasis_rot)*cy1 + sy2
    # step4
    pi2 = 2*math.pi
    xt = (x1-cx1)/rx
    yt = (y1-cy1)/ry
    theta = angle_between_vectors((1,0), (xt, yt))
    xm = (-x1-cx1)/rx
    ym = (-y1-cy1)/ry
    dtheta = angle_between_vectors((xt, yt), (xm, ym)) % pi2
    if sweep_flag == 0 and dtheta > 0.0:
        dtheta -= pi2
    elif sweep_flag == 1 and dtheta < 0.0:
        dtheta += pi2
    return arc_to_bezier_center(cx, cy, rx, ry, theta, dtheta, move_to)

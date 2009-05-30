#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib

import os
import math
import datetime

#
# Helpers
#
class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

class Font:
    def __init__(self):
        self.doc = None
        self.fonts = {}

    def __call__(self, size, bold=False):
        if not (size,bold) in self.fonts:
            fnt = self.doc.font_load('standard; name=%s; size=%d; enc=windows-1250' %
                                     ({False: 'Helvetica',
                                       True: 'Helvetica-Bold'}[bold],
                                      size))
            self.fonts[(size,bold)] = fnt
        return self.fonts[(size,bold)]


#
# Globals
#
s_font = Font()
s_color_dark = 0, 0, 0
s_color_light = 1.0, 0.54509803921568623, 0.0
s_color_bright = 1.0, 0.74901960784313726, 0.45098039215686275
s_page_dim = 500, 600



#
# Implementation
#
def center_text(canvas, font, y, text):
    width = font.advance(text)
    x = (s_page_dim[0] - width) / 2.0
    canvas.text_font(font)
    canvas.text(x, y, text)

def number_to_angle(val):
    return math.pi / 2 - val * math.pi / 30

def time_to_angles(t):
    hour = (t.hour % 12) * 5.
    hour += 5 * t.minute / 60.0
    return number_to_angle(hour), \
           number_to_angle(t.minute), \
           number_to_angle(t.second)

def clock_from_rect(x, y, w, h):
    return Bunch(cx = x + w / 2.0, \
                 cy = y + h / 2.0, \
                 r =  min(w, h) / 2.0)

def draw_time(canvas, c, t):
    # draw hands
    alpha_h, alpha_m, alpha_s = time_to_angles(t)
    canvas.line_cap(jagpdf.LINE_CAP_ROUND)
    draw_hand(canvas, c, alpha_h, 0.6, 8)
    draw_hand(canvas, c, alpha_m, 0.8, 8)
    draw_hand(canvas, c, alpha_s, 0.87, 2)
    # show title
    canvas.color('fs', *s_color_dark)
    center_text(canvas, s_font(30, True), 550, "Current Time in Prague")
    center_text(canvas, s_font(24, True), 510, t.strftime('%I:%M:%S %p'))

def draw_hand(canvas, c, alpha, factor, w):
    canvas.line_width(w)
    canvas.move_to(c.cx, c.cy)
    r = c.r * factor
    x = c.cx + r * math.cos(alpha)
    y = c.cy + r * math.sin(alpha)
    canvas.line_to(x, y)
    canvas.path_paint("s")

def draw_clock(doc, c):
    canvas = doc.page().canvas()
    # radial shading
    col_0 = "%f, %f, %f" % s_color_bright
    col_1 = "%f, %f, %f" % s_color_light
    spec = "domain=0, 1; c0=%(col_0)s; c1=%(col_1)s" % locals()
    interp = doc.function_2_load(spec)
    sh = doc.shading_pattern_load("radial; coords=0, 0, 0, 0, 0, 1",
                                  jagpdf.CS_DEVICE_RGB,
                                  interp)
    canvas.state_save()
    canvas.circle(c.cx, c.cy, c.r)
    canvas.path_paint('w')
    canvas.transform(c.r, 0, 0, c.r, c.cx, c.cy)
    canvas.shading_apply(sh)
    canvas.state_restore()
    # draw clock shape
    canvas.color('fs', *s_color_dark)
    canvas.line_width(12)
    canvas.circle(c.cx, c.cy, c.r)
    canvas.path_paint('s')
    draw_klenot(doc)
    # draw clock face
    step = 2 * math.pi / 60
    angle = math.pi / 2
    r0 = .90 * c.r
    rm = .95 * c.r
    canvas.state_save()
    for i in range(60):
        sin, cos = math.sin(angle), math.cos(angle)
        if 0 == i % 5:
            canvas.line_width(4)
            canvas.move_to(c.cx + r0 * cos, c.cy + r0 * sin)
        else:
            canvas.line_width(2)
            canvas.move_to(c.cx + rm * cos, c.cy + rm * sin)
        canvas.line_to(c.cx + c.r * cos, c.cy + c.r * sin)
        angle = angle - step
        canvas.path_paint('s')
    canvas.state_restore()

def draw_klenot(doc):
    canvas = doc.page().canvas()
    y = 100
    # image
    img_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/klenot.png')
    img = doc.image_load_file(img_file)
    img_width = img.width() / img.dpi_x() * 72
    img_height = img.height() / img.dpi_y() * 72
    x = (s_page_dim[0] - img_width) / 2.0
    canvas.image(img, x, y)
    # hyperlink
    doc.page().annotation_uri(x, y, img_width, img_height, "http://www.klenot.cz")
    # text
    canvas.color('f', *s_color_dark)
    center_text(canvas, s_font(10), y + img_height + 5, 'hosted by')

def draw(doc):
    t = datetime.datetime.today()
    c = clock_from_rect(25, 25, 450, 450)
    doc.page().canvas().color_space("fs", jagpdf.CS_DEVICE_RGB)
    draw_clock(doc, c)
    draw_time(doc.page().canvas(), c, t)

def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'clock.pdf')
    s_font.doc = doc
    doc.page_start(*s_page_dim)
    draw(doc)
    doc.page_end()

    doc.finalize()

if __name__ == '__main__':
    test_main()

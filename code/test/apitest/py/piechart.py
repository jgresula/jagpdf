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

paper = 597.6, 848.68

# -- scheme generator
# http://www.wellstyled.com/tools/colorscheme2/index-en.html
colors_str="""#FF3300
#B32400
#FFCCBF
#FF9980
#00B366
#007D48
#BFFFE4
#80FFC9
#0033CC
#00248F
#BFCFFF
#809FFF
#FF9900
#B36B00
#FFE6BF
#FFCC80
"""
chart_data = [
    ('Africa', 767),
    ('Asia', 3634),
    ('Europe', 729),
    ('South America and the Caribbean', 511),
    ('Northern America', 307),
    ('Oceania', 30)
   ]

def color_to_rgb(str):
    r = int(str[1:3], 16) / 255.0
    g = int(str[3:5], 16) / 255.0
    b = int(str[5:7], 16) / 255.0
    return r, g, b

colors = [color_to_rgb(c) for c in colors_str.split()]

def get_color():
    for i in [0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14]:
        yield colors[i]

def draw_piechart(canvas, cx, cy, rx, ry, font, items, title, font_title):
    """items is a sequence of [name, quantity]"""
    total_quant = reduce(lambda s, i: s + i[1], items, 0)
    items.sort(lambda l, r: cmp(r[1], l[1]))
    color = get_color()
    color_map = dict([(name, color.next()) for name, q in items])
    items.reverse()
    canvas.color_space('fs', jagpdf.CS_DEVICE_RGB)
    # shadow
#     canvas.color('f', 0.2, 0.2, 0.2)
#     canvas.arc(cx+4, cy-4, rx, ry, 0, 2 * math.pi)
#     canvas.path_paint('f')
    # chart
    angle = math.pi / 2.0
    color = get_color()
    max_str_len = 0.0
    canvas.line_join(jagpdf.LINE_JOIN_BEVEL)
    for name, quant in items:
        canvas.color('fs', *color_map[name])
        sweep = quant * 2 * math.pi / total_quant
        canvas.arc(cx, cy, rx, ry, angle, sweep)
        canvas.line_to(cx, cy)
        canvas.path_close()
        canvas.path_paint('fs')
        angle += sweep
        max_str_len = max(max_str_len, font.advance(name))
    # legend boxes
    items.reverse()
    legend_x, legend_y = cx - rx, cy + ry + (1 + len(items))*font.height()
    y = 0
    box_h = font.bbox_ymax() - font.bbox_ymin()
    box_w = 20
    for name, quant in items:
        canvas.color('f', *color_map[name])
        canvas.rectangle(legend_x, legend_y - y + font.bbox_ymin(), box_w, box_h)
        canvas.path_paint('f')
        y += font.height()
    # legend text
    canvas.text_font(font)
    canvas.text_start(legend_x + box_w + 8, legend_y)
    perc_offset = max_str_len + 10
    canvas.color('f', 0, 0, 0)
    for name, quant in items:
        canvas.text("%s" % name)
        canvas.text_translate_line(perc_offset, 0)
        canvas.text("%.2f%%" % (100.0 * quant / total_quant))
        canvas.text_translate_line(-perc_offset, -font.height())
    canvas.text_end()
    # edge
#     canvas.color('s', 0.5, 0.5, 0.5)
#     canvas.arc(cx, cy, rx, ry, 0, 2 * math.pi)
#     canvas.path_paint('s')
    # title
    canvas.text_font(font_title)
    canvas.color('f', 0, 0, 0)
    title_w = font_title.advance(title)
    canvas.text(legend_x + ((2 * rx - title_w) / 2.0), \
                legend_y + 1.4 * font_title.height(), title)



def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'piechart.pdf')
    font = doc.font_load('standard;name=Helvetica;size=12')
    font_title = doc.font_load('standard;name=Helvetica-Bold;size=28')
    doc.page_start(*paper)
    canvas = doc.page().canvas()
    draw_piechart(canvas, 300, 400, 250, 200, font,
                  chart_data, 'World Population', font_title)
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()



#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf

#
# text formatting
#
def format_text(rect, txt, doc, fontid,
                 direction="topdown",
                 show_lines=True,
                 baseline="inside",
                 align="left",
                 para_spacing=1.0):
    finfo = fontid
    width, height = rect[2], rect[3]
    if direction == "topdown":
        topdown_factor = -1
        origin = [rect[0], height+rect[1]]
    else: # bottomup
        topdown_factor = 1
        origin = [rect[0], rect[1]]
    linespacing = topdown_factor*finfo.height()
#    linespacing = topdown_factor*12.0
    if baseline=="inside":
        origin[1] += (direction=="topdown" and [-finfo.bbox_ymax()] or [finfo.bbox_ymin()])[0]

    curr_height = 0
    canvas = doc.page().canvas()
    canvas.text_font(fontid)
    canvas.text_start(*origin)
    coef = -(1000/finfo.size())
    for para in txt.split('\n'):
        for line, linew, is_last in find_linebreaks(para, width, finfo):
            adj = [], []
            if align=="right":
                adj = [coef*(width-linew)], [0]
            elif align=="center":
                adj = [coef*(width-linew)/2], [0]
            elif align=="justify":
                space_pos = [i for i,c in enumerate(line) if c==' ']
                nr_spaces = len(space_pos)
                if nr_spaces and not is_last:
                    adj = nr_spaces*[coef*(width-linew)/nr_spaces] ,space_pos
            canvas.text(line, *adj)
            curr_height += linespacing
            if abs(curr_height)>height:
                break
            canvas.text_translate_line(0, linespacing)
            #canvas.translate(0, linespacing)
        curr_height += para_spacing*linespacing
        if abs(curr_height)>height:
            break
        canvas.text_translate_line(0, para_spacing*linespacing)
    canvas.text_end()


# def find_linebreaks(para, width, metrics):
#     def is_last_line(n):
#         return n==num_words-1
#     line = []
#     curr = 0
#     words = para.split()
#     num_words = len(words)
#     spacew = metrics.advance(' ')
#     for i, word in enumerate(words):
#         wl = metrics.advance(word)
#         if curr + wl > width:
#             yield ' '.join(line), curr - spacew, is_last_line(i-1)
#             line = []
#             curr = 0
#         line.append(word)
#         curr += wl + spacew
#     if line:
#         yield ' '.join(line), curr - spacew, is_last_line(i)


def find_linebreaks(para, width, metrics):
    def is_last_line(n):
        return n==num_words-1
    line = []
    curr = 0
    words = para.split()
    num_words = len(words)
    for i, word in enumerate(words):
        if curr > 0:
            wl = metrics.advance(' ' + word)
        else:
            wl = metrics.advance(word)
        if curr + wl > width:
            yield ' '.join(line), curr, is_last_line(i-1)
            line = []
            curr = 0
            wl = metrics.advance(word)
        line.append(word)
        curr += wl
    if line:
        yield ' '.join(line), curr, is_last_line(i)


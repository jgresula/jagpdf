#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf

class Padding:
    def __init__(self, tbl, line_i, cell_i):
        self.tbl = tbl
        self.line_i = line_i
        self.cell_i = cell_i

    def __getitem__(self, pos):
        try:
            p = self.tbl.props_cell[('cell_pad', self.line_i, self.cell_i)]
            return p[pos]
        except KeyError:
            pass
        try:
            p = self.tbl.props_line[('cell_pad', self.line_i)]
            return p[pos]
        except KeyError:
            pass
        try:
            p = self.tbl.props_column[('cell_pad', self.cell_i)]
            return p[pos]
        except KeyError:
            pass
        p = self.tbl.props_table['cell_pad']
        return p[pos]

class DefaultTableStyle:
    def header_row_bg(self, canvas, *rect):
        pass

    def footer_row_bg(self, canvas, *rect):
        pass

    def zebra_row_bg(self, canvas, *rect):
        pass


class SimpleTable:
    def __init__(self,
                 doc,
                 widths,
                 font,
                 style=DefaultTableStyle(),
                 header=0,
                 footer=0):
        self.doc = doc
        self.widths = widths
        self.lines = []
        self.zebra_sh = None
        self.zebra_pred = lambda line_no: line_no % 2 == 1
        self.width = sum(widths)
        self.header = header
        self.footer = footer
        self.style = style
        #
        self.props_table = {'font': font,
                            'halign': 'left',
                            'text_col': (0, 0, 0),
                            'cell_pad': {'left': 5,
                                         'right': 5,
                                         'top' : 1,
                                         'bottom' : 1}
                            }
        self.props_column = {}
        self.props_line = {}
        self.props_cell = {}

    def _prop(self, name, line_i, cell_i):
        if name == 'cell_pad':
            return Padding(self, line_i, cell_i)
        try:
            return self.props_cell[(name,line_i, cell_i)]
        except KeyError:
            try:
                return self.props_line[(name,line_i)]
            except KeyError:
                try:
                    return self.props_column[(name, cell_i)]
                except KeyError:
                        return self.props_table[name]

    def set_line_prop(self, name, line_i, val):
        self.props_line[(name, line_i)] = val

    def set_column_prop(self, name, cell_i, val):
        self.props_column[(name, cell_i)] = val

    def set_cell_prop(self, name, line_i, cell_i, val):
        self.props_cell[(name, line_i, cell_i)] = val

    def set_table_prop(self, name, val):
        self.props_table[name] = val

    def add_line(self, line):
        # line item is (str, font)
        self.lines.append(line)
        return len(self.lines) - 1

    def render(self, x, y, canvas):
        self.y = 0.0
        self.canvas = canvas
        self.canvas.state_save()
        self.canvas.color_space("fs", jagpdf.CS_DEVICE_RGB)
        self.canvas.translate(x, y)
        body = len(self.lines) - self.footer
        for line_i in range(len(self.lines)):
            self.line_height = 0.0
            # determine height of  this line
            for cell_i in range(len(self.lines[line_i])):
                cell_font = self._prop("font", line_i, cell_i)
                cell_pad = self._prop("cell_pad", line_i, cell_i)
                vert_padding = cell_pad["top"] + cell_pad["bottom"]
                this_cell_height = cell_font.height() + vert_padding
                self.line_height = max(self.line_height, this_cell_height)
            row_area = (0, self.y, self.width, -self.line_height)
            canvas.state_save()
            if line_i < self.header:
                self.style.header_row_bg(canvas, *row_area)
            elif line_i < body:
                # draw zebra
                if self.zebra_pred(line_i - self.header):
                    self.style.zebra_row_bg(canvas, *row_area)
            else: #footer
                self.style.footer_row_bg(canvas, *row_area)
            canvas.state_restore()
            self.x = 0.0
            self.render_line(line_i)
            self.y -= self.line_height
        self.canvas.state_restore()

    def render_line(self, line_i):
        for cell_i in range(len(self.lines[line_i])):
            self.render_cell(line_i, cell_i)

    def render_cell(self, line_i, cell_i):
        text = str(self.lines[line_i][cell_i])
        width = self.widths[cell_i]
        if text:
            font = self._prop("font", line_i, cell_i)
            halign = self._prop("halign", line_i, cell_i)
            pad = self._prop("cell_pad", line_i, cell_i)
            y = self.y - self.line_height + pad['bottom'] - font.descender()
            if halign == "left":
                x = self.x + pad["left"]
            elif halign == "right":
                rpad = pad["right"]
                x = self.x + width - (font.advance(text) + rpad)
            elif halign == "center":
                # simplified, does not consider padding
                text_w = font.advance(text)
                x = self.x + (width - text_w) / 2.0
            self.canvas.text_font(font)
            self.canvas.color("fs", *self._prop("text_col", line_i, cell_i))
            self.canvas.text(x, y, text)
        self.x += width


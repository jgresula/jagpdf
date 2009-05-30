#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


"""Implements limited SVG parser.

Notes:

 - paths only
 - arc is disabled (makes tux.svg ugly - filled with gradient fill)

Functions:

paint_to_canvas()

"""
import jagpdf
import re
import arc
import xml.sax
from xml.sax.handler import ContentHandler


def paint_to_canvas(canvas, svgfile):
    """Parses given SVG file to the canvas"""
    xml.sax.parse(svgfile, SVGHandler(canvas))

# path regular expressions
gwsp = '\s'
gpath_comma_wsp = '\s+,?\s*|,\s*'
gpath_cmd = '[MCzA]'
gpath_number = '[+-]?[0-9]+(?:\.[0-9]+)?'
gpath_flag = '[01]'
gpath_non_neg_number = '[0-9]+(?:\.[0-9]+)?'

class Parser:
    re_cache = {}
    action = {gpath_number: float,
              gpath_non_neg_number: float,
              gpath_flag: int}

    def __init__(self, data, handler):
        self.data = data
        self.pos = 0
        self.args = []
        self.handler = handler
        self.first_point = []
        self.last_point = []

    def parse_path_command(self):
        self.match(gwsp, '*')
        self.args = []
        cmd = self.match(gpath_cmd).group(0)
        self.match(gwsp, '*')
        if cmd == 'M':
            self.sequence(self.coordinate_pair)
            self.handler.move_to(*self.args)
            self.first_point = self.args[-2:]
            self.last_point = self.first_point
        elif cmd == 'z':
            self.handler.path_close()
            self.last_point = self.first_point
        elif cmd == 'C':
            self.sequence(self.curveto_argument)
            self.handler.bezier_to(*self.args)
            self.last_point = self.args[-2:]
        elif cmd == 'A':
            self.sequence(self.arc_argument)
            approx = arc.arc_to_bezier_endpoints(self.last_point[0], self.last_point[1], *self.args)
#             for cmd, args in approx:
#                 getattr(self.handler, cmd)(*args)
            self.last_point = self.args[-2:]
        else:
            raise RuntimeError("Unknown path command: '%s'" % cmd)
        self.match(gwsp, '*')

    def sequence(self, op):
        op()
        while 1:
            pos = self.pos
            self.match(gpath_comma_wsp, '?')
            try:
                op()
            except RuntimeError:
                self.pos = pos
                break

    def parse_path(self):
        while self.pos < len(self.data):
            self.parse_path_command()

    def compiled_re(self, rex, mod):
        try:
            compiled = Parser.re_cache[(mod, rex)]
        except KeyError:
            compiled = re.compile(mod and "(?:%s)%s" % (rex,mod) or rex)
            Parser.re_cache[(mod, rex)] = compiled
        return compiled

    def coordinate_pair(self):
        self.match(gpath_number)
        self.match(gpath_comma_wsp, '?')
        self.match(gpath_number)

    def curveto_argument(self):
        self.coordinate_pair()
        self.match(gpath_comma_wsp, '?')
        self.coordinate_pair()
        self.match(gpath_comma_wsp, '?')
        self.coordinate_pair()

    def arc_argument(self):
        self.match(gpath_non_neg_number)
        self.match(gpath_comma_wsp, '?')
        self.match(gpath_non_neg_number)
        self.match(gpath_comma_wsp, '?')
        self.match(gpath_number)
        self.match(gpath_comma_wsp)
        self.match(gpath_flag)
        self.match(gpath_comma_wsp)
        self.match(gpath_flag)
        self.match(gpath_comma_wsp)
        self.coordinate_pair()

    def match(self, rex, mod=''):
        rec = self.compiled_re(rex, mod)
        m = re.match(rec, self.data[self.pos:])
        if not m:
            raise RuntimeError('Syntax Error, offset %d:\n%s' % (self.pos, self.data[self.pos:self.pos+75]))
        try:
            self.args.append(Parser.action[rex](m.group(0)))
        except KeyError:
            pass
        self.pos += m.end()
        return m




rex_color = re.compile('#([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})')
class SVGHandler(ContentHandler):
    def __init__(self, canvas):
        self.canvas = canvas
        self.ignore = False
        self.style = dict(fill_color='#000000',
                          stroke_color='#000000')

    def startDocument(self):
        self.canvas.state_save()
        self.canvas.color_space('fs', jagpdf.CS_DEVICE_RGB)

    def endDocument(self):
        self.canvas.state_restore()

    def startElement(self, name, attrs):
        if name == 'defs':
            self.ignore = True
        if not self.ignore:
            if name == 'path':
                self.paint_path(attrs)

    def endElement(self, name):
        if name == 'defs':
            self.ignore = False

    def characters(self, ch):
        pass

    def paint_path(self, attrs):
        style = [item.strip() for item in attrs['style'].split(';')]
        style = dict([opt.split(':') for opt in style])
        op = self.set_color(style)
        p = Parser(attrs['d'], self.canvas)
        p.parse_path()
        self.canvas.path_paint(op)

    def set_color(self, style):
        operations = ''
        for op in ['stroke', 'fill']:
            if style.has_key(op):
                color = style[op]
                if color == 'none':
                    continue
                elif color.startswith('url'):
                    color = '#673300'
                if color != self.style[op+'_color']:
                    self.style[op+'_color'] = color
                    m = rex_color.match(color)
                    self.canvas.color(op[0], *[int(s,16)/255.0 for s in m.group(1,2,3)])
                operations = operations + op[0]
        return operations


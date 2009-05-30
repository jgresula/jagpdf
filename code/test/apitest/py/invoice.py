#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import sys
import os
import jag.testlib as testlib
import jag.textfmt as textfmt
from jag.table import SimpleTable

data="""207703579 01.01.2009 13:40:24 407 1
778734502 01.01.2009 17:08:06 72 0
778734502 02.01.2009 07:23:10 500 1
282029923 03.01.2009 08:10:04 940 0
556466921 03.01.2009 09:38:05 768 1
211180263 03.01.2009 21:17:18 869 0
663234547 04.01.2009 09:46:40 716 0
698372074 05.01.2009 20:56:53 832 1
706942829 05.01.2009 21:05:45 764 1
556466921 07.01.2009 15:23:25 851 0
405772839 07.01.2009 18:10:56 192 0
556466921 08.01.2009 14:15:23 90 1
211180263 08.01.2009 20:12:10 405 1
570515251 09.01.2009 13:35:10 1144 0
778734502 10.01.2009 15:08:09 527 0
698372074 11.01.2009 08:43:22 643 0
215010065 11.01.2009 10:23:51 222 1
692318989 11.01.2009 17:04:45 1116 0
556466921 12.01.2009 13:20:23 721 0
698372074 12.01.2009 17:22:07 1191 1
778734502 12.01.2009 19:31:12 197 1
706942829 14.01.2009 17:01:27 1194 1
282029923 15.01.2009 08:55:55 182 0
714219605 15.01.2009 13:33:06 824 1
706942829 16.01.2009 08:31:42 965 0
692318989 16.01.2009 09:38:05 1107 0
714219605 16.01.2009 21:02:36 1122 1
570515251 17.01.2009 14:46:41 938 1
207703579 18.01.2009 11:09:29 777 1
556466921 20.01.2009 08:09:12 1148 0
663234547 20.01.2009 11:48:46 450 1
663234547 24.01.2009 08:06:00 135 1
215010065 28.01.2009 13:11:26 620 0
215010065 30.01.2009 19:57:36 905 1"""

billing_addres="""$1Paul de Hůlka
$0K Chudobě 871
288 03 Nymburk

Customer ID: 4399-32002-89
Phone Number: 412 945 330"""

metelco_addres="""$1MeTelco Czech Republic a.s.
$0U Blahobytu 1214
111 50 Praha 1

IČ: 812 33 291
DIČ: CZ93838401"""

billing_info="""$1Invoice Period: 1.1.2009 - 31.1.2009
$0Date of Invoice: 4.2.2009
Account Number: 233048932/3325


To be paid within 14 days of invoice date.
"""


class Bunch:
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

    def add(self, **kwds):
        self.__dict__.update(kwds)
        return self


class TableStyle:
    def __init__(self, doc):
        self.doc = doc
        self.zebra = None
        # blue
        self.col_bright = 0.56470588235294117, 0.72549019607843135, 0.85882352941176465
        self.col_mid = 0.094117647058823528, 0.36078431372549019, 0.58039215686274515
        self.col_dark = 0.015686274509803921, 0.1803921568627451, 0.31372549019607843

    def _zebra(self, s, e):
        if not self.zebra:
            col = "%.3f, %.3f, %.3f" % self.col_bright
            gr_spec = "domain=0, 1; c0=%s; c1=1, 1, 1" % col
            gradient = self.doc.function_2_load(gr_spec)
            sh_spec = "axial; coords=%.f, 0, %.f, 0" % (s, e)
            self.zebra = self.doc.shading_pattern_load(sh_spec,
                                                       jagpdf.CS_DEVICE_RGB,
                                                       gradient)
        return self.zebra

    def header_row_bg(self, canvas, *rect):
        canvas.color("f", *self.col_mid)
        canvas.color("s", *self.col_dark)
        canvas.rectangle(*rect)
        canvas.path_paint("f")

    def footer_row_bg(self, canvas, *rect):
        self.header_row_bg(canvas, *rect)

    def zebra_row_bg(self, canvas, *rect):
        canvas.rectangle(*rect)
        canvas.path_paint("w")
        canvas.shading_apply(self._zebra(rect[0], rect[2]))


def record_iter():
    price_per_sec = 0.02
    total_sec = 0.0
    total_price = 0.0
    total_wire = 0
    for rec in data.split('\n'):
        fields = rec.split()
        sec = int(fields[-2])
        wiretap = int(fields[-1])
        wiretap_str = wiretap and "x" or ""
        sec_str = "%2d:%02d" % (sec/60, sec%60)
        price = sec * price_per_sec
        price_str = "%.02f" % price
        nr = fields[0]
        fields[0] = " ".join((nr[:3], nr[3:6], nr[6:]))
        result = fields[:-2]
        result.append(sec_str)
        result.append(wiretap_str)
        result.append(price_str)
        if wiretap:
            total_wire += 1
        total_sec += sec
        total_price += price
        yield result, Bunch(sec=total_sec, price=total_price, nr_wire=total_wire)

def write_block(canvas, x, y, block, *fonts):
    canvas.text_start(x, y)
    font = fonts[0]
    canvas.text_font(font)
    for line in unicode(block, 'utf-8').encode("windows-1250").split('\n'):
        line = line.strip()
        if line:
            if line[0] == '$':
                font = fonts[int(line[1])]
                canvas.text_font(font)
                line = line[2:]
            canvas.text(line)
        canvas.text_translate_line(0, -font.height())
    canvas.text_end()



def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'invoice.pdf', jagpdf.create_profile())
    table_fnt = doc.font_load("standard; name=Helvetica; size=10; enc=windows-1250")
    table_fnt_bold = doc.font_load("standard; name=Helvetica-Bold; size=10; enc=windows-1250")
    header_fnt = doc.font_load("standard; name=Helvetica-Bold; size=10; enc=windows-1250")
    invoice_fnt = doc.font_load("standard; name=Helvetica-Bold; size=16; enc=windows-1250")
    offer_fnt = doc.font_load("standard; name=Helvetica-Bold; size=12; enc=windows-1250")
    table_title_fnt = doc.font_load("standard; name=Helvetica-Bold; size=14; enc=windows-1250")
    dingbats_fnt = doc.font_load("standard; name=ZapfDingbats; size=10")
    dingbats_fnt = header_fnt

    doc.page_start(597.6, 848.68)
    canvas = doc.page().canvas()
    canvas.color_space("fs", jagpdf.CS_DEVICE_RGB)
    #
    widths = [110, 90, 80, 80, 40, 80]
    style = TableStyle(doc)
    tbl = SimpleTable(doc, widths, font = table_fnt, style = style, header=1, footer=1)
    tbl.set_table_prop("halign", "center")

    # header
    tbl.add_line(('Number', 'Date', 'Time', 'Duration', 'Wiretap', 'Price'))
    tbl.set_line_prop("halign", 0, "center")
    tbl.set_line_prop("font", 0, header_fnt)
    tbl.set_line_prop("text_col", 0, (1, 1, 1))
    tbl.set_line_prop("cell_pad", 0, {'top': 4, 'bottom': 4})
    # body - add records and formatting style
    for rec, inf in record_iter():
        tbl.add_line(rec)
    tbl.set_column_prop("halign", 5, "right")
    tbl.set_column_prop("cell_pad", 5, {'right': 25})
    tbl.set_column_prop("halign", 4, "center")
    tbl.set_column_prop("text_col", 4, (0, .525, 0.086))
    tbl.set_column_prop("font", 4, dingbats_fnt)
    tbl.set_column_prop("halign", 3, "right")
    tbl.set_column_prop("cell_pad", 3, {'right': 25})
    tbl.set_column_prop("cell_pad", 0, {'left': 10})
    # footer
    dur = "%02d:%02d:%02d" % (inf.sec/3600, (inf.sec%3600)/60, inf.sec%60)
    price_str = "\x80 %.2f" % inf.price
    footer_id = tbl.add_line(('Total', '', '', dur, str(inf.nr_wire), price_str))
    tbl.set_line_prop("font", footer_id, header_fnt)
    tbl.set_line_prop("text_col", footer_id, (1, 1, 1))
    tbl.set_line_prop("cell_pad", footer_id, {'top': 4, 'bottom': 4})
    # render table
    tbl_width = sum(widths)
    tbl_x = (597.6 - tbl_width) / 2
    tbl.render(tbl_x, 670, canvas)
    #
    # render rest
    #
    # add customer, company, ...
    write_block(canvas, tbl_x, 765, billing_addres, table_fnt, table_fnt_bold)
    write_block(canvas, 400, 765, metelco_addres, table_fnt, table_fnt_bold)
    write_block(canvas, 205, 765, billing_info, table_fnt, table_fnt_bold)
    # invoice top
    png = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/metelco.png')
    logo = doc.image_load_file(png)
    canvas.image(logo, 390, 785)
    canvas.color("fs", *style.col_dark)
    canvas.text_font(table_title_fnt)
    canvas.text(260, 678, "Call Details")
    canvas.text_font(invoice_fnt)
    canvas.text(tbl_x, 790, "Invoice #3922094839")
    canvas.move_to(tbl_x, 780)
    canvas.line_to(tbl_x + tbl_width, 780)
    canvas.move_to(tbl_x, 695)
    canvas.line_to(tbl_x + tbl_width, 695)
    canvas.move_to(197, 775)
    canvas.line_to(197, 700)
    canvas.move_to(392, 775)
    canvas.line_to(392, 700)
    canvas.path_paint("s")
    # special offer
    jpg = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/oldphone.jpg')
    phone = doc.image_load_file(jpg)
    canvas.image(phone, tbl_x, 25)
    special_offer(doc, canvas, offer_fnt, tbl_x + 30, 110, 3.14/8)
    canvas.color('f', 0, 0, 0)
    rect = tbl_x + 130, 50, 340, 70
    textfmt.format_text(rect, ad, doc, table_fnt, align='justify', para_spacing=0.5)
    #
    doc.page_end()
    doc.finalize()

ad="""Mr. de H\xf9lka, we have a fantastic offer for you. Buy this classy phone \
for only \x80299.99 and get whole 25 minutes for free!
This phone is packed with an advanced technology which plays an important role \
in the user's life. After buying this phone the users find their work more easy \
or they feel more comfortable while working with this phone. This phone is \
available in two colours, silver and black with solid casing.
"""

def special_offer(doc, canvas, font, cx, cy, angle):
    canvas.state_save()
    canvas.translate(cx, cy)
    canvas.rotate(angle)
    canvas.translate(-cx, -cy)
    text_w = font.advance("Special Offer")
    canvas.color("f", 1.0, 0.32549019607843138, 0.0)
    canvas.arc(cx, cy, (text_w + 10)/2, 14, 0, 6.28)
    canvas.path_paint("f")
    canvas.color("f", 1, 1, 1)
    canvas.text_font(font)
    canvas.text(cx - text_w / 2, cy + 2 - (font.ascender() + font.descender()), "Special Offer")
    canvas.state_restore()


def generate_telecom_data():
    import datetime
    import random
    num_numbers = 15
    numbers = [random.randint(200000000,780999999) for i in range(num_numbers)]
    it = datetime.datetime(2009, 1, 1, 7, 0, 0)
    end_date = datetime.datetime(2009, 1, 31, 23, 59, 59)
    while it < end_date:
        calls_per_day = random.randint(0, 3)
        durations = [random.randint(10, 1200) for i in range(calls_per_day)]
        offsets = [random.randint(600, 15*60*60) for i in range(calls_per_day)]
        offsets.sort()
        for off, d in zip(offsets, durations):
            duration = datetime.timedelta(0, d)
            start = it + datetime.timedelta(0, off)
            end = start + duration
            print numbers[random.randint(0, num_numbers-1)], \
                  start.strftime('%d.%m.%Y'), \
                  start.strftime('%H:%M:%S'), \
                  d, \
                  random.random() > 0.6 and 1 or 0
        it += datetime.timedelta(1)




if __name__ == '__main__':
    #generate_telecom_data()
    test_main()












#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import sys
import os
import jag.testlib as testlib

g_font = testlib.EasyFontTTF()

#raw_input("attach")


def do_some(doc):
    doc.page_start(5.9*72, 5.9*72)
    page=doc.page().canvas()
    page.color("fs", 0.75)
    page.rectangle(20, 20, 72, 72)
    page.path_paint('s')
    page.text_font(g_font(8))
    page.text(25, 57, "a gray rectangle")
    page.text(25, 45, "www.boost.org")
    doc.page().annotation_uri(20, 20, 72, 72, "http://www.boost.org")
    doc.page_end()



def do_goto_each_other(doc):
    def page_vis(this,other):
        page=doc.page().canvas()
        page.text_font(g_font(8))
        page.color("fs", 0.75)
        page.rectangle(20, 20, 72, 72)
        page.path_paint('s')
        page.text(25, 57, "This is Page *%d*" % this)
        page.text(25, 45, "Click for page %d" % other)
        return page

    dest_1st = doc.destination_reserve()
    page_nr = doc.page_number()
    doc.page_start(5.9*72, 5.9*72)
    dest_1st_direct = doc.destination_define("mode=XYZ") #should get page automatically
    dest_2nd = doc.destination_reserve()

    page = page_vis(page_nr+1, page_nr+2)
    doc.page().annotation_goto(20, 20, 72, 72, dest_2nd)
    doc.page_end()

    doc.page_start(5.9*72, 5.9*72)
    page = page_vis(page_nr+2, page_nr+1)
    doc.page().annotation_goto(20, 20, 72, 72, dest_1st)
    doc.page_end()

    doc.page_start(5.9*72, 5.9*72)
    page = page_vis(page_nr+3, page_nr+1)
    doc.page().annotation_goto(20, 20, 72, 72, dest_1st_direct)
    doc.page_end()

    doc.destination_define_reserved(dest_1st, "page=%d;mode=XYZ" % page_nr)
    doc.destination_define_reserved(dest_2nd, "page=%d;mode=XYZ" % (page_nr+1))


def do_outline_and_goto(doc):
    def make_goto(x, y, dest, txt):
        page.color("fs", 0.75)
        page.rectangle(x, y, 36, 36)
        page.path_paint('s')
        page.text(x+10, y+10, txt)
        doc.page().annotation_goto(x, y, 36, 36, dest)

    page_nr = doc.page_number()
    p25 = doc.destination_define("mode=XYZ;zoom=0.25")
    doc.page_start(5.9*72, 5.9*72)
    page = doc.page().canvas()
    p50 = doc.destination_define("mode=XYZ;zoom=0.50")
    p75 = doc.destination_reserve()
    doc.outline().item("Page %d" % (page_nr+1))
    doc.outline().level_down()
    doc.outline().item("zoom 25%", p25)
    doc.outline().item("zoom 50%", p50)
    doc.outline().item("zoom 75%", p75)
    doc.outline().level_up()
    page.text_font(g_font(8))
    make_goto(20, 20, p25, "25%")
    make_goto(20, 60, p50, "50%")
    make_goto(20, 100, p75, "75%")
    doc.page_end()
    doc.destination_define_reserved(p75, "page=%d;mode=XYZ;zoom=0.75" % page_nr)


def do_goto_single_page(doc):
    pheight = 5.9*72
    doc.page_start(5.9*72, pheight)
    page = doc.page().canvas()
    page.color("fs", 0.75)
    page.rectangle(20, 20, 72, 72)
    page.rectangle(20, pheight-92, 72, 72)
    page.path_paint('s')
    page.text_font(g_font(8))
    page.text(25, 57, "click to zoom")
    page.text(25, 45, "to 200%")
    page.text(25, pheight-47, "click to fit height")
    doc.page().annotation_goto(20, pheight-92, 72, 72, "page=%d;mode=FitV" % doc.page_number())
    doc.page().annotation_goto(20, 20, 72, 72, "page=%d;mode=XYZ;left=0;top=%lf;zoom=2" % (doc.page_number(),pheight))
    doc.page_end()


def test_main(argv=None):
    cfg = testlib.test_config()
    #cfg.set("doc.trace_level", "5")
    doc = testlib.create_test_doc(argv, 'annots.pdf', cfg)
    g_font.set_writer(doc)
    do_some(doc)
    do_goto_single_page(doc)
    do_goto_each_other(doc)
    do_outline_and_goto(doc)

    doc.finalize()


if __name__ == "__main__":
    test_main()



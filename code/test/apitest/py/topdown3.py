#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#
import jagpdf
import jag.testlib as testlib

def do_file(argv, name, profile=None):
    doc = testlib.create_test_doc(argv, name, profile)
    outline = doc.outline()
    
    doc.page_start(600, 800)
    outline.item("first page")
    outline.item("rect", 'mode=XYZ; left=40; top=40')
    canvas = doc.page().canvas()
    canvas.text(20, 20, '1st')
    canvas.rectangle(40, 40, 50, 50)
    canvas.rectangle(100, 40, 50, 50)
    canvas.path_paint("s")
    doc.page().annotation_uri(40, 40, 50, 50, 'http://jagpdf.org')
    doc.page().annotation_goto(100, 40, 50, 50, 'page=1; mode=XYZ; left=40; top=40')
    doc.page_end()
    
    doc.page_start(600, 800)
    outline.item("second page")
    canvas = doc.page().canvas()
    canvas.text(20, 20, '2nd')
    canvas.rectangle(40, 40, 50, 50)
    canvas.path_paint("s")
    doc.page_end()

    doc.page_start(600, 800)
    doc.page_end()

    doc.finalize()


def test_main(argv=None):
    profile = testlib.test_config()
    profile.set('doc.topdown', "1")
    do_file(argv, "topdown3.pdf", profile)

if __name__ == "__main__":
    test_main()


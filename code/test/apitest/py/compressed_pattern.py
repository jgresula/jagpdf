#!/usr/bin/env python

# Copyright (c) 2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file 
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

import os
import jagpdf
import jag.testlib as testlib

# reproduces #130
# tiling pattern w/FlateDecode in the top-down mode produces a corrupted content stream

def image_path(name):
    return os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/') + name

def test_main(argv=None):
    profile = jagpdf.create_profile()
    profile.set("doc.topdown", "1")
    #profile.set("doc.compressed", "0")
    doc = testlib.create_test_doc(argv, 'compressed_pattern.pdf', profile)
    img = doc.image_load_file(image_path('logo.png'))

    p = doc.canvas_create()
    p.image(img, 0, 0)
    pattern = doc.tiling_pattern_load("step=109, 32", p)

    
    doc.page_start(8.3 * 72.0, 11.7 * 72.0)
    canvas = doc.page().canvas()
    canvas.color_space_pattern('f')
    canvas.pattern('f', pattern)
    canvas.rectangle(0, 0, 200, 200)
    canvas.rectangle(200, 200, 200, 200)
    canvas.path_paint('f')
    doc.page_end()

    doc.page_start(400, 400)
    canvas = doc.page().canvas()
    canvas.color_space_pattern('f')
    canvas.pattern('f', pattern)
    canvas.rectangle(0, 0, 200, 200)
    canvas.path_paint('f')
    doc.page_end()

    doc.finalize()

if __name__ == '__main__':
    test_main()


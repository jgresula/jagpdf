#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import os
import math
import jagpdf
import jag.testlib as testlib

def test_main(argv=None):
    cfg = jagpdf.create_profile()
    cfg.set('doc.compressed', '1')
#    cfg.set("doc.viewer_preferences", "FitWindow; DisplayDocTitle")
    cfg.set("doc.initial_destination", "mode=XYZ; zoom=1.0")
    
    doc = testlib.create_test_doc(argv, 'imageclip.pdf', cfg)
    img_file = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/mandrill.jpg')
    img = doc.image_load_file(img_file, jagpdf.IMAGE_FORMAT_JPEG)
    media = 500, 500
    doc.page_start(*media)
    canvas = doc.page().canvas()
    center = [c/2.0 for c in media]
    n = 14
    canvas.color('s', 0.8)
    canvas.state_save()
    canvas.circle(center[0], center[1], 210)
    canvas.circle(center[0], center[1], 120)
    canvas.path_paint('Ws')
    for i in range(n + 1):
        canvas.translate(*center)
        canvas.rotate(2.0 * math.pi / n)
        canvas.translate(-center[0], -center[1])
        canvas.state_save()
        canvas.translate(80, 80)
        canvas.scale(0.35, 0.35)
        canvas.image(img, 0, 0)
        canvas.state_restore()
    canvas.state_restore()
    center_scale = .65
    img_width = center_scale * img.width() / img.dpi_x() * 72
    img_height = center_scale * img.height() / img.dpi_y() * 72
    canvas.state_save()
    canvas.translate(center[0]-img_width/2.0, center[1]-img_height/2.0)
    canvas.scale(center_scale, center_scale)
    canvas.image(img, 0, 0)
    canvas.state_restore()
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()


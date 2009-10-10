#!/usr/bin/env python

# Copyright (c) 2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file 
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

import os
import jagpdf
import jag.testlib as testlib

# The resulting file has 'dark' colors in Reader; other viewers are ok. It seems
# that the reason is the image logo.png - it contains an alpha channel which is
# transformed to smask.

def image_path(name):
    return os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/') + name

def test_main(argv=None):
    profile = jagpdf.create_profile()
    profile.set("doc.topdown", "1")
    profile.set("doc.compressed", "0")
    doc = testlib.create_test_doc(argv, 'dark_reader.pdf', profile)
    img = doc.image_load_file(image_path('logo.png'))
    doc.page_start(8.3 * 72.0, 11.7 * 72.0)
    canvas = doc.page().canvas()
    
    # 1 0 0 -1 0 842.4 cm 
    # 1 0 0 1 14.4 14.4 cm
    canvas.translate(14.4, 14.4)
    # 0.24 0 0 0.24 0 0 cm 
    canvas.scale(0.24, 0.24)
    # /DeviceRGB CS 
    # /DeviceRGB cs
    canvas.color_space('fs', jagpdf.CS_DEVICE_RGB)
    # q
    canvas.state_save()
    #  0.79987 0 0 0.79987 0 0 cm
    canvas.scale(0.79987, 0.79987)
    
    #   0.27451 0.44314 0.83529 sc
    canvas.color('f', 0.27451, 0.44314, 0.83529)
    #   0 0 2963 159 re
    canvas.rectangle(0, 0, 2963, 159)
    #   f
    canvas.path_paint('f')
    
    #  q
    canvas.state_save()
    #   3.125 0 0 3.06667 0 159 cm
    canvas.transform(3.125, 0, 0, 3.06667, 0, 159)
    #   q
    #    80 0 0 -15 0 15 cm 
    #    /im1 Do
    #   Q
    #  Q
    canvas.image(img, 0, 0)
    canvas.state_restore()
    # Q
    canvas.state_restore()

    doc.page_end()
    doc.finalize()

if __name__ == '__main__':
    test_main()


#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jagpdf
import jag.testlib as testlib

paper = 597.6, 848.68

def image_path(name):
    return os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/') + name

def test_main(argv=None):
    doc = testlib.create_test_doc(argv, 'autodetectimg.pdf')
    imgs = []
    imgs.append(doc.image_load_file(image_path('lena.jpg'), jagpdf.IMAGE_FORMAT_JPEG))
    imgs.append(doc.image_load_file(image_path('logo.png'), jagpdf.IMAGE_FORMAT_PNG))
    imgs.append(doc.image_load_file(image_path('lena.jpg'), jagpdf.IMAGE_FORMAT_AUTO))
    imgs.append(doc.image_load_file(image_path('logo.png'), jagpdf.IMAGE_FORMAT_AUTO))
    imgs.append(doc.image_load_file(image_path('lena.jpg')))
    imgs.append(doc.image_load_file(image_path('logo.png')))
    # tbd ~ imgspec
    spec1 = doc.image_definition()
    spec1.file_name(image_path('lena.jpg'))
    spec2 = doc.image_definition()
    spec2.file_name(image_path('logo.png'))
    imgs.append(doc.image_load(spec1))
    imgs.append(doc.image_load(spec2))
    doc.page_start(*paper)
    x, y = 100, 50
    o = 0
    for img in imgs:
        doc.page().canvas().image(img, x + o, y + o)
        o += 30
    doc.page_end()
    doc.finalize()

if __name__ == '__main__':
    test_main()

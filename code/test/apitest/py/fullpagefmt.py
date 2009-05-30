#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jagpdf
import jag.testlib as testlib
import jag.textfmt as textfmt

def test_main(argv=None):
    #http://www.lorem-ipsum.info/generator3
    def do_page(font, txt):
        media = 597.6, 848.68
        margin = 20
        rect = margin, margin, media[0]-2*margin, media[1]-2*margin
        doc.page_start(*media)
        canvas = doc.page().canvas()
        canvas.color('s', 0.9)
        canvas.rectangle(*rect)
        canvas.path_paint('s')
        textfmt.format_text(rect, txt, doc, font, align='justify', para_spacing=0.5)
        doc.page_end()
    dejavu = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/fonts/DejaVuSans.ttf')
    textdir = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/text/')
    cfg = jagpdf.create_profile()
    cfg.set('doc.compressed', '1')
    doc = testlib.create_test_doc(argv, 'justified.pdf', cfg)
    font_1252 = doc.font_load('file=%s;size=8' % dejavu) # western
    font_1251 = doc.font_load('file=%s;size=8;enc=windows-1251' % dejavu) # cyrillic
    font_1253 = doc.font_load('file=%s;size=8;enc=windows-1253' % dejavu) # greek
    for font, fname, enc in [[font_1252, 'lipsum.txt', 'windows-1252'],
                             [font_1251, 'russian-lipsum.txt', 'windows-1251'],
                             [font_1253, 'greek-lipsum.txt', 'windows-1253']]:
        text_file = os.path.join(textdir, fname)
        do_page(font, unicode(open(text_file).read(), 'utf-8').encode(enc))
    doc.finalize()

if __name__ == "__main__":
    test_main()


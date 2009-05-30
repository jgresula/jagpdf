# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import os
import jag.testlib as testlib

def test_main(argv=None):
    writer = testlib.create_test_doc(argv, "font-types.pdf")
    writer.page_start(3*72, 6*72)
    page = writer.page().canvas()

    size = 12
    enc = "windows-1252"
    y = 30
    for fnt in ['DejaVuSans.ttf', 'DejaVuSansMono.ttf', 'Juvelo.otf',\
                 'analecta.otf', 'Inconsolata.otf']:
        fstr = 'enc=%s; size=%d; file=${JAG_TEST_RESOURCES_DIR}/fonts/%s'
        fspec = os.path.expandvars( fstr % (enc,size,fnt))
        fontid = writer.font_load(fspec)
        page.text_font(fontid)
        page.text(16, y, "Text in various faces")
        y += 20
    writer.page_end()
    writer.finalize()

if __name__ == '__main__':
    test_main()


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

def test_main(argv=None):
    cfg = testlib.test_config()
    cfg.set("fonts.embedded", "1")
    cfg.set("fonts.subset", "1")
    doc = testlib.create_test_doc(argv, 'ttfmonosubset.pdf', cfg)
    spec = 'enc=windows-1252; size=12; file=${JAG_TEST_RESOURCES_DIR}/fonts/DejaVuSansMono.ttf'
    fspec = os.path.expandvars(spec)
    fontid = doc.font_load(fspec)
    doc.page_start(3.5*72, 3.5*72)
    writer = doc.page().canvas()
    writer.text_font(fontid)
    writer.text(20, 20, "Monospaced font.")
    doc.page_end()
    doc.finalize()


if __name__ == "__main__":
    test_main()

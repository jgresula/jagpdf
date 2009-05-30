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

#raw_input('aaaa')

horse = 'P\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88 \xc3\xbap\xc4\x9bl \xc4\x8f\xc3\xa1belsk\xc3\xa9 \xc3\xb3dy.'

def do_doc(argv, outf, cfg, msg, fnt='Juvelo.otf'):
    doc = testlib.create_test_doc(argv, outf, cfg)
    doc.page_start(3.5*72, 3.5*72)
    msgs = [(msg,'windows-1252'),
            (horse + ' (utf-8)','UTF-8'),
            (unicode(horse, 'utf8').encode('cp1250') + ' (cp1250)', 'windows-1250')]
    y = 20
    for txt, enc in msgs:
        specstr = 'enc=%s; size=10; file=${JAG_TEST_RESOURCES_DIR}/fonts/%s' % (enc,fnt)
        fspec = os.path.expandvars(specstr)
        fontid = doc.font_load(fspec)
        writer = doc.page().canvas()
        writer.text_font(fontid)
        writer.text(20, y, txt)
        y += 20
    doc.page_end()
    doc.finalize()

def test_main(argv=None):
    cfg = testlib.test_config()
    cfg.set("doc.version", "6")
    # CFF is extracted even in 1.6, see #98
    do_doc(argv, 'opentypecff16.pdf', cfg, "OpenType CFF.")

    cfg.set("doc.version", "5")
    do_doc(argv, 'opentypecff15.pdf', cfg, "CFF extracted from OpenType.")


if __name__ == "__main__":
    test_main()



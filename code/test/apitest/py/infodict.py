#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib

# note: the following utf-8 info dict entries are not displayed
# correctly in foxit, adobe is ok

def test_main(argv=None):
    cfg = testlib.test_config()
    cfg.set('info.title', 'title: ' + 'p\xc5\x99\xc3\xadli\xc5\xa1')
    cfg.set('info.author', 'author: ' + 'p\xc5\x99\xc3\xadli\xc5\xa1')
    cfg.set('info.subject', 'subject: ' + 'p\xc5\x99\xc3\xadli\xc5\xa1')
    cfg.set('info.keywords', 'keywords: ' + 'p\xc5\x99\xc3\xadli\xc5\xa1')
    cfg.set('info.creator', 'creator: ' + 'p\xc5\x99\xc3\xadli\xc5\xa1')
    doc = testlib.create_test_doc(argv, 'infodict.pdf', cfg)
    doc.page_start(400, 400)
    doc.page_end()
    doc.finalize()


if __name__ == "__main__":
    test_main()

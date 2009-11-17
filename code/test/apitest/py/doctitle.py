#!/usr/bin/env python

# Copyright (c) 2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file 
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

import jag.testlib as testlib

def test_main(argv=None):
    profile = testlib.test_config()
    profile.set("info.title", "This title should be overwritten.")
    profile.set("doc.viewer_preferences", "DisplayDocTitle")
    doc = testlib.create_test_doc(argv, 'doctitle.pdf', profile)
    doc.page_start(100, 100)
    doc.title("A new title.")
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()

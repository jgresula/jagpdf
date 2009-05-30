#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import sys
import os

def test_main(argv=None):
    if None==argv:
        argv = sys.argv
    out_file = os.path.abspath(os.path.join(argv[1], 'config.pdf'))
    cfg_file = os.path.abspath(os.path.join(argv[1], 'config.ini'))
    doc = jagpdf.create_stream(testlib.FileStreamOut(out_file))
    doc = None
    doc = jagpdf.create_file(out_file)
    doc = None
    cfg = jagpdf.create_profile()
    cfg.set("doc.compressed", "blue-foundation")
    cfg.save_to_file(out_file)
    cfg_str = open(out_file).read()
    cfg3 = jagpdf.create_profile_from_file(out_file)
    cfg2 = jagpdf.create_profile_from_string(cfg_str)
    cfg3.save_to_file(cfg_file)
    cfg3_str = open(cfg_file).read()
    cfg2.save_to_file(cfg_file)
    cfg2_str = open(cfg_file).read()
    os.unlink(out_file)
    os.unlink(cfg_file)
    assert cfg3_str == cfg2_str


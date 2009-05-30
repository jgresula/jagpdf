#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib

def do_document(argv, name, cfg_opts):
    cfg = testlib.test_config()
    for k, v in cfg_opts.iteritems():
        cfg.set(k, v)
    doc = testlib.create_test_doc(argv, name+'.pdf', cfg)
    doc.page_start(400, 400)
    canvas = doc.page().canvas()
    canvas.text_start(20, 380)
    l = [(k, v) for k, v in cfg_opts.iteritems()]
    l.sort()
    for k, v in l:
        canvas.text("%s : %s" % (k, v))
        canvas.text_translate_line(0, -15)
    canvas.text_end()
    doc.page_end()
    doc.finalize()


def test_main(argv=None):
    def doc(name, cfg={}, versions=('3','4')):
        cfg['doc.encryption'] = 'standard'
        cfg["doc.static_file_id"] = "1"
        cfg["info.creation_date"] = "0"
        cfg["info.static_producer"] = "1"
        for ver in versions:
            cfg['doc.version'] = ver
            do_document(argv, 'enc_' + name + '1' + ver, cfg)

    doc('usrpwd', {'stdsh.pwd_user': 'user'})
    doc('ownpwd', {'stdsh.pwd_owner': 'owner'})
    doc('no_restr')
    doc('no_print', {'stdsh.permissions': 'no_print'})
    doc('no_copy', {'stdsh.permissions': 'no_copy'})
    doc('no_hires_print', {'stdsh.permissions': 'no_hires_print'}, ['4'])
    doc('no_modify', {'stdsh.permissions': 'no_modify'})
    doc('no_assemble', {'stdsh.permissions': 'no_assemble'}, ['4'])
    doc('no_accessib', {'stdsh.permissions': 'no_copy',
                        'stdsh.pwd_owner': 'owner'}, ['4'])

if __name__ == "__main__":
    test_main()


#!/usr/bin/env python
#
# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib
import os

# do not test corefonts - correct core font test should always succeed


dejavu_ttf = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/fonts/DejaVuSans.ttf')

#
# attempts to register invalid specs
#
def test_suite_fail(doc, exc_substring):
    # invalid face name
    testlib.must_throw_ex(exc_substring[0],
                   doc.font_load,
                   'name=this-font-does-not-exist;size=10;enc=windows-1252')
    # invalid file
    fspec = os.path.expandvars(
        'size=10;file=${JAG_TEST_RESOURCES_DIR}/images/cc3399-icc.png')
    testlib.must_throw_ex(exc_substring[1],
                   doc.font_load,
                   fspec)
    # non-exisiting file
    testlib.must_throw_ex(exc_substring[2],
                   doc.font_load,
                   'size=10;file=/this/file/does/not/exist')


#
#
#
def test_suite_ok(doc, fullname):
    # invalid face name
    fid = doc.font_load('name=this-font-does-not-exist;size=10')
    # invalid file
    fspec = os.path.expandvars(
        'size=10;file=${JAG_TEST_RESOURCES_DIR}/images/cc3399-icc.png')
    fid2 = doc.font_load(fspec)
    assert fid.this == fid2.this
    fid3 = doc.font_load('standard;name=Helvetica;size=5')
    assert fid.this != fid3.this
    # non-exisiting file
    fid2 = doc.font_load('size=10;file=/this/file/does/not/exist')
    assert fid.this == fid2.this
    finfo = fid
    assert fullname == finfo.family_name()


def test_main(argv=None):
    #
    # typemanager with no default font set
    #
    cfg = jagpdf.create_profile()
    cfg.set('fonts.fallback', '')
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    test_suite_fail(doc, ['this-font-does-not-exist',
                    'FreeType call failed',
                    '/this/file/does/not/exist'])
    doc.finalize()
    #
    # typemanager with nonexisting default font
    #
    cfg = jagpdf.create_profile()
    cfg.set('fonts.fallback', 'file=nonexisting-default')
    doc = jagpdf.create_stream(testlib.NoopStreamOut(), cfg)
    default_typeface_failure = 'Default typeface not found'
    test_suite_fail(doc, [(default_typeface_failure, 'nonexisting-default', 'this-font-does-not-exist'),
                           (default_typeface_failure, 'nonexisting-default', 'FreeType call failed'),
                           (default_typeface_failure, 'nonexisting-default', '/this/file/does/not/exist')])
    doc.finalize()
    #
    #
    #
    default_fonts = [('file=' + dejavu_ttf, 'DejaVu Sans'),
                      ('standard;name=Helvetica', 'Helvetica'),
                     ]
    if testlib.is_windows():
        default_fonts.append(('name=Arial;enc=windows-1252', 'Arial'))
    for default_font, fullname in default_fonts:
        cfg = jagpdf.create_profile()
        cfg.set('fonts.fallback', default_font)
        doc = jagpdf.create_stream(testlib.NoopStreamOut(), cfg)
        test_suite_ok(doc, fullname)
        doc.finalize()


if __name__ == "__main__":
    test_main()

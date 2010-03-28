# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


import jagpdf
import jag.testlib as testlib
import os

def do(writer):
    writer.page_start(3*72, 1.5*72)
    page = writer.page().canvas()
    img = os.path.expandvars('${JAG_TEST_RESOURCES_DIR}/images/klenot.png')
    page.image(writer.image_load_file(img, jagpdf.IMAGE_FORMAT_PNG), 10, 30)
    writer.page_end()


def test_main(argv=None):
    profile = testlib.test_config()
    profile.set("doc.encryption", "standard")
    profile.set("doc.static_file_id", "1")
    profile.set("info.creation_date", "0")
    profile.set("info.static_producer", "1")
    doc = testlib.create_test_doc(argv, 'encrypted_indexed_cs.pdf', profile)
    do(doc)
    doc.finalize()

if __name__ == '__main__':
    test_main()

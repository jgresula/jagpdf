# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib

dim = 200, 200

# tests graphics state machine

def test_main(argv=None):
    doc = jagpdf.create_stream(testlib.NoopStreamOut())
    doc.page_start(*dim)
    canvas = doc.page().canvas()
    testlib.must_throw(canvas.line_to, 20, 20)
    testlib.must_throw(canvas.path_paint, 'f')
    canvas.text_start(0, 0)
    testlib.must_throw(canvas.text, 10, 10, "Not allowed here!")
    canvas.text("This is OK")
    canvas.text_end()
    doc.page_end()
    doc.finalize()

if __name__ == '__main__':
    test_main()

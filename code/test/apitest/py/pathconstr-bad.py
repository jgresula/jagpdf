# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf
import jag.testlib as testlib


def test_main(argv=None):
    doc = jagpdf.create_stream(testlib.NoopStreamOut(),
                                    jagpdf.create_profile())
    doc.page_start(5.9*72, 3.5*72)
    writer = doc.page().canvas()
    # the following operations must fail as they require path construction
    # to be already started
    testlib.must_throw(writer.bezier_to, 1, 2, 3, 4, 5, 6)
    testlib.must_throw(writer.bezier_to_1st_ctrlpt, 0, 0, 0, 0)
    testlib.must_throw(writer.bezier_to_2nd_ctrlpt, 0, 0, 0, 0 )
    testlib.must_throw(writer.line_to, 0, 0)
    testlib.must_throw(writer.path_close)
    doc.page_end()
    doc.finalize()

if __name__ == "__main__":
    test_main()


# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jag.testlib as testlib

#test that graphics states are shared

def test_main(argv=None):
    writer = testlib.create_test_doc(argv,'grstate.pdf')
    writer.page_start(2*72, 300)
    page = writer.page().canvas()
    page.alpha("s", 0.5)
    page.circle(72, 72, 36)
    page.path_paint("s")
    page.alpha("s", 0.6)
    page.circle(72, 120, 36)
    page.path_paint("s")
    page.alpha("s", 0.6)
    page.circle(72, 162, 36)
    page.path_paint("s")

    writer.page_end()
    writer.finalize()


if __name__ == '__main__':
    test_main()


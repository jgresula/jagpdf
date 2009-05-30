#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import jagpdf

def test_main(argv=None):
    assert jagpdf.version() >= 0x010000

if __name__ == '__main__':
    test_main()


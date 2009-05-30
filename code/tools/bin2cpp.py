#!/usr/bin/env python
#
# $(Copyright)
# $(License)
#

import string
import sys
import os

templ="""// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

#include \"precompiled.h\"
#include <interfaces/stdtypes.h>
#include <cstring>

// this file was generated with the following command:
// bin2cpp.py $binfile $cppfile $var_name

namespace jag {
namespace binres {

extern const std::size_t ${var_name}_size = $bin_data_len;
extern const Byte $var_name[${var_name}_size] =
{
$bin_data
};

}} // namespace jag::binres


/** EOF @file */
"""


def main( binfile, cppfile, var_name ):
    bincontent = open( binfile, "rb" ).read()
    bin_data_len = len(bincontent)

    bin_data = []
    bytes_on_line = 10
    prev_i, i = 0, bytes_on_line
    while prev_i < bin_data_len:
        bin_data.append( ", ".join( [ "0x%x"%ord(c) for c in bincontent[prev_i:i] ] ) )
        prev_i = i
        i += bytes_on_line
    bin_data = ",\n".join( bin_data )
    cppcontent = string.Template( templ ).substitute( locals() )
    open( cppfile, "wb" ).write( cppcontent )

if __name__ == "__main__":
    # cmd line: bin2cpp.py binfile cppfile varname

#    args = sys.argv[1:]
    args = "../src/resources/resfiles/iccprofiles/AdobeRGB.icc", "../src/resources/resfiles/icc_adobergb.cpp", "icc_adobe_rgb"
    args = "../src/resources/resfiles/iccprofiles/sRGB.icm", "../src/resources/resfiles/icc_srgb.cpp", "icc_srgb"
    assert len(args) == 3
    assert os.path.isfile( args[0] )
    main( *args )

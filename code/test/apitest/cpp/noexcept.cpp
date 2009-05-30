// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testcommon.h"
using namespace jag;

// verifies that the error codes in C++ works correctly

int main(int, char**)
{
    BOOST_TEST(!pdf::create_file("/this/file/does/not/exist"));

    StreamNoop stream;
    pdf::Document doc(pdf::create_stream(&stream));
    BOOST_TEST(doc);
    BOOST_TEST(!doc.image_load(pdf::ImageDef()));
    BOOST_TEST(doc.page_end());
    return boost::report_errors();
}


/** EOF @file */

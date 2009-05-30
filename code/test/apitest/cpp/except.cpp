// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testcommon.h"
using namespace jag;

// verifies that the pdf::Exception class behaves correctly

namespace
{

void test_main(int, char**)
{
    pdf::Exception exc_stored;
    try
    {
        pdf::create_file("/this/file1/does/not/exist");
    }
    catch(pdf::Exception const& exc)
    {
        jag_error_reset();
        BOOST_TEST(strcmp("file1", exc.what()));
        exc_stored = exc;
        BOOST_TEST(strcmp("file1", exc_stored.what()));
    }

    try
    {
        pdf::create_file("/this/file2/does/not/exist/as/well");
    }
    catch(pdf::Exception const& exc)
    {
        BOOST_TEST(strcmp("file2", exc.what()));
    }

    {
        pdf::Exception exc2(exc_stored);
        BOOST_TEST(strcmp("file1", exc2.what()));
    }

    BOOST_TEST(strcmp("file1", exc_stored.what()));
}

} // namespace



int except(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}



/** EOF @file */

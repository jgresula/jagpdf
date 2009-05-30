// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testcommon.h"
using namespace jag;


namespace
{
  void test_main(int, char**)
  {

      // test that two identical font loads return the same object
      StreamNoop stream;
      pdf::Document doc(pdf::create_stream(&stream));
      pdf::Font f1 = doc.font_load("standard; name=Helvetica; size=40");
      pdf::Font f2 = doc.font_load("standard; name=Helvetica; size=40");
      BOOST_TEST(f1.handle_() == f2.handle_());
      pdf::Font f3 = doc.font_load("standard; name=Helvetica; size=20");
      BOOST_TEST(f1.handle_() != f3.handle_());
  }
} // namespace


int fonts(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}

/** EOF @file */

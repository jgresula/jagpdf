// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testcommon.h"
using namespace jag;

namespace
{
//
//
//
  void test_main(int, char**)
  {
      StreamNoop stream;
      pdf::Document doc(pdf::create_stream(&stream));
      doc.finalize();
  }
} //namespace


//
//
//
int defaultargs(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}


/** EOF @file */

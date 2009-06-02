// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <cstdio>
#include <string>
#include <iostream>
#include <stdlib.h>

#include "testcommon.h"
using namespace jag;

// tests that enum default arguments work

namespace
{
  void test_main(int /*argc*/, char** /*argv*/)
  {
      StreamNoop stream;
      pdf::Document doc(pdf::create_stream(&stream));
      doc.page_start(10.0*72, 10.0*72);
      pdf::Canvas canvas(doc.page().canvas());

      std::ostringstream img_path;
      img_path <<  getenv("JAG_TEST_RESOURCES_DIR")
               << "/images/lena.jpg";

      doc.image_load_file(img_path.str().c_str());
      doc.image_load_file(img_path.str().c_str(), pdf::IMAGE_FORMAT_JPEG);

      doc.page_end();
      doc.finalize();
  }
} // anonymous namespace

int autodetectimg(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}



/** EOF @file */

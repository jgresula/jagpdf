// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <stdio.h>
#include <string>
#include "testcommon.h"
using namespace jag;

namespace
{
//
//
//
  void test_main(int, char** argv)
  {
      std::string out_file(argv[1]);
      out_file += "/basic_extstream.pdf";

      pdf::Profile cfg(pdf::create_profile());
      cfg.set("doc.compressed", "0");

      StreamFile stream(out_file.c_str());

      pdf::Document doc(pdf::create_stream(&stream, pdf::Profile()));
      doc.page_start(5.9*72, 3.5*72);
      doc.page_end();
      doc.finalize();
  }
}

int externalstream(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}


/** EOF @file */

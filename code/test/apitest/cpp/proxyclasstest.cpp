// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testcommon.h"
using namespace jag;

namespace
{
  pdf::Canvas get_canvas(pdf::Document doc)
  {
      pdf::Canvas canvas(doc.page().canvas());
      return canvas;
  }

  void test_main(int, char**)
  {
      StreamNoop stream;
      pdf::Document doc(pdf::create_stream(&stream, pdf::Profile()));
      doc.page_start(72, 72);
      pdf::Canvas canvas(get_canvas(doc));
      doc.page_end();

      doc.page_start(72, 72);
      canvas = get_canvas(doc);
      doc.page_end();
      doc.finalize();
  }
} //namespace

int proxyclasstest(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}

/** EOF @file */

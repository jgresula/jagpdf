// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <stdio.h>
#include <jagpdf/api.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include "testcommon.h"

using namespace jag;

// likely not portable but works
#if defined(__GNUC__)
# include <unistd.h>
#endif

namespace
{
//
//
//
  std::string cfg_save_and_load(pdf::Profile cfg, char const* fname)
  {
      cfg.save_to_file(fname);
      std::ifstream fini(fname);
      std::stringstream cfg_str;
      cfg_str << fini.rdbuf();
      return cfg_str.str();
  }

//
//
//
  void test_main(int /*argc*/, char** argv)
  {
      std::string out_file(argv[1]);
      out_file += "/config.pdf";
      std::string ini_file(argv[1]);
      ini_file += "/config.ini";

      {
          StreamNoop stream;

          pdf::Document doc1(pdf::create_stream(&stream, pdf::Profile()));
          pdf::Document doc2(pdf::create_file(out_file.c_str(), pdf::Profile()));
      }
      {
          pdf::Profile cfg(pdf::create_profile());
          cfg.set("doc.compressed", "save-this-town");

          cfg.save_to_file(ini_file.c_str());
          std::ifstream fini(ini_file.c_str());
          std::stringstream cfg_str;
          cfg_str << fini.rdbuf();

          pdf::Profile cfg3 = pdf::create_profile_from_file(ini_file.c_str());
          pdf::Profile cfg2 = pdf::create_profile_from_string(cfg_str.str().c_str());

          std::string str1 = cfg_save_and_load(cfg2, ini_file.c_str());
          std::string str2 = cfg_save_and_load(cfg3, ini_file.c_str());

          BOOST_TEST(str1 == str2);
          JAG_MUST_THROW(pdf::create_profile_from_string("non-sense"));
      }

      unlink(out_file.c_str());
      unlink(ini_file.c_str());
  }
} //namespace

int config(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}

/** EOF @file */

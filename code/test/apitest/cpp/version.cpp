// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testcommon.h"

using namespace jag::pdf;

namespace
{
  void test_main(int /*argc*/, char** /*argv*/)
  {
      UInt version = jag::pdf::version();
      UInt major = version >> 16;
      UInt minor = (version >> 8) & 0xff;
      UInt patch = version & 0xff;

      BOOST_TEST(major == this_version_major);
      BOOST_TEST(minor == this_version_minor);
      BOOST_TEST(patch == this_version_patch);
      BOOST_TEST(version == this_version);
  }
} // namespace


int version(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}




/** EOF @file */

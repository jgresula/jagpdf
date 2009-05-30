// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <core/errlib/errlib.h>
#include <jagpdf/detail/errcodes.h>
#include <iostream>
#include "testtools.h"

#ifdef _WIN32
# include <windows.h>
#endif

using namespace jag;

namespace
{

  template <class E>
  void test_fmt(char fmt, unsigned errcode)
  {
      E exc((msg_invalid_operation()));
      BOOST_TEST(exc.what()[0] == fmt);
      BOOST_TEST(exc.errcode() == errcode);
  }

  void test_get_win_error()
  {
#ifdef _WIN32
      try
      {
          std::string fname("q:/this file does not exist *");
          HANDLE handle = ::CreateFile(fname.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
          BOOST_TEST(handle == INVALID_HANDLE_VALUE);
          throw exception_io_error(msg_info_t(10, "::CreateFile failed"))
              << JAGLOC
              << win_error_info(::GetLastError())
              << io_object_info(fname);
      }
      catch(exception const& exc)
      {
          std::ostringstream ss;
          output_exception(exc, ss);
          std::string msg(ss.str());
          BOOST_TEST(0 != strstr(msg.c_str(), "syserr"));
      }
#endif
  }

  void test()
  {
      test_fmt<exception_io_error>('1', err_io_error);
      test_fmt<exception_operation_failed>('1', err_operation_failed);
      test_fmt<exception_invalid_operation>('1', err_invalid_operation);

      test_get_win_error();
  }
} // anonymous namespace

int exceptions(int, char ** const)
{
    int result = guarded_test_run(test);
    result += boost::report_errors();
    return result;
}


/** EOF @file */

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <core/jstd/thread.h>
#include <core/jstd/tss.h>

#include <core/generic/stringutils.h>
#include <jagpdf/api.h>

#include "testtools.h"
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <assert.h>

using namespace jag;

namespace
{
  void thread_fn(int id)
  {
#ifndef __GNUC__
      jag::jstd::tls_on_thread_attach();
#endif

      try
      {
          const int buff_size = 128;
          char templ[buff_size];
          sprintf(templ, "/jag/this/directory/does/not_exist/%d/%%d.pdf", id);


          for (int i=0; i<100; ++i)
          {
              BOOST_TEST(!jag_error_code());
              BOOST_TEST(is_empty(jag_error_msg()));

              char buff[buff_size];
              sprintf(buff, templ, i);

              // do a bad call
              jag_Document writer(jag_create_file(buff, 0));
              BOOST_TEST(!writer);
              jstd::scheduled_yield();

              // check if we really got an error
              BOOST_TEST(jag_error_code());
              jstd::scheduled_yield();


              // check we got an error we expect
              char const* err_msg = jag_error_msg();
//            puts(err_msg);
              BOOST_TEST(!is_empty(err_msg));
              BOOST_TEST(strstr(err_msg, buff));
              jstd::scheduled_yield();


              // reset the error slot and see if it is really empty
              jstd::scheduled_yield();
              BOOST_TEST(jag_error_code() == err_io_error);
              jag_error_reset();

              jstd::scheduled_yield();
              BOOST_TEST(!jag_error_code());
              BOOST_TEST(is_empty(jag_error_msg()));

              jstd::scheduled_yield();
          }
      }
      catch(std::exception const& exc)
      {
          puts(exc.what());
      }

#ifndef __GNUC__
      jag::jstd::tls_on_thread_detach();
#endif
  }



  void test()
  {
      jag::jstd::tls_on_process_attach();

      const int num_threads = 30;

      boost::ptr_vector<jstd::Thread> threads;
      for (int i=0; i<num_threads; ++i)
          threads.push_back(new jstd::Thread(boost::bind(thread_fn, i)));

      for (int i=0; i<num_threads; ++i)
          threads[i].join();

      jag::jstd::tls_on_process_detach();
  }

} //anonymous namespace

int errortls(int, char ** const)
{
    int result = guarded_test_run(test);
    result += boost::report_errors();
    return result;
}



namespace jag { namespace jstd {
    // this confirms that tls hooks are called
    void tls_cleanup_implemented() {}
}}



/** EOF @file */

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#include "testtools.h"
#include <core/jstd/mmap.h>
#include <core/jstd/fileso.h>
#include <core/jstd/thread.h>
#include <core/jstd/md5.h>

using namespace jag;
using namespace jag::jstd;

namespace
{
  void test()
  {
      JAG_MUST_THROW(MMapFileStreamOutput("/this/file/cerainly/does/not/exist"), jag::exception);

      char const* tmpname = tmpnam(0);

      try
      {
          // create ~200k file
          MMapFileStreamOutput f(tmpname, 100*1024);
          MD5Hash md5out;
          ULong pos_=0;
          for(int i=0; i<15390; ++i)
          {
              f.write("nazdar!", 0); // bogus

              f.write("nazdar!", 7);
              md5out.append("nazdar!", 7);
              f.write("bazar!", 6);
              md5out.append("bazar!", 6);
              pos_ += 13;
              if (!(i%1000))
              {
                  BOOST_TEST(f.tell() == pos_);
              }
          }
          md5out.finish();
          f.close();


          // read the file and check md5
          File infile;
          MD5Hash md5in;
          infile.create(tmpname, File::READ);
          const UInt buff_size = 1024;
          char buffer[buff_size];
          ULong nr_read;
          while(infile.read(&buffer, buff_size, &nr_read))
          {
              md5in.append(buffer, static_cast<md5_word_t>(nr_read));
          }
          md5in.append(buffer, static_cast<md5_word_t>(nr_read));
          md5in.finish();

          BOOST_TEST(infile.tell() == 15390*13);
          BOOST_TEST(!memcmp(md5out.sum(), md5in.sum(), sizeof(MD5Hash::Sum)));
      }
      catch(...)
      {
          unlink(tmpname);
          throw;
      }

      unlink(tmpname);
  }
} // anonymous namespace

int mmapfile(int, char ** const)
{
    int result = guarded_test_run(test);
    result += boost::report_errors();
    return result;
}


/** EOF @file */

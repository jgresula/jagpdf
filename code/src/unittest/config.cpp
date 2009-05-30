// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#include <core/generic/refcountedimpl.h>
#include <core/jstd/configimpl.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/file_stream.h>
#include <core/errlib/errlib.h>
#include <boost/format.hpp>

#define SHOW_CAUGHT_EXCEPTION 1
#include "testtools.h"

using namespace jag::jstd;
using namespace jag;
using namespace boost;

namespace
{
  extern jag::jstd::cfg_pair_t const s_test_symbols_[];
  jag::jstd::cfg_pair_t const* s_test_symbols = s_test_symbols_;


  typedef boost::intrusive_ptr<ConfigImpl> ConfigPtr;
  ConfigPtr create_profile()
  {
      return ConfigPtr(
                       new RefCountImpl<ConfigImpl,RefCountMT>(s_test_symbols));
  }


  //
  //
  //
  void test()
  {
      {
          //
          // options verification
          //
          ConfigPtr cfg(create_profile());
          JAG_MUST_THROW(cfg->verify("unknown"), exception_invalid_value);
          cfg->verify("doc.version");
      }

      {
          ConfigPtr cfg(create_profile());

          //
          // ill formed input
          //
          char const* ill_formed[] = {
              "no equal sign\n",
              " = no keyword\n",
              "unknown_keyword=val\n",
              0
          };
          for(char const**s = ill_formed; *s; ++s)  {
              JAG_MUST_THROW(
                             load_from_string(*s, s_test_symbols),
                             exception_invalid_value);
          }


          //
          // well formed input
          //
          char const* well_formed[] = {
              "info.title=b\n" " info.title=b\n" " info.title = val",
              // comments
              "#  \n" "# 123\n" "### ##\n",
              // eof and '=' as a value part
              "info.title = b = c\n info.title = c",
              "info.title =\n",
              "info.title =  \n",
              0
          };
          for(char const**s = well_formed; *s; ++s)  {
              load_from_string(*s, s_test_symbols);
          }
      }


      {
          //
          // options read/write
          //
          ConfigPtr cfg(load_from_string("doc.compressed=50",
                                         s_test_symbols));
          // read string and int
          BOOST_TEST(!strcmp("50", cfg->get("doc.compressed")));
          BOOST_TEST(50 == cfg->get_int("doc.compressed"));
          // set and read
          cfg->set("doc.compressed", "324");
          BOOST_TEST(324 == cfg->get_int("doc.compressed"));
          // reading non-int
          JAG_MUST_THROW(cfg->get_int("fonts.default"), exception_invalid_value);
      }


      {
          //
          // save and load from a stream, clone
          //
          ConfigPtr cfg(create_profile());

          cfg->set("doc.compressed", "0");
          cfg->set("doc.version", "20");
          MemoryStreamOutput mout;
          cfg->save(mout);


          char const* in_mem = (char const*)mout.data();
          std::string str(in_mem, in_mem + mout.tell());
          ConfigPtr cfg2(load_from_string(str.c_str(), s_test_symbols));

          BOOST_TEST(!strcmp("0", cfg2->get("doc.compressed")));
          BOOST_TEST(!strcmp("20", cfg2->get("doc.version")));

          intrusive_ptr<IProfileInternal> cloned(cfg2->clone());
          BOOST_TEST(0 == cloned->get_int("doc.compressed"));
          BOOST_TEST(20 == cloned->get_int("doc.version"));
      }

      {
          // save to file, then load from file, then save to string and
          // compare the result
          MemoryStreamOutput mout_orig;
          ConfigPtr cfg(create_profile());
          cfg->save(mout_orig);

          {
              // load saved config from string
              char const* start = (char const*)(mout_orig.data());
              std::string str_orig(start, start+mout_orig.tell());
              ConfigPtr cfg_mem(load_from_string(str_orig.c_str(), s_test_symbols));
              MemoryStreamOutput mout_str;
              cfg_mem->save(mout_str);

              BOOST_TEST(mout_orig.tell() == mout_str.tell());
              BOOST_TEST(!memcmp(mout_str.data(), mout_orig.data(), mout_str.tell()));
          }

          {
              // save config to file and load it from file
              std::string tmpname = tmpnam(0);
              cfg->save_to_file(tmpname.c_str());
              ConfigPtr cfg_file(load_from_file(tmpname.c_str(), s_test_symbols));
              unlink(tmpname.c_str());
              MemoryStreamOutput mout;
              cfg_file->save(mout);

              BOOST_TEST(mout_orig.tell() == mout.tell());
              BOOST_TEST(!memcmp(mout.data(), mout_orig.data(), mout.tell()));
          }
      }
  }
} // anonymous namespace

int config(int, char ** const)
{
    int result = guarded_test_run(test);
    result += boost::report_errors();
    return result;
}


namespace
{

  ///////////////////////////////////////////////////////////////////////////
  //
  // test configuration
  //

  jag::jstd::cfg_pair_t const s_test_symbols_[] =
      {
          { "doc.version"           , "5" },
          { "doc.encryption"        , "" },
          { "doc.static_file_id"    , "0" },
          { "doc.compressed"  , "1" },
          { "doc.strict_mode"       , "1" },
          { "doc.trace_level"       , "1" },
          { "doc.trace_show_loc"    , "0" },

          { "patterns.tiling_type"     , "1" },

          { "fonts.embedded"     , "1" },
          { "fonts.default"       , "standard;name=Helvetica;size=12"},
          { "fonts.synthesized"  , "1" },
          { "fonts.subset"       , "1" },
          { "fonts.force_cid"    , "1" },

          { "images.interpolated"        , "0" },
          { "images.intent"              , "" },
          { "images.default_dpi"         , "72" },
          { "images.softmask_16_to_8"    , "1" },

          { "info.title"             , "" },
          { "info.author"            , "" },
          { "info.subject"           , "" },
          { "info.keywords"          , "" },
          { "info.creator"           , "" },
          { "info.creation_date"     , "1" },

          { "stdsh.pwd_owner"         , "" },
          { "stdsh.pwd_user"          , "" },
          { "stdsh.key_length"        , "40" },
          { "stdsh.revision"          , "2" },
          { "stdsh.perm_bit3"         , "0" },
          { "stdsh.perm_bit4"         , "0" },
          { "stdsh.perm_bit5"         , "0" },
          { "stdsh.perm_bit6"         , "0" },
          { "stdsh.perm_bit9"         , "0" },
          { "stdsh.perm_bit10"        , "0" },
          { "stdsh.perm_bit11"        , "0" },
          { "stdsh.perm_bit12"        , "0" },
          { 0                    , 0 }
      };

} // anonymous namespace


/** EOF @file */

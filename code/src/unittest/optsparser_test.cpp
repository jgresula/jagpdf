// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#include "testtools.h"
#include <core/jstd/optionsparser.h>
#include <core/generic/floatpointtools.h>
#include <core/errlib/errlib.h>

using namespace jag;
using namespace jag::jstd;
using namespace boost::spirit::classic;

#ifdef max
# undef max
# undef min
#endif


namespace
{
  enum { FACENAME, POINTSIZE, ENCODING, FONTFILE };

  struct TestKeywords
      : public symbols<unsigned>
  {
      TestKeywords()
      {
          add
              ("name", FACENAME)
              ("size", POINTSIZE)
              ("enc", ENCODING)
              ("file", FONTFILE)
              ;
    }
  } g_keywords;

  enum { CORE, FAKE_EXPL };
  struct ExplicitValues
      : public symbols<unsigned>
  {
      ExplicitValues()
      {
          add
              ("standard", CORE)
              ;
      }
  } g_expl_values;

  enum {K_VERSION, K_TYPE, K_SOME, K_ANOTHER};
  struct Values
      : public SymTab
  {
      Values() {
          add
              ("version", K_VERSION)
              ("type", K_TYPE)
              ("some", K_SOME)
              ("another", K_ANOTHER)
              ;
      }
  } g_some_values;


//
//
//
  void test_options()
  {
      char const* src_str = "name=ferda;size=3;enc=2,3.14,54, 43, none;standard";
      ParsedResult const& p1 = parse_options(src_str, ParseArgs(&g_keywords, &g_expl_values));
      //BOOST_TEST(p1.parse_info_full());

      BOOST_TEST(p1.has_symbol(POINTSIZE));
      BOOST_TEST(!p1.has_symbol(FONTFILE));

      BOOST_TEST(p1.to_<int>(ENCODING, 15) == 2);
      BOOST_TEST(p1.to_<int>(ENCODING, 15, 3) == 43);
      BOOST_TEST(p1.to_<int>(ENCODING, 15, 0xff) == 15);

      BOOST_TEST(equal_doubles(p1.to_<double>(ENCODING, 0.0, 1), 3.14));
      BOOST_TEST(equal_doubles(p1.to_<double>(ENCODING, 0.0, 2), 54.0));
      BOOST_TEST(equal_doubles(p1.to_<double>(ENCODING, 7.0, 0xff), 7.0));

      BOOST_TEST(p1.to_<string_32_cow>(FACENAME) == "ferda");
      BOOST_TEST(p1.to_<string_32_cow>(ENCODING, 0, 4) == "none");
      BOOST_TEST(p1.to_<string_32_cow>(FONTFILE, 0).empty());
      BOOST_TEST(p1.to_<string_32_cow>(FONTFILE, "").empty());


      BOOST_TEST(p1.has_explicit_value(CORE));
      BOOST_TEST(!p1.has_explicit_value(FAKE_EXPL));

      BOOST_TEST(p1.num_values(ENCODING)==5);

      // empty string DOES NOT fail
      parse_options("", ParseArgs(&g_keywords));
      // semicolon only DOES NOT fail
      parse_options(";", ParseArgs(&g_keywords, &g_expl_values));
      parse_options(" ", ParseArgs(&g_keywords));
      parse_options(";;", ParseArgs(&g_keywords));
      parse_options("  ;   ;  ", ParseArgs(&g_keywords));



      // keyword with fixed values
      ParseArgs args1(&g_keywords);
      args1.use_values_for_symbol(FACENAME, g_some_values);
      args1.use_values_for_symbol(FONTFILE, g_some_values);
      args1.use_values_for_symbol(POINTSIZE, g_some_values);

      JAG_MUST_THROW(parse_options("name=unknown", args1), jag::exception);
      JAG_MUST_THROW(parse_options("name=unknown;file=some", args1), jag::exception);
      ParsedResult const& p2 = parse_options("name=some;file=another;enc=unknown", args1);
      BOOST_TEST(p2.to_<unsigned>(FACENAME)==K_SOME);
      BOOST_TEST(p2.to_<unsigned>(FONTFILE)==K_ANOTHER);
      BOOST_TEST(p2.to_<unsigned>(POINTSIZE, K_TYPE)==K_TYPE);

      ParseArgs args2(&g_keywords, &g_some_values);
      args2.use_values_for_symbol(FACENAME, g_some_values);
      ParsedResult const& p3 = parse_options("name=some;another", args2);
      BOOST_TEST(p3.explicit_value()==K_ANOTHER);
      ParsedResult const& p4 = parse_options("name=some", args2);
      BOOST_TEST(p4.explicit_value()==ParsedResult::NO_EXPL_VALUE);
      BOOST_TEST(p4.explicit_value(K_ANOTHER)==K_ANOTHER);


      ParsedResult const& p5 = parse_options("some; file=1.0, -1.0", args2);
      printf(">> %lf, %lf\n",
              p5.to_<double>(FONTFILE, 0.0, 0),
              p5.to_<double>(FONTFILE, 0.0, 1));

      BOOST_TEST(equal_doubles(p5.to_<double>(FONTFILE, 0.0, 0), 1.0));
      BOOST_TEST(equal_doubles(p5.to_<double>(FONTFILE, 0.0, 1), -1.0));
  }


//
//
//
  void test()
  {
      test_options();
  }
} // anonymous namespace


int optsparser_test(int, char ** const)
{
    int result = guarded_test_run(test);
    result += boost::report_errors();
    return result;
}



/** EOF @file */

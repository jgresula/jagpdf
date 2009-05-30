// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#define JAG_INCLUDE_STRING_UTILS_EX
#include <core/jstd/configimpl.h>
#include <core/generic/refcountedimpl.h>
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>
#include <msg_jstd.h>
#include <interfaces/streams.h>
#include <core/generic/stringutils.h>
#include <core/jstd/file_stream.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/streamhelpers.h>
#include <core/generic/checked_cast.h>
#include <core/generic/stringutils.h>
#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/range/iterator_range.hpp>

#if defined(_MSC_VER)
# pragma warning(push)
// unreachable code
# pragma warning(disable:4702)
#endif
  #include <boost/lexical_cast.hpp>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif



using namespace boost;
//using namespace boost::spirit;

namespace jag {
namespace jstd {


//
// some operations on the default config definition
//
namespace
{
  // linear search through the unsorted list of strings
  cfg_pair_t const* find_cfg_pair(cfg_pair_t const* symbols, char const* kwd)
  {
      for(cfg_pair_t const* it=symbols; it->name; ++it)
      {
          if (!strcmp(kwd, it->name))
              return it;
      }
      return 0;
  }

  // retrieves a default value for the given symbol
  char const* default_value(cfg_pair_t const* symbols, char const* kwd)
  {
      cfg_pair_t const* pair = find_cfg_pair(symbols, kwd);
      if (pair)
      {
          JAG_ASSERT_MSG(pair->default_value, "option's default value is NULL");
          return pair->default_value;
      }
      else
      {
          return 0;
      }
  }

  // figures out whether the given symbol is valid
  bool is_valid_kwd(cfg_pair_t const* symbols, char const* kwd)
  {
      return find_cfg_pair(symbols, kwd);
  }
} // anonymous namespace



///////////////////////////////////////////////////////////////////////////
// class ConfigImpl

//
//
//
ConfigImpl::ConfigImpl(cfg_pair_t const* symbols)
    : m_symbols(symbols)
    , m_verification_active(true)
{}

//
//
//
void ConfigImpl::set(Char const* option, Char const* value)
{
    if (!is_empty(option))
    {
        set_r(option, option+strlen(option),
              value, value+strlen(value));
    }
}

//
//
//
void ConfigImpl::set_r(Char const* kwd_begin, Char const* kwd_end,
                        Char const* value_begin, Char const* value_end)
{
    // verify that the passed option is valid
    string_t kwd(kwd_begin, kwd_end);
    verify(kwd.c_str());

    // update the map
    config_map_t::iterator it = m_config_map.find(kwd);
    if (it != m_config_map.end())
        m_config_map.erase(it);

    m_config_map.insert(config_map_t::value_type(
                            kwd,
                            string_t(value_begin, value_end)));
}

//
//
//
Char const* ConfigImpl::get(Char const* kwd) const
{
    verify(kwd);
    Char const* value = 0;
    config_map_t::const_iterator it = m_config_map.find(kwd);
    value = (it == m_config_map.end())
        ? default_value(m_symbols, kwd)
        : it->second.c_str();

    if (!value)
        JAG_INTERNAL_ERROR;

    return value;
}


//
//
//
int ConfigImpl::get_int(Char const* kwd) const
{
    Char const* val = get(kwd);
    try
    {
        // suboptimal: lexical_cast make a copy of the string, spirit::int_p
        // might be better here
        return lexical_cast<int>(val);
    }
    catch(bad_lexical_cast const&)
    {
        throw exception_invalid_value(
            msg_config_integer_value(kwd)) << JAGLOC;
    }
}


//
//
//
void ConfigImpl::verify(Char const* kwd) const
{
    JAG_PRECONDITION(m_symbols);
    if (!m_verification_active)
        return;

    if (!is_valid_kwd(m_symbols, kwd))
    {
        throw exception_invalid_value(
            msg_config_unknown_option(kwd)) << JAGLOC;
    }
}


//
//
//
void ConfigImpl::save(ISeqStreamOutput& stream) const
{
    // the order is the same as in the symbol definition list
    for(cfg_pair_t const* sym=m_symbols; sym->name; ++sym)
    {
        char const* val = get(sym->name);

        stream.write(sym->name, strlen(sym->name));
        stream.write("=", 1);
        stream.write(val, strlen(val));
        stream.write("\n", 1);
    }
}


//
//
//
void ConfigImpl::save_to_file(Char const* fname) const
{
    FileStreamOutput fout(fname);
    save(fout);
}


//
//
//
intrusive_ptr<IProfileInternal> ConfigImpl::clone() const
{
    intrusive_ptr<ConfigImpl> result(
        new RefCountImpl<ConfigImpl,RefCountMT>(m_symbols));

    result->m_config_map = m_config_map;
    // intentionally do not copy m_verification_active
    return result;
}




///////////////////////////////////////////////////////////////////////////
// parsing a config from a stream

namespace
{
  //
  // Handles parsing events and redirects them to a config object
  //
  class ParseHandler
  {
      ConfigImpl& m_cfg;
      char const* m_kwd_start;
      char const* m_kwd_end;

  public:
      ParseHandler(ConfigImpl& cfg)
          : m_cfg(cfg)
          , m_kwd_start(0)
          , m_kwd_end(0)
      {}

      void on_kwd(char const* str, char const* end) {
          m_kwd_start=str;
          m_kwd_end=end;
      }

      void on_val(char const* str, char const* end) {
          JAG_PRECONDITION(m_kwd_start && m_kwd_end);
          m_cfg.set_r(m_kwd_start, m_kwd_end, str, end);
      }
  };


  //
  // Parses the configuration form the passed string.
  //
  intrusive_ptr<ConfigImpl> load_from_string_r(
      Char const* str,
      std::size_t len,
      cfg_pair_t const* symbols)
  {
      intrusive_ptr<ConfigImpl> cfg(
          new RefCountImpl<ConfigImpl,RefCountMT>(symbols));

      ParseHandler handler(*cfg);

      typedef algorithm::split_iterator<char const*> split_iter_t;
      typedef iterator_range<char const*> iter_rng_t;

      split_iter_t end;
      split_iter_t line_iter(str, str + len,
                             algorithm::token_finder(is_any_of("\n\r"),
                                                     token_compress_on));

      for(; line_iter != end; ++line_iter)
      {
          // trim the whole line, ignore blank lines and comments
          iter_rng_t line(trim_range(line_iter->begin(), line_iter->end()));
          if (line.begin() >= line.end()) continue;
          if (*line.begin() == '#') continue;

          iter_rng_t eq_pos(algorithm::find_first(line, "="));
          if (eq_pos.empty())
          {
              throw exception_invalid_value(
                  msg_config_parse_error()) << JAGLOC;
          }

          iter_rng_t kwd(trim_range(line.begin(), eq_pos.begin()));
          iter_rng_t val(trim_range(eq_pos.end(), line.end()));

          // TEST: change the following if it is tested somewhere
          handler.on_kwd(kwd.begin(), kwd.end());
          handler.on_val(val.begin(), val.end());
      }

      return cfg;
  }
} // anonymous namespace


//
//
//
intrusive_ptr<ConfigImpl> load_from_string(
    Char const* str,
    cfg_pair_t const* symbols)
{
    return load_from_string_r(str, strlen(str), symbols);
}


//
//
//
intrusive_ptr<ConfigImpl> load_from_file(
    Char const* fname,
    cfg_pair_t const* symbols)
{
    FileStreamInput fin(fname);
    MemoryStreamOutput mout;
    copy_stream(fin, mout);
    return load_from_string_r(jag_reinterpret_cast<Char const*>(mout.data()),
                              static_cast<size_t>(mout.tell()), symbols);
}

}} // namespace jag::jstd

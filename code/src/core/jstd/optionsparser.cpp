// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#define JAG_INCLUDE_STRING_UTILS_EX
#include <core/jstd/optionsparser.h>
#include <msg_jstd.h>
#include <core/errlib/errlib.h>
#include <core/generic/assert.h>
#include <core/generic/macros.h>
#include <core/generic/stringutils.h>

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/ref.hpp>
#include <boost/spirit/include/classic_core.hpp>

// new
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/spirit/include/classic_symbols.hpp>


#include <map>
#include <set>

// TBD - ParseInfo should be intrusive_ptr instead of shared_ptr

using namespace boost;
using namespace boost::spirit::classic;

namespace jag {
namespace jstd {

namespace
{
  const unsigned INVALID_KWD = 0xffffffff;

  ///
  /// Storage for parsed content.
  ///
  struct ParsedStorage
  {
      typedef std::pair<unsigned, int>             kwd_t;
      typedef std::pair<char const*, char const*>  value_t;
      typedef std::map<kwd_t, value_t>             options_t;
      typedef std::map<unsigned, unsigned>         fixed_values_t;
      typedef std::set<unsigned>                   explicit_values_t;

      options_t               m_options;
      explicit_values_t       m_values;
      fixed_values_t          m_fixed_values;
  };

} // namespace anonymous





namespace detail {

//
// Complete info about parsed data and the result. Its scope should be aligned
// with the lifetime of the parsed string.
//
struct ParseInfo
    : boost::noncopyable
{
    ParsedStorage           data;
    char const*             str;
};

} // namespace detail

namespace
{

  //
  //
  //
  unsigned id_from_string(char const* str, SymTab const& syms)
  {
      unsigned* val = boost::spirit::classic::find(syms, str);
      if (val)
          return *val;

      // ERROR: unknown keyword or explicit value
      throw exception_invalid_value(msg_opt_parser_invalid_token(str)) << JAGLOC;
  }
}


//
//
//
ParsedResult parse_options(char const* options, ParseArgs const& pargs)
{
    shared_ptr<detail::ParseInfo> pi(new detail::ParseInfo);
    pi->str=options;

    typedef algorithm::split_iterator<char const*> split_iter_t;
    typedef iterator_range<char const*> iter_rng_t;

    split_iter_t end;
    split_iter_t opts_iter(options, options + strlen(options), first_finder(";"));

    // iterate over all options
    for(; opts_iter != end; ++opts_iter)
    {
        iter_rng_t opts(trim_range(opts_iter->begin(), opts_iter->end()));

        if(opts.begin() >= opts.end())
            continue; // empty option is ok

        iter_rng_t eq_pos(algorithm::find_first(opts, "="));
        iter_rng_t kwd(trim_range(opts.begin(), eq_pos.begin()));

        ParsedStorage& stg = pi->data;
        ParsedStorage::kwd_t current_kwd;
        SymTab const* symtab_vals = 0;

        std::string kwd_str(kwd.begin(), kwd.end());

        if (eq_pos.empty())
        {
            // on explicit value
            unsigned val_id = id_from_string(kwd_str.c_str(), pargs.expl_vals());
            stg.m_values.insert(val_id);
            current_kwd.first = INVALID_KWD;
        }
        else
        {
            // on keyword
            unsigned kwd_id = id_from_string(kwd_str.c_str(), pargs.kwds());
            current_kwd.first = kwd_id;
            current_kwd.second = 0;
            symtab_vals = pargs.find_values_for_kwd(kwd_id);

            // iterate over values
            split_iter_t val_iter(eq_pos.end(), opts.end(), first_finder(","));
            for(; val_iter != end; ++val_iter)
            {
                iter_rng_t val(trim_range(val_iter->begin(), val_iter->end()));
                if (symtab_vals)
                {
                    // on fixed value
                    typedef ParsedStorage::fixed_values_t::value_type item_t;
                    JAG_ASSERT(current_kwd.first != INVALID_KWD);

                    std::string val_str(val.begin(), val.end());
                    unsigned val_id = id_from_string(val_str.c_str(), *symtab_vals);
                    stg.m_fixed_values.insert(item_t(current_kwd.first, val_id));
                    ++current_kwd.second;
                }
                else
                {
                    // on value
                    typedef ParsedStorage::options_t::value_type item_t;
                    typedef ParsedStorage::value_t value_t;
                    JAG_ASSERT(current_kwd.first != INVALID_KWD);
                    stg.m_options.insert(
                        item_t(current_kwd, value_t(val.begin(), val.end())));
                    ++current_kwd.second;
                }
            }

        }
    }

    return ParsedResult(pi);
}

// ---------------------------------------------------------------------------
//                         class ParseArgs
//
const SymTab ParseArgs::s_empty = SymTab();

//
//
//
ParseArgs::ParseArgs(SymTab const* kwds, SymTab const* expl)
    : m_kwds(kwds ? *kwds : ParseArgs::s_empty)
    , m_explicit(expl ? *expl : ParseArgs::s_empty)
{
}

//
//
//
void ParseArgs::use_values_for_symbol(unsigned kwd, SymTab const& values)
{
    m_symbol_values.push_back(sym_value_t(kwd,&values));
}


//
//
//
SymTab const* ParseArgs::find_values_for_kwd(unsigned kwd) const
{
    JAG_PRECONDITION(kwd != INVALID_KWD);
    sym_values_t::const_iterator it =
        std::find_if(m_symbol_values.begin(),
                     m_symbol_values.end(),
                     lambda::bind(&sym_value_t::first,lambda::_1)==kwd);

    if (it!=m_symbol_values.end())
        return it->second;

    return 0;
}



// ---------------------------------------------------------------------------
//                         class ParsedResult
//

//
//
//
ParsedResult::ParsedResult(parse_info_ptr_t const& parse_info)
    : m_parse_info(parse_info)
{
}


//
//
//
ParsedResult::char_range_t const* ParsedResult::find(unsigned symbol, int pos) const
{
    typedef ParsedStorage PD;

    PD::options_t::const_iterator it =
        m_parse_info->data.m_options.find(PD::kwd_t(symbol, pos));

    if (it == m_parse_info->data.m_options.end()) {
        return 0;
    }

    return &it->second;
}


//
//
//
bool ParsedResult::has_symbol(unsigned symbol) const
{
    char_range_t const* str = find(symbol, 0);
    return str;
}


      typedef std::pair<unsigned, int>             kwd_t;
      typedef std::pair<char const*, char const*>  value_t;
      typedef std::map<kwd_t, value_t>             options_t;
      typedef std::set<unsigned>                   explicit_values_t;

      options_t               m_options;
      explicit_values_t       m_values;


namespace
{
  // functor deciding whether a map item has a certain key
  struct is_key_t
  {
      unsigned m_key;
      is_key_t(unsigned key) : m_key(key) {}
      bool operator()(ParsedStorage::options_t::value_type const& val) const {
          return val.first.first == m_key;
      }
  };
}


//
//
//
int ParsedResult::num_values(unsigned symbol) const
{
    return std::count_if(
        m_parse_info->data.m_options.begin(),
        m_parse_info->data.m_options.end(),
        is_key_t(symbol));
}


//
//
//
bool ParsedResult::has_explicit_value(unsigned symbol) const
{
    typedef ParsedStorage::explicit_values_t::const_iterator iterator;
    iterator it = m_parse_info->data.m_values.find(symbol);
    return it != m_parse_info->data.m_values.end();
}

//
//
//
unsigned ParsedResult::explicit_value(unsigned default_) const
{
    if (m_parse_info->data.m_values.empty())
        return default_;

    JAG_ASSERT_MSG(m_parse_info->data.m_values.size()==1,
                   "more then one explicit values, the first one is taken");
    return *m_parse_info->data.m_values.begin();
}


//
//
//
template<class T, class P>
T ParsedResult::string_to_T(P const& parser, unsigned symbol, T default_, int pos) const
{
    char_range_t const* str = find(symbol, pos);
    if (!str)
        return default_;

    T retval=0;
    if (!parse(str->first, str->second, parser[assign_a(retval)], space_p).full)
    {
        std::string val(str->first, str->second);
        throw exception_invalid_value(msg_opt_parser_value_err(val.c_str())) << JAGLOC;
    }

    return retval;
}

//
//
//
template<>
Double ParsedResult::to_(unsigned symbol, double default_, int pos) const
{
    return string_to_T<double>(real_p, symbol, default_, pos);
}

//
//
//
template<>
int ParsedResult::to_(unsigned symbol, int default_, int pos) const
{
    return string_to_T<int>(int_p, symbol, default_, pos);
}

//
//
//
template<>
string_32_cow
ParsedResult::to_(unsigned symbol, char const* default_, int pos) const
{
    char_range_t const* str = find(symbol, pos);
    if (!str)
        return default_ ? string_32_cow(default_) : string_32_cow();

    return string_32_cow(str->first, str->second);
}


//
// specialization for usigned is used for keywords with fixed values
//
template<>
unsigned
ParsedResult::to_(unsigned symbol, unsigned default_, int pos) const
{
    JAG_UNUSED_FUNCTION_ARGUMENT(pos);
    JAG_PRECONDITION(pos==0);

    typedef ParsedStorage::fixed_values_t::const_iterator iter;
    iter it = m_parse_info->data.m_fixed_values.find(symbol);
    if (it == m_parse_info->data.m_fixed_values.end())
        return default_;

    return it->second;
}

}}


/** EOF @file */

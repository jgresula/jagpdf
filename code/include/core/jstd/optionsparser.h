// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef OPTIONSPARSER_JG920_H__
#define OPTIONSPARSER_JG920_H__

#include <external/flex_string/string.h>
#include <core/generic/assert.h>
#include <msg_jstd.h>
#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace jag {
namespace jstd {

// fwd
namespace detail{ struct ParseInfo; }


// metafunction giving return type for ParsedResult generic to_() method
template<class T> struct parsed_default_t { typedef T type; };
template<> struct parsed_default_t<string_32_cow> { typedef char const* type; };

//
//
//
class ParsedResult
{
public:
    typedef std::pair<char const*, char const*> char_range_t;
    typedef boost::shared_ptr<detail::ParseInfo> parse_info_ptr_t;

public:
    enum {NO_EXPL_VALUE=0xffffffff};
    ParsedResult(parse_info_ptr_t const& parse_info);
    ParsedResult() {}

public:
    template<class U>
    U to_(unsigned symbol,
          typename parsed_default_t<U>::type = typename parsed_default_t<U>::type(),
          int pos = 0) const;

    bool has_explicit_value(unsigned) const;
    unsigned explicit_value(unsigned defautl_=NO_EXPL_VALUE) const;
    bool has_symbol(unsigned) const;
    int num_values(unsigned) const;

private:
    char_range_t const* find(unsigned symbol, int pos) const;
    template<class T, class P>
    T string_to_T(P const& parser, unsigned symbol, T default_, int pos=0) const;

private:
    friend bool is_valid(ParsedResult const&);
    parse_info_ptr_t   m_parse_info;
};

//
//
//
inline bool is_valid(ParsedResult const& o)
{
    return o.m_parse_info;
}


//
//
//
typedef boost::spirit::classic::symbols<unsigned> SymTab;
class ParseArgs
{
public:
    ParseArgs(SymTab const* kwds, SymTab const* expl=0);
    void use_values_for_symbol(unsigned, SymTab const& values);

public:
    SymTab const& kwds() const { return m_kwds; }
    SymTab const& expl_vals() const { return m_explicit; }
    SymTab const* find_values_for_kwd(unsigned) const;

private:
    SymTab const& m_kwds;
    SymTab const& m_explicit;

    // The following lines associate some keywords with a predefined set of
    // values. The association is kept in an unsorted vector as the number of
    // items is supposed to be low.
    typedef std::pair<unsigned, SymTab const*> sym_value_t;
    typedef std::vector<sym_value_t>           sym_values_t;
    sym_values_t                               m_symbol_values;

    static const SymTab s_empty;
};


//
//
//
ParsedResult parse_options(char const* options,
                           ParseArgs const& args);


// ---------------------------------------------------------------------------
//                      Helper functions
//

//
//
//
template<class T>
void parse_array(ParsedResult const& p,
                 unsigned key,
                 std::vector<T>& array,
                 bool allow_empty = true,
                 int required_length = -1)
{
    JAG_PRECONDITION(required_length);

    int len = p.num_values(key);
    if (!len)
    {
        if (!allow_empty)
        {
            throw exception_invalid_value(
                msg_opt_not_specified()) << JAGLOC;
        }
        return;
    }

    if ((required_length > 0) && (len != required_length))
    {
        throw exception_invalid_value(
            msg_opt_length_differs()) << JAGLOC;
    }

    array.resize(len);
    for(int i=0; i<len; ++i)
        array[i] = p.to_<T>(key, T(), i);
}

//
//
//
template<class T>
bool parse_array(ParsedResult const& p,
                 unsigned key,
                 T* array,
                 bool allow_empty,
                 int required_length)
{
    JAG_PRECONDITION(required_length>0);
    int len = p.num_values(key);
    if (!len)
    {
        if (!allow_empty)
        {
            throw exception_invalid_value(
                msg_opt_not_specified()) << JAGLOC;
        }
        return false;
    }

    if (len != required_length)
    {
        throw exception_invalid_value(
            msg_opt_length_differs()) << JAGLOC;
    }

    for(int i=0; i<len; ++i)
        array[i] = p.to_<T>(key, T(), i);

    return true;
}


}}

#endif // OPTIONSPARSER_JG920_H__
/** EOF @file */

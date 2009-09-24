// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "destination.h"
#include "docwriterimpl.h"
#include "objfmt.h"
#include "indirectobjectref.h"
#include <msg_pdflib.h>
#include <core/errlib/errlib.h>
#include <core/errlib/errlib.h>
#include <core/generic/floatpointtools.h>

#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/variant/get.hpp>
#include <limits>

using namespace boost::spirit::classic;
using namespace boost;
using namespace jag::jstd;

namespace jag {
namespace pdf {

namespace
{
  char const* dest_syms[DestinationDef::DEST_LAST_ITEM]=  {
      "left", "top", "zoom", "page", "mode", "right", "bottom"};

  struct DestKeys
      : public SymTab
  {
      DestKeys()
      {
          for(int i=0; i<DestinationDef::DEST_LAST_ITEM; ++i)
              add(dest_syms[i], i);
      }
  } g_destination_syms;


  enum {XYZ, FIT, FITH, FITV, FITR, FITB, FITBH, FITBV, FIT_LAST_ITEM};
  char const* fit_modes[FIT_LAST_ITEM]= {
      "XYZ", "Fit", "FitH", "FitV", "FitR", "FitB", "FitBH", "FitBV"};

  struct FitModes
      : public SymTab
  {
      FitModes()
      {
          for(int i=0; i<FIT_LAST_ITEM; ++i)
              add(fit_modes[i], i);
      }

  } g_fit_modes;

}



///////////////////////////////////////////////////////////////////////////
// class Destination

const Double DestinationDef::no_change = std::numeric_limits<Double>::max();

//
//
//
DestinationDef::DestinationDef(DocWriterImpl& doc)
    : m_doc(&doc)
{
}

///
/// Stores the passed string and parses it. The string needs to be stored as the
/// lifetime of m_parsed and the parsed string should be the same.
///
DestinationDef::DestinationDef(DocWriterImpl& doc, char const* dest)
    : m_doc(&doc)
    , m_strdef(dest)
    , m_parsed(parse_options(m_strdef.c_str(), ParseArgs(&g_destination_syms)))

{
    JAG_PRECONDITION(dest);
}


///
///
///
void DestinationDef::set_double(unsigned sym, Double val)
{
    JAG_PRECONDITION_MSG(sym<NUM_EXPLICIT_SYMBOLS, "order of enum values has changed");
    if (!equal_doubles(val, DestinationDef::no_change))
    {
        m_mask.set(sym);
        m_explicit_values[sym]=val;
    }
}

///
/// Checks that the required symbol is specified either explicitly or in the
/// string spec.
///
void DestinationDef::ensure_key(unsigned sym) const
{
    if (!m_mask.test(sym)
        && !(is_valid(m_parsed)&&m_parsed.has_symbol(sym)))
    {
        throw exception_invalid_value(msg_option_not_found_ex(dest_syms[sym])) << JAGLOC;
    }
}

///
///
///
void DestinationDef::output_double(ObjFmt& fmt, unsigned sym) const
{
    double val;
    if (m_mask.test(sym))
    {
        JAG_ASSERT(sym<NUM_EXPLICIT_SYMBOLS);
        val = get<double>(m_explicit_values[sym]);
    }
    else if (is_valid(m_parsed) && m_parsed.has_symbol(sym))
    {
        val = m_parsed.to_<double>(sym, 0.0);
    }
    else
    {
        fmt.null();
        return;
    }

    if (sym == TOP && m_doc->is_topdown())
    {
        // handle top-down
        const int page_num = to_<int>(PAGE);
        JAG_ASSERT(page_num < m_doc->page_number()); // should be catched earlier
        Double pheight = m_doc->page_height(page_num);
        val = pheight - val;
    }
    fmt.output(val);

}

///
///
///
void DestinationDef::set_page_num(int page_num, bool always)
{
    if (always || !(is_valid(m_parsed) && m_parsed.has_symbol(PAGE)))
    {
        m_mask.set(PAGE);
        m_explicit_values[PAGE]=page_num;
    }
}


///
///
///
void DestinationDef::set_xyz(Double left, Double top, Double z)
{
    set_string(MODE, "XYZ");
    set_double(LEFT, left);
    set_double(TOP, top);
    set_double(ZOOM, z);
}

///
///
///
void DestinationDef::set_string(unsigned sym, char const* val)
{
    JAG_PRECONDITION(sym<NUM_EXPLICIT_SYMBOLS);
    m_mask.set(sym);
    m_explicit_values[sym]=val;
}

///
/// Retrieves the value associated with the given symbol. First, it tries to
/// locate the value in the explicit values and then in the string.
///
template<class T>
T DestinationDef::to_(unsigned sym) const
{
    if (m_mask.test(sym))
        return get<T>(m_explicit_values[sym]);

    if (!is_valid(m_parsed))
        JAG_INTERNAL_ERROR;

    return m_parsed.to_<T>(sym);
}


///
///
///
void DestinationDef::output_object(ObjFmt& fmt)
{
    ensure_key(PAGE);
    ensure_key(MODE);

    const int page_num = to_<int>(PAGE);
    if (page_num >= m_doc->page_number())
        throw exception_invalid_value(msg_no_page_for_destination(page_num))
            << JAGLOC;

    unsigned const* fit_mode = find(g_fit_modes, to_<string_32_cow>(MODE).c_str());
    if (!fit_mode)
        throw exception_invalid_value(
            msg_option_invalid_value(dest_syms[MODE])) << JAGLOC;

    JAG_ASSERT(*fit_mode>=0 && *fit_mode<FIT_LAST_ITEM);

    fmt
        .array_start()
        .ref(m_doc->page_ref(page_num))
        .output(fit_modes[*fit_mode])
        .space()
    ;

    switch (*fit_mode)
    {
    case XYZ:
        output_double(fmt, LEFT);
        fmt.space();
        output_double(fmt, TOP);
        fmt.space();
        output_double(fmt, ZOOM);
        break;

    case FIT:
    case FITB:
        break;

    case FITBH:
    case FITH:
        output_double(fmt, TOP);
        break;

    case FITV:
    case FITBV:
        output_double(fmt, LEFT);
        break;

    case FITR:
        ensure_key(LEFT);
        ensure_key(BOTTOM);
        ensure_key(RIGHT);
        ensure_key(TOP);
        output_double(fmt, LEFT);
        fmt.space();
        output_double(fmt, BOTTOM);
        fmt.space();
        output_double(fmt, RIGHT);
        fmt.space();
        output_double(fmt, TOP);
        break;
    }

    fmt.array_end();
}




}} // namespace jag::

/** EOF @file */

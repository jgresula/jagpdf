// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef DESTINATION_JG1441_H__
#define DESTINATION_JG1441_H__

#include <interfaces/stdtypes.h>
#include "interfaces/directobject.h"
#include <core/jstd/optionsparser.h>
#include <external/flex_string/string.h>
#include <boost/array.hpp>
#include <boost/variant.hpp>
#include <bitset>
#include <string>


namespace jag {
namespace pdf {

class DocWriterImpl;

///
/// Represents a destination object. The data for the object can be parsed from
/// a string or are passed explicitily through an object method. When a value is
/// present in the string and was also specified explicitly via a method call
/// then the latter one is used.
///
class DestinationDef
    : public IDirectObject
{
public: //IDirectObject
    void output_object(ObjFmt& fmt);

public:
    explicit DestinationDef(DocWriterImpl& doc);
    DestinationDef(DocWriterImpl& doc, char const* dest);
    void set_page_num(int page_num, bool always=true);
    void set_xyz(Double left, Double top, Double z=no_change);

    static const Double no_change;

    // do not change the order
    enum {LEFT, TOP, ZOOM, PAGE, MODE, RIGHT, BOTTOM, DEST_LAST_ITEM};

private:
    void set_double(unsigned sym, Double val);
    void set_string(unsigned sym, char const* val);
    void output_double(ObjFmt& fmt, unsigned sym) const;
    void ensure_key(unsigned sym) const;

    template<class T> T to_(unsigned sym) const;


private:
    enum {NUM_EXPLICIT_SYMBOLS=MODE+1};

    DocWriterImpl*      m_doc;
    std::string         m_strdef;
    jstd::ParsedResult  m_parsed;
    std::bitset<32>     m_mask;

    typedef boost::variant<int,Double,string_32_cow> item_t;
    boost::array<item_t,NUM_EXPLICIT_SYMBOLS> m_explicit_values;
};

}} // namespace jag::pdf

#endif // DESTINATION_JG1441_H__
/** EOF @file */

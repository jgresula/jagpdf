// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FONTSPECIMPL_H_JAG_2241__
#define __FONTSPECIMPL_H_JAG_2241__

#include <core/generic/assert.h>
#include <resources/interfaces/fontspec.h>
#include <resources/interfaces/font.h>
#include <resources/interfaces/resourcehandle.h>
#include <string>

namespace jag { namespace resources
{
struct CharEncodingRecord;

class FontSpecImpl
    : public IFontSpec
{
public:
    FontSpecImpl();

public: //IFontSpec
    void filename(Char const* val);
    void facename(Char const* val);
    void encoding(Char const* val);
    void size(Double val) { m_pointsize=val; }
    void bold(Int val) { m_bold=val; }
    void italic(Int val) { m_italic=val; }
    void adobe14(Int yes) { m_adobe14=yes; }

public:
    Char const* facename() const { return m_facename.c_str(); }
    std::string const& facename_str() const { return m_facename; }
    Char const* encoding() const { return m_encoding.c_str(); }
    Char const* filename() const { return m_filename.c_str(); }
    Char const* filename2() const { return m_filename2.c_str(); }
    const Double size() const { return m_pointsize; }
    Int bold() const { return m_bold; }
    Int italic() const { return m_italic; }
    std::string fullname() const;
    bool adobe14() const { return m_adobe14; }
    bool defined_by_filename() const { return !m_filename.empty(); }
    boost::intrusive_ptr<FontSpecImpl> clone() const;

public: //internal setters
    typedef std::pair<char const*, char const*> str_t;
    void filename(str_t const& str);
    void filename2(str_t const& str);
    void facename(str_t const& str);
    void encoding(str_t const& str);

public:
    void ensure_consistency() const;

private:
    // !! when adding a new member, do not forget to update clone()
    std::string m_facename;
    std::string m_encoding;
    std::string m_filename;
    std::string m_filename2;
    Double m_pointsize;
    Int m_bold;
    Int m_italic;
    Int m_adobe14;
};

}} //namespace jag::resources

#endif //__FONTSPECIMPL_H_JAG_2241__

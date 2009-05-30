// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PDFFONTDATA_H_JAG_2126__
#define __PDFFONTDATA_H_JAG_2126__

#include <resources/interfaces/typeface.h>
#include <resources/interfaces/charencoding.h>
#include <boost/intrusive_ptr.hpp>
#include <string>

namespace jag
{
class IFontEx;
class ITypeface;

namespace pdf
{

//////////////////////////////////////////////////////////////////////////
class PDFFontData
{
public:
    enum FontType { SIMPLE_FONT, COMPOSITE_FONT };

    PDFFontData(IFontEx const& font, bool force_cid);
    bool synthesized_bold() const { return m_synthesized_bold; }
    bool synthesized_italic() const { return m_synthesized_italic; }
    ITypeface const& typeface() const { return m_typeface; }
    FontType font_type() const { return m_font_type; }

    friend bool operator<(PDFFontData const& lhs, PDFFontData const& rhs);

private:
    const bool m_synthesized_bold;
    const bool m_synthesized_italic;
    ITypeface const& m_typeface;
    const FontType m_font_type;
};

bool operator==(PDFFontData const& lhs, PDFFontData const& rhs);
bool operator<(PDFFontData const& lhs, PDFFontData const& rhs);
size_t hash_value(PDFFontData const& obj);



//////////////////////////////////////////////////////////////////////////
class PDFFontDictData
    : private PDFFontData
{
public:
    // PDFFontData
    using PDFFontData::synthesized_bold;
    using PDFFontData::synthesized_italic;
    using PDFFontData::typeface;
    using PDFFontData::font_type;

    PDFFontDictData(IFontEx const& font, bool force_cid);
    EnumCharacterEncoding font_encoding() const { return m_font_encoding; }
    PDFFontData const& font_data() const { return *this; }

    friend bool operator<(PDFFontDictData const& lhs,
                          PDFFontDictData const& rhs);

private:
    // encoding field of the pdf font dictionary, the values as are same as
    // EnumCharacterEncoding, but some encodings are mapped to H-Identity
    const EnumCharacterEncoding m_font_encoding;
};


/// predicate based on pdf font dictionary encoding
bool operator<(PDFFontDictData const& lhs, PDFFontDictData const& rhs);


}} //namespace jag::pdf

#endif //__PDFFONTDATA_H_JAG_2126__

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "pdffont.h"
#include "fontdictionary.h"
#include "fontmanagement.h"
#include <core/generic/assert.h>
#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/ref.hpp>


namespace tuples = boost::tuples;
using namespace jag::jstd;


namespace jag {
namespace pdf {

// ---------------------------------------------------------------------------
//                class PDFFont

//
//
//
PDFFont::PDFFont(FontManagement& font_mgm,
                 IFontEx const* font,
                 FontDictionary* font_dict)
    : m_font(font)
    , m_font_dict(font_dict)
    , m_font_mgm(font_mgm)
    , m_conv_ctrl(font_dict && (ENC_IDENTITY == font_dict->fdict_data().font_encoding())
                  ? font->encoding_canonical()
                  : "")
{
    JAG_PRECONDITION(m_font);
    JAG_PRECONDITION(m_font->has_multiple_encondings() || font_dict);
}


//
//
//
bool PDFFont::has_multiple_encondings() const
{
    return m_font->has_multiple_encondings();
}


//
//
//
PDFFontIterator PDFFont::create_iterator(Char const* begin,
                                         Char const* end) const
{
    JAG_PRECONDITION(has_multiple_encondings());
    return PDFFontIterator(*this, begin, end);
}


//
//
//
UnicodeConverter* PDFFont::acquire_converter() const
{
    JAG_PRECONDITION(!has_multiple_encondings());
    return m_conv_ctrl.acquire_converter();
}


//
//
//
void PDFFont::release_converter() const
{
    JAG_PRECONDITION(!has_multiple_encondings());
    m_conv_ctrl.release_converter();
}


//
//
//
bool operator<(PDFFont const& lhs, PDFFont const& rhs)
{
    return
        tuples::tie(lhs.m_font, lhs.m_font_dict)
        <
        tuples::tie(rhs.m_font, rhs.m_font_dict);
}


// ---------------------------------------------------------------------------
//            class PDFFontIterator

//
//
//
PDFFontIterator::PDFFontIterator(PDFFont const& obj,
                                 Char const* begin, Char const* end)
    : m_obj(&obj)
    , m_iter(obj.font()->unicode_to_8bit()->create_iterator(begin, end))
{}


//
//
//
PDFFont const* PDFFontIterator::next(std::vector<Char>& str)
{
    EnumCharacterEncoding* enc = m_iter.next(str);
    if (!enc)
        return 0;

    PDFFont::EncMap& map = m_obj->m_enc_to_font;
    PDFFont::EncMap::iterator it = map.find(*enc);
    if (it == map.end())
    {
        IFontEx const& fnt = m_obj->font()->font_for_encoding(*enc);
        PDFFont const& pdf_fnt = m_obj->m_font_mgm.font_load(fnt);
        map.insert(PDFFont::EncMap::value_type(*enc, &pdf_fnt));
        return &pdf_fnt;
    }

    return it->second;
}





}} //namespace jag::pdf

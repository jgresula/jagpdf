// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "pdffontdata.h"
#include "encdifferences.h"
#include <core/generic/assert.h>
#include <resources/interfaces/font.h>
#include <resources/typeman/fontutils.h>
#include <resources/interfaces/typefaceops.h>
#include <resources/interfaces/charencoding.h>
#include <boost/functional/hash.hpp>
#include "boost/tuple/tuple_comparison.hpp"

namespace tuples = boost::tuples;

namespace jag {
namespace pdf {

//////////////////////////////////////////////////////////////////////////
// class PDFFontData
//////////////////////////////////////////////////////////////////////////


/// For given encoding decides whether it is output as a simple or composite font
PDFFontData::FontType find_out_font_type(
    EnumCharacterEncoding enc,
    bool force_cid, ITypeface const& face)
{
    switch(face.type())
    {
    case FACE_TYPE_1_ADOBE_STANDARD:
        switch(enc)
        {
        case ENC_CP_1252:   //win-ansi
        case ENC_PDF_STANDARD:
        case ENC_BUILTIN:
            return PDFFontData::SIMPLE_FONT;
        default:
            ;
        }
        if (supports_differences(enc))
            return PDFFontData::SIMPLE_FONT;


    case FACE_TRUE_TYPE:
        if (force_cid)
            break;

        switch(enc)
        {
        case ENC_CP_1252:   //win-ansi
            return PDFFontData::SIMPLE_FONT;
        default:
            ;
        }
        break;

    default:
        ;
    }

    return PDFFontData::COMPOSITE_FONT;
}

/**
 * @brief Determines whether given encoding is recoginezed by PDF.
 *
 * @param enc generic encoding
 * @return ENC_IDENTITY if not recognized by PDF, enc otherwise
 */
EnumCharacterEncoding find_out_font_dict_encoding(
    EnumCharacterEncoding enc,
    bool force_cid,
    ITypeface const& face)
{
    EnumCharacterEncoding result = ENC_IDENTITY;
    switch (face.type())
    {
    case FACE_TYPE_1_ADOBE_STANDARD:
        switch(enc)
        {
        case ENC_CP_1252:   //win-ansi
        case ENC_PDF_STANDARD:
        case ENC_BUILTIN:
            result = enc;
        default:
            ;
        }
        if (supports_differences(enc))
        {
            result = enc;
        }
        break;


    case FACE_TRUE_TYPE:
        if (force_cid)
            break;

        switch(enc)
        {
        case ENC_CP_1252:

        case ENC_SHIFT_JIS:
        case ENC_GB2312:
        case ENC_HANGEUL:
        case ENC_BIG5:
            result = enc;
            break;
        default:
            ;
        }
        break;

    default:
        ;
    }

    return result;
}


//////////////////////////////////////////////////////////////////////////
  PDFFontData::PDFFontData(IFontEx const& font, bool force_cid)
    : m_synthesized_bold(resources::synthesized_bold(font))
    , m_synthesized_italic(resources::synthesized_italic(font))
    , m_typeface(font.typeface())
    , m_font_type(find_out_font_type(font.encoding_id(), force_cid, m_typeface))
{
}


//////////////////////////////////////////////////////////////////////////
bool operator==(PDFFontData const& lhs, PDFFontData const& rhs)
{
    return
           lhs.synthesized_bold() == rhs.synthesized_bold()
        && lhs.synthesized_italic() == rhs.synthesized_italic()
        && lhs.typeface() == rhs.typeface()
        && lhs.font_type() == rhs.font_type()
    ;
}

//////////////////////////////////////////////////////////////////////////
bool operator<(PDFFontData const& lhs, PDFFontData const& rhs)
{
    return
        tuples::tie(lhs.m_typeface,
                    lhs.m_synthesized_bold,
                    lhs.m_synthesized_italic,
                    lhs.m_font_type)
        <
        tuples::tie(rhs.m_typeface,
                    rhs.m_synthesized_bold,
                    rhs.m_synthesized_italic,
                    rhs.m_font_type)
    ;
}


//////////////////////////////////////////////////////////////////////////
size_t hash_value(PDFFontData const& obj)
{
    size_t seed = 0;
    boost::hash_combine(seed, obj.synthesized_bold());
    boost::hash_combine(seed, obj.synthesized_italic());
    boost::hash_combine(seed, &obj.typeface());
    boost::hash_combine(seed, obj.font_type());

    return seed;
}





//////////////////////////////////////////////////////////////////////////
// class PDFFontDictData
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
 PDFFontDictData::PDFFontDictData(IFontEx const& font, bool force_cid)
    : PDFFontData(font, force_cid)
    , m_font_encoding(find_out_font_dict_encoding(font.encoding_id(), force_cid, font.typeface()))
{
}

//////////////////////////////////////////////////////////////////////////
bool operator<(PDFFontDictData const& lhs, PDFFontDictData const& rhs)
{
    PDFFontData const& lhs_base = static_cast<PDFFontData const&>(lhs);
    PDFFontData const& rhs_base = static_cast<PDFFontData const&>(rhs);

    return
           lhs_base < rhs_base
        || (lhs_base == rhs_base && lhs.font_encoding() < rhs.font_encoding())
    ;
}


}} //namespace jag::pdf

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//
#include "precompiled.h"
#include <resources/typeman/fontimpl.h>
#include <resources/typeman/charencrecord.h>
#include "fontspecimpl.h"
#include <interfaces/execcontext.h>
#include <interfaces/configinternal.h>
#include <resources/typeman/typefaceimpl.h>
#include <resources/interfaces/typeman.h>
#include <resources/interfaces/typefaceops.h>
#include <core/generic/scopeguard.h>
#include <core/errlib/errlib.h>
#include <core/jstd/encodinghelpers.h>
#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/intrusive_ptr.hpp>


using namespace jag::jstd;
using namespace boost;

namespace jag {
namespace resources {

// ---------------------------------------------------------------------------
//                 class FontImpl


//////////////////////////////////////////////////////////////////////////
FontImpl::FontImpl(FontSpecImpl const& fspec,
                   ITypeface const& face,
                   CharEncodingRecord const& enc_rec,
                   IExecContext const& exec_ctx)
    : m_typeface(face)
    , m_pointsize(fspec.size())
    , m_bold(face.bold())
    , m_italic(face.italic())
    , m_enc_rec(enc_rec)
    , m_conv_ctrl(enc_rec.encoding_canonical)
    , m_coef(m_pointsize / m_typeface.metrics().units_per_EM)
    , m_kerning(exec_ctx.config().get_int("text.kerning"))
{
}

//////////////////////////////////////////////////////////////////////////
EnumCharacterEncoding FontImpl::encoding_id() const
{
    return m_enc_rec.encoding;
}

//////////////////////////////////////////////////////////////////////////
Char const* FontImpl::encoding_canonical() const
{
    return m_enc_rec.encoding_canonical;
}


//
//
//
Double FontImpl::height() const
{
    return m_coef*m_typeface.metrics().baseline_distance;
}


//
//
// 
Double FontImpl::glyph_width(UInt16 glyph_index) const
{
    return m_coef * m_typeface.gid_horizontal_advance(glyph_index);
}


///
///
///
Double FontImpl::horizontal_advance_dbg(jag::Char const* text, jag::ULong length) const
{
    // If a font built-in encoding is used then NULL is returned from
    // acquire_converter(). In such case the characters are used directly
    // without any conversion.
    UnicodeConverter* conv(m_conv_ctrl.acquire_converter());
    ON_BLOCK_EXIT_OBJ(m_conv_ctrl, &jstd::UConverterCtrl::release_converter);

    Double result = 0.0;
    Char const*const end = text + length;
    if (conv)
    {
        if (m_kerning)
        {
            Int prev = 0;
            while (text != end)
            {
                Int cp = conv->next_code_point(&text, end);
                result += m_typeface.char_horizontal_advance(cp);
                //JAG_ASSERT((prev!=32 && cp!=32) || 0 == m_typeface.kerning_for_chars(prev, cp));
                result += m_typeface.kerning_for_chars(prev, cp);
                prev = cp;
            }
        }
        else
        {
            while (text != end)
            {
                Int cp = conv->next_code_point(&text, end);
                result += m_typeface.char_horizontal_advance(cp);
            }
        }
    }
    else
    {
        // built-it font encoding
        while (text != end)
            result += m_typeface.char_horizontal_advance(*text++);
    }

    return m_coef * result;
}

//
//
//
Double FontImpl::kerning_for_gids(UInt left, UInt right) const
{
    Int kern = m_typeface.kerning_for_gids(left, right);
    if (kern)
        return m_coef * kern;

    return 0.0;
}

//
//
// 
Double FontImpl::kerning_for_chars(Int left, Int right) const
{
    Int kern = m_typeface.kerning_for_chars(left, right);
    if (kern)
        return m_coef * kern;

    return 0.0;
}


///
///
///
Double FontImpl::advance(jag::Char const* text) const
{
    return horizontal_advance_dbg(text, strlen(text));
}

///
///
///
Double FontImpl::advance_r(jag::Char const* start, jag::Char const* end) const
{
    return horizontal_advance_dbg(start, end-start);
}




Int FontImpl::is_in_font_dbg(jag::Char const* text, jag::UInt length) const
{
    UnicodeConverter* conv(m_conv_ctrl.acquire_converter());
    JAG_ASSERT(conv);
    ON_BLOCK_EXIT_OBJ(m_conv_ctrl, &jstd::UConverterCtrl::release_converter);

    Char const*const end = text + length;
    while (text != end)
    {
        Int cp = conv->next_code_point(&text, end);
        if (!m_typeface.codepoint_to_gid(cp))
            return 0;
    }

    return 1;
}



//////////////////////////////////////////////////////////////////////////
bool operator==(FontImpl const& lhs, FontImpl const& rhs)
{
    return
           &lhs.m_typeface == &rhs.m_typeface
        && lhs.m_pointsize == rhs.m_pointsize
        && lhs.m_bold == rhs.m_bold
        && lhs.m_italic == rhs.m_italic
        && lhs.m_enc_rec.encoding == rhs.m_enc_rec.encoding
    ;
}

//////////////////////////////////////////////////////////////////////////
bool operator<(FontImpl const& lhs, FontImpl const& rhs)
{
    ITypeface const * lhs_typeface = &lhs.m_typeface;
    ITypeface const * rhs_typeface = &rhs.m_typeface;

    return
        tuples::tie(lhs_typeface, lhs.m_pointsize, lhs.m_bold, lhs.m_italic, lhs.m_enc_rec.encoding)
        <
        tuples::tie(rhs_typeface, rhs.m_pointsize, rhs.m_bold, rhs.m_italic, rhs.m_enc_rec.encoding)
    ;
}

//////////////////////////////////////////////////////////////////////////
size_t hash_value(FontImpl const& font)
{
    size_t seed = 0;
    boost::hash_combine(seed, font.m_pointsize);
    boost::hash_combine(seed, font.m_typeface);
    boost::hash_combine(seed, font.m_bold);
    boost::hash_combine(seed, font.m_italic);
    boost::hash_combine(seed, font.m_enc_rec.encoding);

    return seed;
}


// ---------------------------------------------------------------------------
//                 class MultiEncFontImpl


//
//
//
MultiEncFontImpl::MultiEncFontImpl(ITypeMan& typeman,
                                   IExecContext const& exec_ctx,
                                   FontSpecImpl const& fspec,
                                   EnumCharacterEncoding* enc_start,
                                   EnumCharacterEncoding* enc_end)
    : m_font_spec(fspec.clone())
    , m_from_unicode(new jstd::UnicodeToCP(enc_start,
                                             enc_end))
    , m_typeman(typeman)
    , m_exec_ctx(exec_ctx)
    , m_pointsize(fspec.size())
{
    JAG_PRECONDITION(enc_start);
    JAG_PRECONDITION(enc_start < enc_end);

    // load the first font and use its typeface for providing metrics
    font_for_encoding(*enc_start);
    IFontEx const* fnt = m_enc_to_font.find(*enc_start)->second;
    JAG_ASSERT(fnt);
    m_typeface = &fnt->typeface();

    m_coef = m_pointsize / m_typeface->metrics().units_per_EM;
    m_bold = m_typeface->bold();
    m_italic = m_typeface->italic();
}



//
//
//
IFontEx const&
MultiEncFontImpl::font_for_encoding(EnumCharacterEncoding enc) const
{
    FontMap::iterator it = m_enc_to_font.find(enc);
    if (it == m_enc_to_font.end())
    {
        intrusive_ptr<FontSpecImpl> fspec(m_font_spec->clone());
        fspec->encoding(encoding_from_id(enc));
        IFontEx const& fnt = m_typeman.font_load_spec(fspec, m_exec_ctx);
        m_enc_to_font.insert(FontMap::value_type(enc, &fnt));
        return fnt;
    }

    return *it->second;
}

//
//
//
Double MultiEncFontImpl::advance(jag::Char const* text) const
{
    return advance_r(text, text + strlen(text));
}


//
//
//
Double MultiEncFontImpl::advance_r(jag::Char const* start, jag::Char const* end) const
{
    Double result = 0.0;
    std::vector<Char> str;
    UnicodeToCPIterator it(
        m_from_unicode->create_iterator(start, end));

    for(;;)
    {
        EnumCharacterEncoding* enc = it.next(str);
        if (!enc)
            break;

        IFontEx const& fnt = font_for_encoding(*enc);
        result += fnt.horizontal_advance_dbg(&str[0], str.size());
    }

    return result;
}


Double MultiEncFontImpl::glyph_width(UInt16 /*glyph_index*/) const
{
    throw exception_invalid_operation() << JAGLOC;
}


//
//
//
bool operator<(MultiEncFontImpl const& lhs, MultiEncFontImpl const& rhs)
{
    FontSpecImpl const& l = *lhs.m_font_spec;
    FontSpecImpl const& r = *rhs.m_font_spec;

    // now only adobe 14 fonts are support
    JAG_ASSERT(l.adobe14() && r.adobe14());

    return
        tuples::tie(l.size(), l.facename_str())
        <
        tuples::tie(r.size(), r.facename_str());
}


//
// not implemented functions, an internal error if invoked
//
ITypeface const& MultiEncFontImpl::typeface() const {
    JAG_INTERNAL_ERROR;
}

EnumCharacterEncoding MultiEncFontImpl::encoding_id() const {
    JAG_INTERNAL_ERROR;
}

Char const* MultiEncFontImpl::encoding_canonical() const {
    JAG_INTERNAL_ERROR;
}

Double MultiEncFontImpl::horizontal_advance_dbg(jag::Char const*,
                                                jag::ULong) const {
    JAG_INTERNAL_ERROR;
}

Int MultiEncFontImpl::is_in_font_dbg(jag::Char const*,
                                     jag::UInt) const {
    JAG_INTERNAL_ERROR;
}

Double MultiEncFontImpl::kerning_for_gids(UInt, UInt) const
{
    JAG_INTERNAL_ERROR;
}

Double MultiEncFontImpl::kerning_for_chars(Int, Int) const
{
    JAG_INTERNAL_ERROR;
}


}} // namespace jag::resources

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FONTIMPL_H_JAG_2216__
#define __FONTIMPL_H_JAG_2216__

#include <core/jstd/uconverterctrl.h>
#include <resources/interfaces/font.h>
#include <resources/typeman/typefaceimpl.h>
#include <interfaces/stdtypes.h>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <map>

namespace jag {
class ITypeMan;
class IExecContext;

namespace resources {
class FontSpecImpl;
struct CharEncodingRecord;


//
//
//
class FontImpl
    : public IFontEx
{
public:
    FontImpl(FontSpecImpl const& fspec,
             ITypeface const& face,
             CharEncodingRecord const& enc_rec,
             IExecContext const& exec_ctx);

public:
    /** @name IFont implementation */
    //@{
    Int is_bold() const { return m_bold; }
    Int is_italic() const { return m_italic; }
    Double size() const { return m_pointsize; }
    Char const* family_name() const { return m_typeface.family_name(); }
    Char const* style_name() const { return m_typeface.style_name(); }
    Double advance(Char const* text) const;
    Double advance_r(jag::Char const* start, jag::Char const* end) const;
    Double glyph_width(UInt16 glyph_index) const;
    Double height() const;
    Double ascender() const { return m_coef*m_typeface.metrics().ascent; }
    Double descender() const { return m_coef*m_typeface.metrics().descent; }
    Double bbox_xmin() const { return m_coef*m_typeface.metrics().bbox_xmin; }
    Double bbox_ymin() const { return m_coef*m_typeface.metrics().bbox_ymin; }
    Double bbox_xmax() const { return m_coef*m_typeface.metrics().bbox_xmax; }
    Double bbox_ymax() const { return m_coef*m_typeface.metrics().bbox_ymax; }
    //@}

public: // IFontEx
    ITypeface const& typeface() const { return m_typeface; }
    EnumCharacterEncoding encoding_id() const ;
    Char const* encoding_canonical() const;
    Double horizontal_advance_dbg(Char const* text, ULong length) const;
    Int is_in_font_dbg(Char const* text, UInt length) const;
    Double kerning_for_gids(UInt left, UInt right) const;
    Double kerning_for_chars(Int left, Int right) const;
    bool has_multiple_encondings() const { return false; }
    jstd::UnicodeToCP* unicode_to_8bit() const { return 0; }
    IFontEx const& font_for_encoding(EnumCharacterEncoding /*enc*/) const {
        JAG_INTERNAL_ERROR;
    }

private:
    ITypeface const& m_typeface;

private:
    friend size_t hash_value(FontImpl const& font);
    friend bool operator==(FontImpl const& lhs, FontImpl const& rhs);
    friend bool operator<(FontImpl const& lhs, FontImpl const& rhs);

    // when adding a new font attribute, don't forget to update
    // - operators ==, < and hash_value
    Double      m_pointsize;
    Int       m_bold;
    Int       m_italic;
    CharEncodingRecord const& m_enc_rec;
    mutable jstd::UConverterCtrl   m_conv_ctrl;

    const Double m_coef;
    const bool m_kerning;
};

// operators
size_t hash_value(FontImpl const& font);
bool operator==(FontImpl const& lhs, FontImpl const& rhs);
bool operator<(FontImpl const& lhs, FontImpl const& rhs);
inline bool operator!=(FontImpl const& lhs, FontImpl const& rhs) {
    return !(lhs==rhs);
}



//
//
//
class MultiEncFontImpl
    : public IFontEx
{
public:
    MultiEncFontImpl(ITypeMan& typeman,
                     IExecContext const& exec_ctx,
                     FontSpecImpl const& fspec,
                     EnumCharacterEncoding* enc_start,
                     EnumCharacterEncoding* enc_end);

public: // IFont
    // now not implemented, see FontImpl for reference when implementing
    Int is_bold() const { return m_bold; }
    Int is_italic() const { return m_italic; }
    Double size() const { return m_pointsize; }
    Char const* family_name() const { return m_typeface->family_name(); }
    Char const* style_name() const { return m_typeface->style_name(); }
    Double advance(jag::Char const* text) const;
    Double advance_r(jag::Char const* start, jag::Char const* end) const;
    Double glyph_width(UInt16 glyph_index) const;

    Double height() const {
        return m_coef * m_typeface->metrics().baseline_distance; }

    Double ascender() const {
        return m_coef * m_typeface->metrics().ascent; }

    Double descender() const {
        return m_coef * m_typeface->metrics().descent; }

    Double bbox_xmin() const {
        return m_coef * m_typeface->metrics().bbox_xmin; }

    Double bbox_ymin() const {
        return m_coef * m_typeface->metrics().bbox_ymin; }

    Double bbox_xmax() const {
        return m_coef * m_typeface->metrics().bbox_xmax; }

    Double bbox_ymax() const {
        return m_coef * m_typeface->metrics().bbox_ymax; }

public: // internal error if any of those is invoked
    ITypeface const& typeface() const;
    EnumCharacterEncoding encoding_id() const;
    Char const* encoding_canonical() const;
    Double horizontal_advance_dbg(jag::Char const* text, jag::ULong length) const;
    Int is_in_font_dbg(jag::Char const* text, jag::UInt length) const;
    Double kerning_for_gids(UInt left, UInt right) const;
    Double kerning_for_chars(Int left, Int right) const;

public:
    bool has_multiple_encondings() const { return true; }
    IFontEx const& font_for_encoding(EnumCharacterEncoding enc) const;
    jstd::UnicodeToCP* unicode_to_8bit() const { return m_from_unicode.get(); }

private:
    friend bool operator<(MultiEncFontImpl const& lhs, MultiEncFontImpl const& rhs);

    // when adding a new font attribute, don't forget to update
    // - operator<
    boost::intrusive_ptr<FontSpecImpl> m_font_spec;
    boost::scoped_ptr<jstd::UnicodeToCP> m_from_unicode;
    ITypeMan& m_typeman;
    IExecContext const& m_exec_ctx;
    typedef std::map<EnumCharacterEncoding, IFontEx const*> FontMap;
    mutable FontMap m_enc_to_font;

    ITypeface const* m_typeface;
    Double m_pointsize;
    Int m_bold;
    Int m_italic;
    Double m_coef;
};

bool operator<(MultiEncFontImpl const& lhs, MultiEncFontImpl const& rhs);


}} // namespace jag::resources



#endif //__FONTIMPL_H_JAG_2216__

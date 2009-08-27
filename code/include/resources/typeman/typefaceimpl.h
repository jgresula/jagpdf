// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TYPEFACEIMPL_H_JAG_1318__
#define __TYPEFACEIMPL_H_JAG_1318__

#include <core/generic/noncopyable.h>
#include <core/jstd/md5.h>
#include <resources/interfaces/typeface.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>


#include <ft2build.h>
#include FT_FREETYPE_H

// reflect type1 font in the typeface implementation
// still many things is missing


namespace jag
{
// fwd
class IStreamInput;

namespace resources
{
// fwd
class FTOpenArgs;


/// TypefaceImpl representation.
class TypefaceImpl
    : public ITypeface
{
public:
    /** @name ITypeface */
    //@{
    FaceType type() const { return m_type; }
    TypefaceMetrics const& metrics() const;

    Panose const& panose() const { return m_panose; }
    Int width_class() const { return m_width_class; }
    Int weight_class() const { return m_weight_class; }
    Double italic_angle() const { return m_italic_angle; }
    Int fixed_width() const { return m_fixed_width; }
    Int bold() const;
    Int italic() const;

    Char const* postscript_name() const;
    Char const* family_name() const;
    Char const* style_name() const;
    std::string full_name() const;

    Int char_horizontal_advance(Int codepoint) const;
    Int gid_horizontal_advance(UInt gid) const;
    Int kerning_for_gids(UInt left, UInt right) const;
    Int kerning_for_chars(Int left, Int right) const;

    Int can_embed() const { return m_can_embed; }
    Int can_subset() const { return m_can_subset; }
    UInt16 codepoint_to_gid(Int codepoint) const;
    FaceCharIterator char_iterator() const;

    Char const* encoding_scheme() const { return ""; }
    int num_streams() const;
    std::auto_ptr<IStreamInput> font_program(int index, unsigned options) const;
    std::auto_ptr<IStreamInput>
    subset_font_program(UsedGlyphs const& glyphs,
                        unsigned options) const;
    Hash16 const& hash() const { return m_md5.sum(); }
    //@}

public:
    TypefaceImpl(boost::shared_ptr<FT_LibraryRec_> ftlib, std::auto_ptr<FTOpenArgs> args);
    ~TypefaceImpl();

    FT_Face face() { return m_face; }

private:
    void calculate_hash();
    int data_size(int index) const;
    void detect_type();
    void preflight();

private:
    // args for opening face, owns resources associated with typeface
    boost::scoped_ptr<FTOpenArgs>   m_open_args;
    FT_Face                         m_face;
    jstd::MD5Hash                m_md5;
    FaceType                        m_type;
    bool                            m_can_embed;
    bool                            m_can_subset;
    boost::shared_ptr<FT_LibraryRec_> m_ftlib;
    TypefaceMetrics                 m_metrics;
    Panose                          m_panose;
    Int                           m_weight_class;
    Double                          m_italic_angle;
    Int                           m_fixed_width;
    Int                           m_width_class;
};

// size_t hash_value(TypefaceImpl const& font);
// bool operator<(TypefaceImpl const& lhs, TypefaceImpl const& rhs);
// bool operator==(TypefaceImpl const& lhs, TypefaceImpl const& rhs);
// inline bool operator!=(TypefaceImpl const& lhs, TypefaceImpl const& rhs) { return !(lhs==rhs); }


}} // namespace jag::resources

#endif //__TYPEFACEIMPL_H_JAG_1318__

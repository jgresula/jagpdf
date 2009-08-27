// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef T1ADOBESTANDARDFACE_JG1620_H__
#define T1ADOBESTANDARDFACE_JG1620_H__

#include <resources/interfaces/typeface.h>
#include "t1adobestandardfonts.h"

namespace jag {
namespace resources {

class FontSpecImpl;

class T1AdobeStandardFace
    : public ITypeface
{
    t1s_face const*  m_face;

public:
    T1AdobeStandardFace(FontSpecImpl const& spec);

public: //ITypeface
    FaceType type() const {
        return FACE_TYPE_1_ADOBE_STANDARD;
    }

    TypefaceMetrics const& metrics() const {
        return m_face->FontMetrics;
    }

    Panose const& panose() const;

    Int weight_class() const {
        return m_face->Weight;
    }

    Int width_class() const {
        return 5;
    }

    Double italic_angle() const {
        return m_face->ItalicAngle;
    }

    Int fixed_width() const {
        return m_face->IsFixedPitch;
    }

    Int bold() const {
        return m_face->Weight >= 700;
    }

    Int italic() const {
        return m_face->ItalicAngle < 0.0;
    }

    Char const* family_name() const {
        return m_face->FamilyName;
    }

    Char const* style_name() const { return ""; }

    std::string full_name() const {
        return m_face->FullName;
    }

    Char const* postscript_name() const {
        return m_face->FontName;
    }

    Int char_horizontal_advance(Int codepoint) const;
    Int gid_horizontal_advance(UInt gid) const;
    Int kerning_for_gids(UInt left, UInt right) const;
    Int kerning_for_chars(Int left, Int right) const;

    Int can_embed() const {
        return false;
    }

    Int can_subset() const {
        return false;
    }

    UInt16 codepoint_to_gid(Int codepoint) const;
    FaceCharIterator char_iterator() const;

    Char const* encoding_scheme() const {
        return m_face->EncodingScheme;
    }

    Int num_streams() const {
        return 0;
    }

    std::auto_ptr<IStreamInput> font_program(int index, unsigned options) const;
    std::auto_ptr<IStreamInput> subset_font_program(UsedGlyphs const& glyphs,
                                                    unsigned options) const;

    Hash16 const& hash() const {
        return m_face->Hash;
    }
};


}} // namespace jag::resources

#endif // T1ADOBESTANDARDFACE_JG1620_H__
/** EOF @file */

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef FONTINFO_JG1337_H__
#define FONTINFO_JG1337_H__

#include <interfaces/refcounted.h>

namespace jag {

/// Represents a font.
///
/// Uniquely identifies a font and provides information about the font.
///
class IFont
    : public INotRefCounted
{
public:
    /// Returns non-zero value if the font is bold.
    virtual Int is_bold() const = 0;
    /// Returns non-zero value if the font is italic.
    virtual Int is_italic() const = 0;
    /// Retrieves font size in user space units.
    virtual Double size() const = 0;
    /// Retrieves font family.
    virtual Char const* family_name() const = 0;
    
    /// Retrieves width of the passed string in user space units.
    ///
    /// If kerning is turned on then the kerning offsets are counted in.
    virtual Double advance(Char const* txt_u) const = 0;

    /// Retrieves width of the passed string in user space units.
    ///
    /// If kerning is turned on then the kerning offsets are counted in.
    virtual Double advance_r(jag::Char const* begin, jag::Char const* end) const = 0;

    /// Retrieves width of the given glyph.
    ///
    /// @version 1.4
    virtual Double glyph_width(UInt16 glyph_index) const = 0;

    
    /// Retrieves baseline distance in user space units.
    virtual Double height() const = 0;
    /// Retrieves ascender in user space units.
    virtual Double ascender() const = 0;
    /// Retrieves descender in user space units.
    virtual Double descender() const = 0;
    /// Retrieves the horizontal minimum (left-most) in user space units.
    virtual Double bbox_xmin() const = 0;
    /// Retrieves the vertical minimum (bottom-most) in user space units.
    virtual Double bbox_ymin() const = 0;
    /// Retrieves the horizontal maximum (right-most) in user space units.
    virtual Double bbox_xmax() const = 0;
    /// Retrieves the vertical maximum (top-most) in user space units.
    virtual Double bbox_ymax() const = 0;

public:
    virtual Char const* style_name() const API_ATTR("internal") = 0;

protected:
    ~IFont() {}
};


} // namespace jag

#endif // FONTINFO_JG1337_H__
/** EOF @file */

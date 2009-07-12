// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FONT_H_JAG_2252__
#define __FONT_H_JAG_2252__

#include <interfaces/refcounted.h>
#include <resources/interfaces/typeface.h>
#include <resources/interfaces/fontinfo.h>
#include <resources/interfaces/charencoding.h>
#include <boost/intrusive_ptr.hpp>

namespace jag {

// fwd
namespace jstd { class UnicodeToCP; }

///
/// Provides extended font information.
///
class IFontEx
    : public IFont
{
public:
    virtual ITypeface const& typeface() const = 0;

    /// integral encoding id
    virtual EnumCharacterEncoding encoding_id() const = 0;
    virtual Char const* encoding_canonical() const = 0;

    virtual Double horizontal_advance_dbg(jag::Char const* text,
                                          jag::ULong length) const = 0;
    virtual Int is_in_font_dbg(jag::Char const* text,
                               jag::UInt length) const = 0;
    
    virtual Double kerning_for_gids(UInt left, UInt right) const = 0;
    virtual Double kerning_for_chars(Int left, Int right) const = 0;

    virtual bool has_multiple_encondings() const = 0;
    virtual IFontEx const& font_for_encoding(EnumCharacterEncoding enc) const = 0;
    virtual jstd::UnicodeToCP* unicode_to_8bit() const = 0;

    virtual ~IFontEx() {}
};


} // namespace jag

#endif //__FONT_H_JAG_2252__

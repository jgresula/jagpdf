// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/resourcebox/colorspacehelpers.h>
#include <resources/interfaces/colorspaceman.h>
#include <core/errlib/errlib.h>

namespace jag {
namespace resources {


ColorSpaceType color_space_type(ColorSpaceHandle handle)
{
    unsigned result = handle.id() & CS_COLOR_SPACE_TYPE_MASK;
    if (result & CS_PATTERN)
        return CS_PATTERN;

    return static_cast<ColorSpaceType>(result);
}


ColorSpaceHandle unmask_pattern(ColorSpaceHandle csh)
{
    JAG_PRECONDITION(is_pattern_color_space(csh) && !is_trivial_pattern_color_space(csh));
    return ColorSpaceHandle(csh).bitwise_and(static_cast<UInt>(~CS_PATTERN));
}


bool is_pattern_color_space(ColorSpaceHandle csh)
{
    return csh.id() & CS_PATTERN;
}



bool is_trivial_pattern_color_space(ColorSpaceHandle csh)
{
    return csh.id()==CS_PATTERN;
}

ColorSpaceType pattern_secondary_cs_type(ColorSpaceHandle handle)
{
    JAG_PRECONDITION(is_pattern_color_space(handle) && !is_trivial_pattern_color_space(handle));
    return static_cast<ColorSpaceType>(handle.id() & (CS_COLOR_SPACE_TYPE_MASK & ~CS_PATTERN));
}



bool is_trivial_color_space(ColorSpaceHandle handle)
{
    // there is a special case with CS_PATTERN - if only this bit is set then
    // it is a trivial color space, otherwise it is not.
    JAG_PRECONDITION(is_valid(handle));
    return is_trivial_pattern_color_space(handle) || (handle.id() <= CS_LAST_TRIVIAL_COLOR_SPACE);
}


int num_components(ColorSpaceHandle handle, IColorSpaceMan const* cs_man, bool allow_zero)
{
    switch (color_space_type(handle))
    {
    case CS_DEVICE_GRAY:
    case CS_CALGRAY:
        return 1;

    case CS_CIELAB:
    case CS_DEVICE_RGB:
    case CS_CALRGB:
        return 3;

    case CS_DEVICE_CMYK:
        return 4;

    case CS_INDEXED:
        /// suspicious
        JAG_ASSERT(!"No one should ask for this.");

    case CS_PATTERN:
        {
            // it might be complicated to find out it for uncolored pattern
            // - indexed cs calls this function but pattern cs
            //   is not allowed to be a base for it (spec. 4.5.5)
            JAG_ASSERT(!"No one should ask for this.");
            if (!is_trivial_pattern_color_space(handle))
                return num_components(handle, cs_man);
            return 1; // colored pattern
        }

    default:
        // we need to ask color space man
        if (cs_man)
            return cs_man->num_components(handle);
    }

    if (!allow_zero)
    {
        JAG_INTERNAL_ERROR;
    }

    return 0;
}



}} // namespace jag::resources

/** EOF @file */

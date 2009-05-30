// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __COLORSPACEHELPERS_JG2318_H__
#define __COLORSPACEHELPERS_JG2318_H__

#include <interfaces/constants.h>
#include <resources/interfaces/resourcehandle.h>


namespace jag {
//fwd
class IColorSpaceMan;

namespace resources {

/**
 * @brief Extracts color space type from color space handle.
 *
 * @param handle color space
 * @return color space type
 */
ColorSpaceType color_space_type(ColorSpaceHandle handle);


/**
 * @brief Indicates whether the given handle represents a trivial color space.
 *
 * 'Trivial' means that it does not need any explicit definition.
 *
 * @param handle color space
 * @return true if it is a simple color space, false otherwise
 */
bool is_trivial_color_space(ColorSpaceHandle handle);


/// Indicates whether the given color space is a pattern color space.
bool is_pattern_color_space(ColorSpaceHandle csh);



/// Indicates whether the given color space is a trivial pattern color space (i.e. without secondary color space).
bool is_trivial_pattern_color_space(ColorSpaceHandle csh);


/// Unmasks pattern bit from the colorspace.
ColorSpaceHandle unmask_pattern(ColorSpaceHandle csh);


/// Type of the pattern's secondary color space.
ColorSpaceType pattern_secondary_cs_type(ColorSpaceHandle handle);

/**
 * @brief Retrives number of required color space components to express a color.
 *
 * @param handle color space
 * @param cs_man color space manager (might be null)
 * @param allow_zero if true then the function is allowed to return zero
 *
 * @return number of components or zero (if allowed)
 */
int num_components(ColorSpaceHandle handle, IColorSpaceMan const* cs_man, bool allow_zero=false);


}} // namespace jag::resources

#endif // __COLORSPACEHELPERS_JG2318_H__
/** EOF @file */

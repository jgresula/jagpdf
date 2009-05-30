// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CONSTANTS_H_JG2246__
#define __CONSTANTS_H_JG2246__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/stdtypes.h>

namespace jag
{

// ---------------------------------------------------------------------------
//                    PRIVATE (not exported by SWIG)
//

#if !defined(SWIG)

enum StreamOffsetOrigin
{
    OFFSET_FROM_CURRENT,
    OFFSET_FROM_END,
    OFFSET_FROM_BEGINNING
};

#endif


// ---------------------------------------------------------------------------
//                    PUBLIC (exported by SWIG)
//

typedef UInt ColorSpace;
typedef UInt Pattern;
typedef UInt ImageMaskID;
typedef UInt Destination;
typedef UInt Function;


enum LineCapStyle
{
    LINE_CAP_BUTT,
    LINE_CAP_ROUND,
    LINE_CAP_SQUARE
};

enum LineJoinStyle
{
    LINE_JOIN_MITER,
    LINE_JOIN_ROUND,
    LINE_JOIN_BEVEL
};


enum RenderingIntentType
{
    RI_ABSOLUTE_COLORIMETRIC = 0,
    RI_RELATIVE_COLORIMETRIC = 1,
    RI_SATURATION            = 2,
    RI_PERCEPTUAL            = 3,
    RI_UNDEFINED             = -1
} API_ATTR("undocumented,"
           "internal_enum_val RI_UNDEFINED");

enum ImageFormat
{
    IMAGE_FORMAT_AUTO,
    IMAGE_FORMAT_NATIVE,
    IMAGE_FORMAT_PNG,
    IMAGE_FORMAT_JPEG,
    IMAGE_FORMAT_UNRECOGNIZED = 0xffff
} API_ATTR("internal_enum_val IMAGE_FORMAT_UNRECOGNIZED");;

enum ColorSpaceType
{
    CS_UNDEFINED    = 0,  // this must remain 0
    CS_DEVICE_RGB    = 1 << 0,
    CS_DEVICE_CMYK    = 1 << 1,
    CS_DEVICE_GRAY    = 1 << 2,
    CS_PATTERN        = 1 << 3,

    // ^ trivial color spaces

    CS_LAST_TRIVIAL_COLOR_SPACE = CS_PATTERN,

    CS_CIELAB   = 1 << 4,
    CS_CALGRAY  = 1 << 5,
    CS_CALRGB   = 1 << 6,
    CS_INDEXED    = 1 << 7,
    CS_ICCBASED = 1 << 8,

    // - the last assigned one, !this number must be power of 2
    CS_COLOR_SPACE_ID_START_BIT = 9,
    CS_COLOR_SPACE_TYPE_MASK    = (1<<CS_COLOR_SPACE_ID_START_BIT)-1,
} API_ATTR("internal_enum_val "
           "CS_UNDEFINED "
           "CS_COLOR_SPACE_ID_START_BIT "
           "CS_PATTERN "
           "CS_COLOR_SPACE_TYPE_MASK "
           "CS_LAST_TRIVIAL_COLOR_SPACE");


} //namespace jag


#endif //__CONSTANTS_H_JG2246__


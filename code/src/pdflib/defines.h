// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __DEFINES_H_JG2212__
#define __DEFINES_H_JG2212__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

namespace jag {
namespace pdf {


/// filter types
enum StreamFilter
{     // DO NOT CHANGE ORDER
      STREAM_FILTER_FLATE
    , STREAM_FILTER_DCTDECODE
};


//
// categories of graphics operators
//
enum GraphicsOperatorCategory {
    OPCAT_GENERAL_GRAPHICS_STATE = 1U << 0,  // W, J, j, M, d, ri, i, gs
    OPCAT_SPECIAL_GRAPHICS_STATE = 1U << 1,  // q, Q, cm
    OPCAT_PATH_CONSTRUCTION =      1U << 2,  // m, l, c, v, y, h, re
    OPCAT_PATH_PAINTING =          1U << 3,  // S, s, f, F, f*, B, B*, b, b*, n
    OPCAT_CLIPPING_PATHS =         1U << 4,  // W, W*
    OPCAT_TEXT_OBJECTS =           1U << 5,  // BT, ET
    OPCAT_TEXT_STATE =             1U << 6,  // Tc, Tw, Tz, TL, Tf, Tr, Ts
    OPCAT_TEXT_POSITIONING =       1U << 7,  // Td, TD, Tm, T*
    OPCAT_TEXT_SHOWING =           1U << 8,  // Tj, TJ, ', "
    OPCAT_TYPE_3_FONTS =           1U << 9,  // d0, d1
    OPCAT_COLOR =                  1U << 10, // CS, cs, SC, SCN, sc, scn, G, g, RG, rg, K, k
    OPCAT_SHADING_PATTERNS =       1U << 11, // sh
    OPCAT_INLINE_IMAGES =          1U << 12, // BI, ID, EI
    OPCAT_XOBJECTS =               1U << 13, // Do
    OPCAT_MARKED_CONTENT =         1U << 14, // MP , DP , BMC, BDC, EMC
    OPCAT_COMPATIBILITY =          1U << 15, // BX, EX
    // artifical
    OPCAT_XOBJECT_IMAGE =          1U << 16,
    OPCAT_XOBJECT_MASK =           1U << 17
};


char const* rendering_intent_string(int index);


}} //namespace jag::pdf


#endif //__DEFINES_H_JG2212__


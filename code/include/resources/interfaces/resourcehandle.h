// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCEHANDLE_H_JAG_1855__
#define __RESOURCEHANDLE_H_JAG_1855__


#include <interfaces/handle.h>


namespace jag {


struct RESOURCE_COLOR_SPACE {};
struct RESOURCE_PATTERN {};
struct RESOURCE_IMAGE {};
struct RESOURCE_GRAPHICS_STATE {};
struct RESOURCE_FONT {};
struct RESOURCE_TYPEFACE {};
struct RESOURCE_PALETTE {};
struct RESOURCE_IMAGE_MASK {};
struct RESOURCE_IMAGE_SOFT_MASK {};
struct RESOURCE_FUNCTION {};
struct RESOURCE_SHADING {};


typedef THandle<RESOURCE_FUNCTION> FunctionHandle;
typedef THandle<RESOURCE_PATTERN> PatternHandle;
typedef THandle<RESOURCE_SHADING> ShadingHandle;
typedef THandle<RESOURCE_COLOR_SPACE> ColorSpaceHandle;
typedef THandle<RESOURCE_IMAGE> ImageHandle;

typedef THandle<RESOURCE_IMAGE_MASK> ImageMaskHandle;
typedef THandle<RESOURCE_IMAGE_SOFT_MASK> ImageSoftMaskHandle;
typedef THandle<RESOURCE_GRAPHICS_STATE> GraphicsStateHandle;
typedef THandle<RESOURCE_TYPEFACE> TypefaceHandle;

} //namespace jagw

#endif //__RESOURCEHANDLE_H_JAG_1855__

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCEPODARRAYS_H_JAG_1353__
#define __RESOURCEPODARRAYS_H_JAG_1353__

#include <core/generic/staticpodarray.h>

namespace jag
{

typedef StaticPODArray<UInt,2*4> ColorKeyMaskArray;
typedef StaticPODArray<Double,2*4> DecodeArray;
typedef StaticPODArray<Double,4> ColorComponents;

} // namespace jag::resources

#endif //__RESOURCEPODARRAYS_H_JAG_1353__


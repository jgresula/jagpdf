// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __XOBJECTUTILS_H_JAG_2243__
#define __XOBJECTUTILS_H_JAG_2243__

#include <resources/interfaces/resourceconstants.h>
#include <resources/resourcebox/resourcepodarrays.h>

namespace jag {
// fwd
class IProfileInternal;
class IImageMaskData;

namespace pdf {
// fwd
class ObjFmtBasic;
class DocWriterImpl;

void xobject_output(ObjFmtBasic& fmt, InterpolateType val, IProfileInternal const& cfg);
void xobject_output(ObjFmtBasic& fmt, DecodeArray const& val);

enum { DMT_SOFT, DMT_HARD, DMT_NOT_SUPPORTED };
unsigned detect_mask_type(IImageMaskData const& mask_data, DocWriterImpl const& doc);

}} //namespace jag::pdf

#endif //__XOBJECTUTILS_H_JAG_2243__


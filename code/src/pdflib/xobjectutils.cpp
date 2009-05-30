// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "xobjectutils.h"
#include "objfmtbasic.h"
#include "docwriterimpl.h"
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>
#include <resources/interfaces/imagemaskdata.h>


namespace jag {
namespace pdf {

//////////////////////////////////////////////////////////////////////////
void xobject_output(ObjFmtBasic& fmt, InterpolateType interpolate, IProfileInternal const& cfg)
{
    if (INTERPOLATE_UNDEFINED == interpolate)
    {
        if (cfg.get_int("images.interpolated"))
        {
            fmt.dict_key("Interpolate").space().output_bool(true);
        }
    }
    else
    {
        // value set explicitly by a client
        fmt.dict_key("Interpolate").space().output_bool(interpolate ? true : false);
    }
}

//////////////////////////////////////////////////////////////////////////
void xobject_output(ObjFmtBasic& fmt, DecodeArray const& decode)
{
    if (is_valid(decode))
    {
        fmt.dict_key("Decode");
        fmt.array_start();
        for(size_t i=0; i<decode.size(); ++i)
        {
            fmt.output(decode.data()[i]).space();
        }
        fmt.array_end();
    }
}


//
//
//
unsigned detect_mask_type(IImageMaskData const& mask_data, DocWriterImpl const& doc)
{
    if ((mask_data.bits_per_component() == 1) &&
        (doc.version() == 3))
    {
        return DMT_HARD;
    }

    if (doc.version() >= 4)
    {
        return DMT_SOFT;
    }

    return DMT_NOT_SUPPORTED;
}





}} //namespace jag::pdf

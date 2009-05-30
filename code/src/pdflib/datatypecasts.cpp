// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "datatypecasts.h"
#include <core/errlib/errlib.h>
#include <resources/interfaces/typeface.h>


namespace jag {
namespace pdf {


char const* font_type_str(ITypeface const& face)
{
    switch(face.type())
    {
    case FACE_TRUE_TYPE:
    case FACE_OPEN_TYPE_CFF:
        return "TrueType";
    case FACE_TYPE_1_ADOBE_STANDARD:
        return "Type1";
    default:
        JAG_INTERNAL_ERROR;
    }
}


char const* encoding_type_str(EnumCharacterEncoding enc)
{
    if (enc == ENC_CP_1252)
        return "WinAnsiEncoding";

    JAG_INTERNAL_ERROR;
}


/**
 * @todo check map names as there are multiple encoding even for one cp/lfCharset
 */
char const* cmap_str(EnumCharacterEncoding enc)
{
    switch(enc)
    {
    case ENC_SHIFT_JIS: return "90ms-RKSJ-H";  //cp 932
    case ENC_GB2312:    return "GB-EUC-H";     //cp 936
    case ENC_HANGEUL:   return "KSCms-UHC-H";  //cp 949
    case ENC_BIG5:      return "ETen-B5-H";    //cp 950
    default:
        JAG_INTERNAL_ERROR;
    }
}


std::pair<char const*, int> character_collection_str(EnumCharacterEncoding enc, Int /*version*/)
{
    switch(enc)
    {
    case ENC_IDENTITY:
        return std::make_pair("Identity", 0);

    case ENC_SHIFT_JIS: // "90ms-RKSJ-H";  //cp 932
        return std::make_pair("Japan1", 1);

    case ENC_GB2312:    // "GB-EUC-H";     //cp 936
        return std::make_pair("GB1", 0);

    case ENC_HANGEUL:   // "KSCms-UHC-H";  //cp 949
        return std::make_pair("Korea1", 1);

    case ENC_BIG5:      // "ETen-B5-H";    //cp 950
        return std::make_pair("CNS1", 0);

    default:
        JAG_INTERNAL_ERROR;
    }
}


}} // namespace jag::pdf

/** EOF @file */

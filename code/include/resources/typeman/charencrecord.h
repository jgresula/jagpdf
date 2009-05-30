// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CHARENCODINGRECORD_H_JAG_1212__
#define __CHARENCODINGRECORD_H_JAG_1212__

#include <resources/interfaces/charencoding.h>

namespace jag { namespace resources
{

//////////////////////////////////////////////////////////////////////////
struct CharEncodingRecord
{
    CharEncodingRecord(EnumCharacterEncoding enc, char const* encoding, int lfcharset=-1)
        : encoding(enc)
        , encoding_canonical(encoding)
        , win_lfcharset(lfcharset)
    {}

    const EnumCharacterEncoding encoding;
    char const*const            encoding_canonical;
    /// -1 means that cp is not supported by LOGFONT
    const int                   win_lfcharset;
};


}} //namespace jag::resources

#endif //__CHARENCODINGRECORD_H_JAG_1212__

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/conversions.h>
#include <core/jstd/icumain.h>
#include <unicode/ucnv.h>
#include <core/generic/scopeguard.h>
#include <string.h>

namespace jag {
namespace jstd {

FromUTF8::FromUTF8(Char const* src, UInt length)
{
    UErrorCode err = U_ZERO_ERROR;
    UConverter *conv = ucnv_open("utf8", &err);
    CHECK_ICU(err);
    ON_BLOCK_EXIT(ucnv_close, conv);

    if (!length)
        length = static_cast<UInt>(strlen(src));

    int32_t utf16_len = ucnv_toUChars(conv, 0, 0, src, length, &err);
    if(err==U_BUFFER_OVERFLOW_ERROR)
    {
        err = U_ZERO_ERROR;
        m_utf16.reset(new UChar[utf16_len+1]);
        ucnv_toUChars(conv, m_utf16.get(), utf16_len, src, length, &err);
        CHECK_ICU(err);
        m_utf16[utf16_len]=0;
    }
}

}} // namespace jag::jstd

/** EOF @file */

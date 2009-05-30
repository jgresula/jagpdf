// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/win32/win32common.h>

namespace jag { namespace jstd
{

/**
* @todo use tls for local buffer
*/
wchar_t const* format_win32_message(DWORD err)
{
    static wchar_t buffer[255];
    ::FormatMessageW(
        FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err, 0, buffer, 255, 0);

    return buffer;
}

}} // namespace jag::jstd

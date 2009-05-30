// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/message_sink_console.h>
#include <cstdio>

namespace jag { namespace jstd
{

//////////////////////////////////////////////////////////////////////////
void MessageSinkConsole::message(
    jag::UInt /*code*/
    , MessageSeverity sev
    , jag::Char const* /*msg*/)
{
    switch(sev)
    {
    case MSG_INFO:
        //fprintf(stdout, "I%d: %s\n", code, msg);
        break;

    case MSG_WARNING:
        //fprintf(stdout, "W%d: %s\n", code, msg);
        break;

    case MSG_ERROR:
        //fprintf(stderr, "E%d: %s\n", code, msg);
        break;
    }
}

}} //namespace jag::jstd


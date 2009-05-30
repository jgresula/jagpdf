// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __MESSAGE_SINK_CONSOLE_H_JG2133__
#define __MESSAGE_SINK_CONSOLE_H_JG2133__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/message_sink.h>

namespace jag { namespace jstd
{

class MessageSinkConsole
    : public IMessageSink
{
public:
    void message(jag::UInt code, MessageSeverity sev, jag::Char const* msg);
};

}} //namespace jag::jstd


#endif //__MESSAGE_SINK_CONSOLE_H_JG2133__


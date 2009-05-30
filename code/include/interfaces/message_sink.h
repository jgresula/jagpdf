// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __MESSAGE_SINK_H_JG2117__
#define __MESSAGE_SINK_H_JG2117__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/stdtypes.h>
#include <core/generic/noncopyable.h>
#include <interfaces/message_severity.h>

namespace jag
{

class IMessageSink
    : public noncopyable
{
public:
    /**
     * @brief message callback
     *
     * @param code message code
     * @param sev message severity
     * @param msg message string (UTF-8)
     */
    virtual void message(jag::UInt code, MessageSeverity sev, jag::Char const* msg) = 0;
};

} //namespace jag


#endif //__MESSAGE_SINK_H_JG2117__




// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __MSGRECORD_H_JG024__
#define __MSGRECORD_H_JG024__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/stdtypes.h>
#include <interfaces/message_severity.h>


namespace jag
{

struct MessageRecord
{
    char const*     m_message;
    UInt            m_msg_code;
    MessageSeverity    m_severity;
};

} //namespace jag


#endif //__MSGRECORD_H_JG024__


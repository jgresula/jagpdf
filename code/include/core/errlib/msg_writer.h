// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __MSG_WRITTER_H_JG1429__
#define __MSG_WRITTER_H_JG1429__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/message_sink.h>
#include <core/errlib/msgcodes.h>
#include <core/errlib/log.h>
#include <core/generic/assert.h>
#include <boost/format.hpp>
#include <string>


namespace jag
{

//////////////////////////////////////////////////////////////////////////
//template<class Device>
//PrintfState<Device, jag::WChar> format_message(unsigned int& msgcode, Device& device)
//{
//    msgcode = (msgcode < 0 || msgcode >= TOTAL_NUMBER_OF_MESSAGES) ? 0 : msgcode;
//    XPrintf(device, msg_records[msgcode]->m_message);
//}

//////////////////////////////////////////////////////////////////////////
inline void write_message(unsigned int msgcode)
{
    msgcode = msgcode >= TOTAL_NUMBER_OF_MESSAGES ? 0 : msgcode;
    std::string buffer(msg_records[msgcode].m_message);
    message_sink().message(
          msg_records[msgcode].m_msg_code
        , msg_records[msgcode].m_severity
        , buffer.c_str()
   );
}


//////////////////////////////////////////////////////////////////////////
template<class T1>
void write_message(unsigned int msgcode, T1 const& t1)
{
    msgcode = msgcode >= TOTAL_NUMBER_OF_MESSAGES ? 0 : msgcode;
    message_sink().message(
        msg_records[msgcode].m_msg_code
        , msg_records[msgcode].m_severity
        , boost::str(boost::format(msg_records[msgcode].m_message) % t1).c_str()
   );

}

//////////////////////////////////////////////////////////////////////////
template<class T1, class T2>
void write_message(unsigned int msgcode, T1 const& t1, T2 const& t2)
{
    msgcode = msgcode >= TOTAL_NUMBER_OF_MESSAGES ? 0 : msgcode;
    message_sink().message(
        msg_records[msgcode].m_msg_code
        , msg_records[msgcode].m_severity
        , boost::str(boost::format(msg_records[msgcode].m_message) % t1 % t2).c_str()
   );
}



} //namespace jag


#endif //__MSG_WRITTER_H_JG1429__

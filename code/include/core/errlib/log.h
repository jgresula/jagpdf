// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __LOG_H_JG_2008__
#define __LOG_H_JG_2008__

namespace jag
{
class IMessageSink;

IMessageSink& message_sink();
void set_message_sink(IMessageSink*);

} //namespace jag


#endif //__LOG_H_JG_2008__

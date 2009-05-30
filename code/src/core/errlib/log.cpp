// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/errlib/log.h>
#include <interfaces/message_sink.h>
#include <cstdio>


namespace {

//////////////////////////////////////////////////////////////////////////
class StdErrSink
    : public jag::IMessageSink
{
    StdErrSink() {}
public: //IMessageSink
    void message(jag::UInt /*code*/, jag::MessageSeverity /*sev*/, jag::Char const* /*msg*/)    {
//        fprintf(stderr, "%s\n", msg);
    }

public:
    static IMessageSink& instance();
};

//////////////////////////////////////////////////////////////////////////
jag::IMessageSink& StdErrSink::instance()
{
    static StdErrSink sink;
    return sink;
}


jag::IMessageSink*    g_message_sink = 0;
} // anonymous namespace

namespace jag
{

//////////////////////////////////////////////////////////////////////////
IMessageSink& message_sink()
{
    return g_message_sink
        ? *g_message_sink
        : StdErrSink::instance()
    ;

}

//////////////////////////////////////////////////////////////////////////
void set_message_sink(IMessageSink* sink)
{
    g_message_sink = sink;
}

} //namespace jag

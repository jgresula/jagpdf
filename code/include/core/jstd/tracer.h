// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef TRACER_JG911_H__
#define TRACER_JG911_H__

namespace jag {
namespace jstd {


// Defining JAG_TRACING_ACTIVE turns tracing on regardless of
// debug/release build. If not defined tracing is active only in debug
// build
#if !defined(JAG_TRACING_ACTIVE) && defined(JAG_DEBUG)
#  define JAG_TRACING_ACTIVE
#endif

#ifdef JAG_TRACING_ACTIVE
const bool g_tracing_active = true;
#else
const bool g_tracing_active = false;
#endif


enum TracingLevels { TL_NONE, TL_ERR, TL_WRN, TL_INFO, TL_DETAIL, TL_PARANOID, TracingLevels_LAST };
void show_trace_location(bool val);
void set_tracing_level(TracingLevels);

class Tracer
{
    int               m_level;
    char const*const  m_file;
    const int         m_line;
    bool              m_already_out;

public:
    Tracer(char const* file, int line /*func*/, TracingLevels level);
    Tracer& operator<<(char const* str);
    Tracer& operator<<(void const* ptr);
    Tracer& operator<<(char c);
    Tracer& operator<<(int val);
    ~Tracer();

    Tracer& if_(bool val);

private:
    int output_prologue();

private:
    template<class T>
    Tracer& output(T val, char const* prefix=0);
};

#define TRACE_ERR       if(!jag::jstd::g_tracing_active) ; else jag::jstd::Tracer(__FILE__, __LINE__, jag::jstd::TL_ERR)
#define TRACE_WRN       if(!jag::jstd::g_tracing_active) ; else jag::jstd::Tracer(__FILE__, __LINE__, jag::jstd::TL_WRN)

#define TRACE_INFO      if(!jag::jstd::g_tracing_active) ; else jag::jstd::Tracer(__FILE__, __LINE__, jag::jstd::TL_INFO)
#define TRACE_DETAIL    if(!jag::jstd::g_tracing_active) ; else jag::jstd::Tracer(__FILE__, __LINE__, jag::jstd::TL_DETAIL)
#define TRACE_PARANOID  if(!jag::jstd::g_tracing_active) ; else jag::jstd::Tracer(__FILE__, __LINE__, jag::jstd::TL_PARANOID)


}} // namespace jag::jstd

#endif // TRACER_JG911_H__
/** EOF @file */

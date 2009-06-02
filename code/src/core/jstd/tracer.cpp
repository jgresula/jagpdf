// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/tracer.h>
#include <core/generic/assert.h>
#include <iostream>
#include <ctime>
#include <string.h>

namespace jag {
namespace jstd {

bool           g_tracing_show_loc = true;
TracingLevels  g_tracing_level = TL_DETAIL;
char const     *g_tracing_level_id = "-EWidp";

#ifdef JAG_WIN32
 const char g_path_sep = '\\';
#else
 const char g_path_sep = '/';
#endif

void show_trace_location(bool val)
{
    g_tracing_show_loc = val;
}

void set_tracing_level(TracingLevels level)
{
    g_tracing_level = level;
}


///////////////////////////////////////////////////////////////////////////
// class Tracer
Tracer::Tracer(char const* file, int line /*func*/, TracingLevels level)
    : m_level(level <= g_tracing_level ? level : 0)
    , m_file(file)
    , m_line(line)
    , m_already_out(false)
{
}

Tracer::~Tracer()
{
    if (m_level)
    {
        char const* file = strrchr(m_file, g_path_sep);
        file = file ? file+1 : m_file;

        if (g_tracing_show_loc)
            std::cout << " (" << file << ':' << m_line << ')' ;

        std::cout << std::endl;
    }
}


Tracer& Tracer::if_(bool val)
{
    JAG_ASSERT_MSG(val || !m_already_out, "dismissing this trace, but something is already out");
    if (!val)
        m_level = 0;

    return *this;
}


int Tracer::output_prologue()
{
    if (m_level && !m_already_out)
    {
        const int BUFFER_SIZE = 10;
        char date[BUFFER_SIZE];

        time_t ltime;
        time(&ltime);
        struct tm *today = localtime(&ltime);
        strftime(date, BUFFER_SIZE, "%H:%M:%S", today);
        std::cout << date << ' ' << g_tracing_level_id[m_level] << "  ";
        m_already_out = true;
    }

    return m_level;
}


template<class T>
Tracer& Tracer::output(T val, char const* prefix)
{
    if (output_prologue())
    {
        if (prefix)
            std::cout << prefix;

        std::cout << val;
    }

    return *this;
}

Tracer& Tracer::operator<<(char const* str) { return output(str); }
Tracer& Tracer::operator<<(void const* ptr) { return output(ptr, "0x"); }
Tracer& Tracer::operator<<(char c) { return output(c); }
Tracer& Tracer::operator<<(int val) { return output(val); }



}} // namespace jag::jstd

/** EOF @file */

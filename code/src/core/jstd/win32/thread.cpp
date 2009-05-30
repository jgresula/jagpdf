// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <windows.h>
#include <core/generic/assert.h>
#include <core/jstd/thread.h>

namespace jag {
namespace jstd {

//
//
//
static
DWORD WINAPI thread_func(PVOID pvParam)
{
    Thread::fn_t* fn = static_cast<Thread::fn_t*>(pvParam);
    JAG_ASSERT(fn);

    try
    {
        (*fn)();
    }
    catch(...)
    {
        JAG_ASSERT(!"unhandled exception in thread");
        abort();
    }

    return 0;
}


//
//
//
Thread::Thread(fn_t fn)
    : m_thread_fn(fn)
{
    DWORD thread_id;
    m_thread = ::CreateThread(
        NULL,              // default security attributes
        0,                 // use default stack size
        thread_func,       // thread function
        &m_thread_fn,      // argument to thread function
        0,                 // use default creation flags
        &thread_id);       // returns the thread identifier

    if (!m_thread)
        throw std::runtime_error("thread creation failed");
}

//
//
//
Thread::~Thread()
{
    JAG_ASSERT(m_thread);
    ::CloseHandle(m_thread);
}


//
//
//
void Thread::join()
{
    JAG_ASSERT(m_thread);
    ::WaitForSingleObject(m_thread, INFINITE);
}


//
// Free functions.
//


void scheduled_yield()
{
    ::Sleep(0);
}


}} // namespace jag::jstd

/** EOF @file */

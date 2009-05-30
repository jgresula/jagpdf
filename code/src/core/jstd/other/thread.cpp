// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/generic/assert.h>
#include <pthread.h>
#include <core/jstd/thread.h>


namespace jag {
namespace jstd {

//
//
//
static
void* thread_func(void* arg)
{
    Thread::fn_t* fn = static_cast<Thread::fn_t*>(arg);
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
    , m_thread(0)
{
    if (pthread_create(&m_thread, 0, thread_func, &m_thread_fn))
        throw std::runtime_error("thread creation failed");

    JAG_ASSERT(m_thread);
}

//
//
//
Thread::~Thread()
{
    JAG_ASSERT(m_thread);
//    pthread_detach(m_thread);
}


//
//
//
void Thread::join()
{
    JAG_ASSERT(m_thread);
    void* rval_ptr;
    pthread_join(m_thread, &rval_ptr);
}


//
// Free functions.
//

void scheduled_yield()
{
    sched_yield();
}


}} // namespace jag::jstd

/** EOF @file */

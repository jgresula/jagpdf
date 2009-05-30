// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef THREAD_JG234_H__
#define THREAD_JG234_H__

#include <boost/config.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>


#ifdef BOOST_HAS_WINTHREADS
# include <windows.h>
# include "win32/thread.h"
#elif defined(BOOST_HAS_PTHREADS)
# include <pthread.h>
# include "other/thread.h"
#else
# error "unsupported platform"
#endif

namespace jag {
namespace jstd {

//
// Represents a thread.
//
class Thread
    : public boost::noncopyable
{
public:
    typedef boost::function0<void> fn_t;
    explicit Thread(fn_t fn);
    ~Thread();
    void join();

private:
    fn_t m_thread_fn;

#   ifdef BOOST_HAS_WINTHREADS
    HANDLE        m_thread;
#   else
    pthread_t     m_thread;
#   endif
};


/// Yields the processor from the currently executing thread to
/// another ready to run, active thread of equal priority.
void scheduled_yield();

}} // namespace jag::jstd



#endif // THREAD_JG234_H__
/** EOF @file */

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testtools.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <core/jstd/dynlib.h>
#include <core/jstd/thread.h>
#include <stdio.h>
#include <assert.h>

using namespace jag::jstd;

//
// dynamical load stuff
typedef void (*library_fn_t)(void);
static library_fn_t active_lib = 0;


// thread function
//
void thread_fn()
{
    (*active_lib)();
    (*active_lib)();
    (*active_lib)();
}

//
// main entrypoint
//
void test_main()
{
# ifdef JAG_WIN32
    char const*const dllmytss_name = "dllmytss";
# else
    char const*const dllmytss_name = "libdllmytss";
# endif

    try
    {
        DynamicLibrary dll(dllmytss_name);
//         DynamicLibrary* pdll = new DynamicLibrary("dllmytss");
//         DynamicLibrary& dll = *pdll;

        active_lib = get_dll_symbol<library_fn_t>(dll, "dll_entrypoint");

        (*active_lib)();

        const int NUM_THREADS = 1;
        boost::ptr_vector<Thread> threads;
        threads.reserve(NUM_THREADS);

        for(int i=0; i<NUM_THREADS; ++i)
            threads.push_back(new Thread(thread_fn));

        for(int i=0; i<NUM_THREADS; ++i)
            threads[i].join();
    }
    catch(std::exception& exc)
    {
        fprintf(stderr, "ERROR: %s\n", exc.what());
        BOOST_ERROR("failed");
    }

    puts("[app] done");
}

//
//
//
int main()
{
    int result = guarded_test_run(test_main);
    result += boost::report_errors();
    return result;
}
















// Dynamic Initialization and Destruction with Concurrency
// -------------------------------------------------------
// see: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2444.html


// synchronized init (run_once, thread-safe singleton)
// - run_once is preferable, boost seems to have fast implementations
// - create a tss slot; the slot will hold a keyed list (map) of values
//
// --
//
// individual values must be keyed (by what - unique id, address of a shared obj)
// operations
// - get .. key
// - reset .. key, value_ptr, cleanup
// - release .. key (detaches value_ptr from tss)
//
//
// per-thread cleanup
// - must be called explicitly
// - or provided by platform (distinguish mingw and cygwin)
//
// --
//
// synchronized tss slot destruction
//
//
// ----
// - single cleanup function per type
// ?????
// - slot creation/deletion - can be done in ctor/dtor of global variable
//   - are all threads done by then (i.e. cleanup was called)
//   - cleanup for the main thread must be invoked explicitly
// - look into AllocHeap (used by boost for allocations)
// - dynamic unload
//   - not all threads are necessarily done
//

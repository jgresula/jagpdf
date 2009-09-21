// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


// Notes:
// - process attach goes after global variable constructors
// - process detach goes befor global variable destructors

# include <stdio.h>
# include <core/generic/assert.h>
# include <core/jstd/tss.h>
# include <core/jstd/atomicop.h>
# include <core/jstd/thread.h>
# include <boost/scoped_ptr.hpp>


static long tss_instance_count = 0;


#ifndef _WIN32
#  define DLLMYTSS __attribute__((visibility("default")))
#  if defined(__GNUC__)

// GCC speficic initialization
static void gcc_dll_attach() __attribute__ ((constructor));
static void gcc_dll_detach() __attribute__ ((destructor));

static
void gcc_dll_attach()
{
    if (!jag::jstd::tls_on_process_attach())
        abort();
}

static
void gcc_dll_detach()
{
    jag::jstd::tls_on_process_detach();
    printf("[dllmytss]: dll_detach (tss instances: %ld)\n", tss_instance_count);
    JAG_ASSERT(tss_instance_count==0);
}

namespace jag { namespace jstd {
    // this confirms that tls hooks are called
    void tls_cleanup_implemented() {}
}}


#  endif
#else
#  define DLLMYTSS __declspec(dllexport)

// WIN32 speficic initialization
int __stdcall DllMain(void* /*hinst*/, unsigned reason,  void* /*reserved*/)
{
    JAG_ASSERT(reason < 4);

    bool result = true;

    switch(reason)
    {
    case 1: // process attach
        result = jag::jstd::tls_on_process_attach();
        break;

    case 0: // process detach
        result = jag::jstd::tls_on_process_detach();
        printf("[dllmytss]: dll_detach (tss instances: %ld)\n", tss_instance_count);
        JAG_ASSERT(tss_instance_count==0);
        break;

    case 2: // thread attach
        result = jag::jstd::tls_on_thread_attach();
        break;

    case 3: // thread detach
        result = jag::jstd::tls_on_thread_detach();
        break;
    }

    return result ? 1 : 0;
}

namespace jag { namespace jstd {
    // this confirms that tls hooks are called
    void tls_cleanup_implemented() {}
}}


# endif





//
// thread specific data structure
//
class TSSData
{
    unsigned m_val;

public:
    TSSData()
        : m_val(0)
    {
        jag::jstd::atomic_increment(&tss_instance_count);
        printf("TSSData(0x%p) constructed on thread 0x%x\n",
               this,
               static_cast<unsigned int>(jag::jstd::current_thread_id()));
    }

    ~TSSData() {
        jag::jstd::atomic_decrement(&tss_instance_count);
        printf("TSSData(0x%p) destroyed on thread 0x%x with value set to 0x%x\n",
               this,
               static_cast<unsigned int>(jag::jstd::current_thread_id()),
               m_val);
    }


    void set(unsigned val) {
        m_val = val;
    }

    unsigned get() const {
        return m_val;
    }
};


struct TSSData2
{
    TSSData2() {
        jag::jstd::atomic_increment(&tss_instance_count);
    }

    ~TSSData2() {
        jag::jstd::atomic_decrement(&tss_instance_count);
    }

    int get() const { return 15; }
};


//
//
//
struct OnLoadAndUnload
{
    OnLoadAndUnload() {
        puts("[dllmytss]: global variable constructor");
        fflush(stdout);
    }

    ~OnLoadAndUnload() {
        puts("[dllmytss]: global variable destructor");
        fflush(stdout);
    }
};

OnLoadAndUnload on_load_and_unload;



//
//
//
extern "C"
{
    DLLMYTSS
    void dll_entrypoint();
}


//
// global tls keys
//
const unsigned id_tls_my_data  = 0x123084;
const unsigned id_tls_my_data2 = 0x123085;


//
// exported function
//
void dll_entrypoint()
{

    TSSData* data = static_cast<TSSData*>(jag::jstd::get_tls_data(id_tls_my_data));
    jag::ULong tid = jag::jstd::current_thread_id();
    if (data)
        JAG_ASSERT(data->get() == tid);

    std::auto_ptr<TSSData> tss(new TSSData);
    tss->set(tid);
    jag::jstd::set_tls_data(id_tls_my_data, tss.release(), jag::jstd::make_tls_cleanup<TSSData>());

//    TSSData2
    TSSData2* data2 = static_cast<TSSData2*>(jag::jstd::get_tls_data(id_tls_my_data2));
    if (data2)
        JAG_ASSERT(data2->get() == 15);

    std::auto_ptr<TSSData2> tss2(new TSSData2);
    jag::jstd::set_tls_data(id_tls_my_data2, tss2.release(), jag::jstd::make_tls_cleanup<TSSData2>());
}



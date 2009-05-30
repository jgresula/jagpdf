// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <pthread.h>
#include <stdexcept>
#include <stdio.h>
#include <core/jstd/tss.h>
#include <core/generic/assert.h>

// It seems (at least on Cygwin) that it is not possible to create a
// tss key in a constructor of a global variable (pthred_key_create
// fails). For that reason we have to acquire the tss key using
// pthread_once. That is suboptimal as we need to synchronize on every
// tss fetch.

namespace jag {
namespace jstd {

class TlsSlot
{
    pthread_key_t m_tls_index;
    bool          m_valid;
public:
    TlsSlot()
        : m_tls_index(0)
        , m_valid(true) // cannot throw exceptions so using errflag
    {
        int err = pthread_key_create(&m_tls_index, detail::tls_slot_cleanup);
        if (err)
        {
            m_valid = false;
            printf("TLS err %d\n", err);
            JAG_ASSERT(!"cannot create tls key");
        }
    }

    ~TlsSlot() {
        if (m_tls_index)
        {
            if (pthread_key_delete(m_tls_index))
            {
                JAG_ASSERT(!"cannot delete tls key");
            }
        }
    }

public:
    bool valid() const {
        return m_valid;
    }

    pthread_key_t key() const {
        JAG_ASSERT(m_valid);
        return m_tls_index;
    }
};


static TlsSlot* g_tls_slot_ptr = 0;
static pthread_once_t g_tls_init = PTHREAD_ONCE_INIT;

static
void acquire_tls_key()
{
    g_tls_slot_ptr = new TlsSlot;
}

//
//
//
namespace detail
{
  detail::TlsSlotData* get_tls_data_head()
  {
      pthread_once(&g_tls_init, acquire_tls_key);
      return static_cast<detail::TlsSlotData*>(pthread_getspecific(g_tls_slot_ptr->key()));
  }


  void set_tls_data_head(void* data)
  {
      tls_cleanup_implemented();
      pthread_once(&g_tls_init, acquire_tls_key);
      if (pthread_setspecific(g_tls_slot_ptr->key(), data))
      {
          // this should be very rare
          throw std::runtime_error("cannot set tls value");
      }
  }
}


//
// module is responsible to calling these functions
//
bool tls_on_process_attach()
{
//     if (!g_tls_slot_ptr->valid())
//         return false;

    return true;
}


bool tls_on_process_detach()
{
    detail::TlsSlotData* data = detail::get_tls_data_head();
    detail::tls_slot_cleanup(data);
    if (g_tls_slot_ptr)
        delete g_tls_slot_ptr;
    return true;
}



}} // namespace jag::jstd



/** EOF @file */

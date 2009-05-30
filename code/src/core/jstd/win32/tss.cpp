// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <windows.h>
#include <core/generic/assert.h>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

#include <core/jstd/tss.h>

namespace jag {
namespace jstd {

// as we are are always going to use tls we can afford to
// acquire/release tls slot in ctor/dtor of global variable
//
// dtors of global variables go after process detach so all tls data
// can be cleaned up by that time
class TlsSlot
{
    DWORD m_tls_index;
public:
    TlsSlot() {
        m_tls_index = ::TlsAlloc();
        // failure is reported by tls_on_process_attach
        JAG_ASSERT(m_tls_index != TLS_OUT_OF_INDEXES);
    }

    ~TlsSlot() {
        if (m_tls_index!=TLS_OUT_OF_INDEXES)
            ::TlsFree(m_tls_index);
    }

public:
    bool valid() const {
        return m_tls_index != TLS_OUT_OF_INDEXES;
    }

    DWORD key() const {
        JAG_ASSERT(m_tls_index != TLS_OUT_OF_INDEXES);
        return m_tls_index;
    }
};

static TlsSlot g_tls_slot;


//
//
//
namespace detail
{
  detail::TlsSlotData* get_tls_data_head()
  {
      return static_cast<detail::TlsSlotData*>(::TlsGetValue(g_tls_slot.key()));
  }


  void set_tls_data_head(void* data)
  {
      tls_cleanup_implemented();
      if (!::TlsSetValue(g_tls_slot.key(), data))
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
    if (!g_tls_slot.valid())
        return false;

    return true;
}

bool tls_on_process_detach()
{
    return tls_on_thread_detach();
}

bool tls_on_thread_attach()
{
    return true;
}

bool tls_on_thread_detach()
{
    detail::TlsSlotData* data = detail::get_tls_data_head();
    detail::tls_slot_cleanup(data);
    return true;
}

}} // namespace jag::jstd


/** EOF @file */

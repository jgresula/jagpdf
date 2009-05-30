// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/tss.h>
#include <core/generic/assert.h>

namespace jag {
namespace jstd {

namespace detail
{
  //
  //
  //
  HeadAndFound find_tls_entry(unsigned key)
  {
      TlsSlotData* head = get_tls_data_head();
      TlsSlotData* found = head;

      while (found)
      {
          if (found->m_key!=key)
              found = found->m_next;
          else
              break;
      }

      return HeadAndFound(head, found);
  }

  //
  //
  //
  void tls_slot_cleanup(void* ptr)
  {
      detail::TlsSlotData* data = static_cast<detail::TlsSlotData*>(ptr);
      while (data)
      {
          TlsSlotData* old = data;
          try {
              (*data->m_cleanup)(data->m_ptr);
          } catch(...) {
              JAG_ASSERT(!"tls custom cleanup thrown an exception");
          }
          data = data->m_next;
          delete old;
      }
  }

} //namespace detail

//
//
//
void* get_tls_data(unsigned key)
{
    detail::HeadAndFound result(detail::find_tls_entry(key));
    return result.second
        ? result.second->m_ptr
        : 0;
}


//
//
//
void set_tls_data(unsigned key, void* value, boost::shared_ptr<ITlsCleanup> cleanup)
{
    detail::HeadAndFound head_and_found = detail::find_tls_entry(key);
    detail::TlsSlotData* data = head_and_found.second;

    if (data)
    {
        (*data->m_cleanup)(data->m_ptr);
    }
    else
    {
        std::auto_ptr<detail::TlsSlotData> new_one(new detail::TlsSlotData);
        new_one->m_next = head_and_found.first;
        detail::set_tls_data_head(new_one.get());
        data = new_one.release();
    }

    data->m_key = key;
    data->m_ptr = value;
    data->m_cleanup = cleanup;
}


}} // namespace jag::jstd


/** EOF @file */

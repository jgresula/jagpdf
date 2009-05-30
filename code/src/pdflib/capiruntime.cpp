// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "capiruntime.h"
#include <core/generic/assert.h>
#include <core/jstd/tss.h>
#include <sstream>

JAG_EXPORT void JAG_CALLSPEC jag_release(void* obj)
{
    if (obj)
        static_cast<jag::IRefCounted*>(static_cast<void*>(obj))->Release();
}


JAG_EXPORT void JAG_CALLSPEC jag_addref(void* obj)
{
    if (obj)
        static_cast<jag::IRefCounted*>(static_cast<void*>(obj))->AddRef();
}



namespace
{
  struct error_record
  {
      error_record()
          : m_err_code(0)
      {}

      unsigned    m_err_code;
      std::string m_message;
  };


  // retrieves the error record for the current thread
  error_record* get_error_record()
  {
      // retrieve error record from thread specific storage; create it
      // if it does not exist
      error_record* rec = static_cast<error_record*>(
          jag::jstd::get_tls_data(jag::jstd::tss_id_error_record));

      if (!rec)
      {
          std::auto_ptr<error_record> err_rec(new error_record);
          jag::jstd::set_tls_data(jag::jstd::tss_id_error_record,
                                  err_rec.get(),
                                  jag::jstd::make_tls_cleanup<error_record>());

          rec = err_rec.release();
      }
      return rec;
  }
}



namespace jag {

void tls_set_error_info(exception const& exc)
{
    error_record* rec = get_error_record();

    JAG_ASSERT(rec);
    std::stringstream ss;
    output_exception(exc, ss);
    rec->m_message = ss.str();
    rec->m_err_code = exc.errcode();
}

} // namespace jag




JAG_EXPORT char const* JAG_CALLSPEC jag_last_error_msg(jag_error* code)
{
    error_record* rec = get_error_record();
    JAG_ASSERT(rec);

    if (code)
        *code = rec->m_err_code;

    return rec->m_message.c_str();
}


JAG_EXPORT jag_error JAG_CALLSPEC jag_error_code()
{
    error_record* rec = get_error_record();
    JAG_ASSERT(rec);
    return rec->m_err_code;
}

JAG_EXPORT const char* JAG_CALLSPEC jag_error_msg()
{
    error_record* rec = get_error_record();
    JAG_ASSERT(rec);
    return rec->m_message.c_str();
}

JAG_EXPORT void JAG_CALLSPEC jag_error_reset()
{
    error_record* rec = get_error_record();
    JAG_ASSERT(rec);
    rec->m_message.resize(0);
    rec->m_err_code = 0;
}



/** EOF @file */

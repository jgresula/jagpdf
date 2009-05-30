// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/icumain.h>
#include <unicode/udata.h>

namespace jag
{

//////////////////////////////////////////////////////////////////////////
boost::shared_ptr<UConverter> create_uconverter(char const* name)
{
    UErrorCode err = U_ZERO_ERROR;
    boost::shared_ptr<UConverter> conv(ucnv_open(name, &err), &ucnv_close);
    CHECK_ICU(err);
    return conv;
}

//////////////////////////////////////////////////////////////////////////
boost::shared_ptr<UConverter> create_uconverter_no_throw(char const* name)
{
    UErrorCode err = U_ZERO_ERROR;
    boost::shared_ptr<UConverter> conv(ucnv_open(name, &err), &ucnv_close);
    if (U_FAILURE(err))
        return boost::shared_ptr<UConverter>();

    return conv;
}

namespace
{
  //
  //
  //
  template<class FN, class SRC, class DST>
  void unicode_conversion(UConverter* conv,
                           FN const& fn,
                           SRC const* src_start, SRC const* src_end,
                           std::vector<DST>& output)
  {
      const std::size_t buffer_len = 1024;
      DST target_buffer[buffer_len];
      DST const*const target_end = target_buffer+buffer_len;

      UErrorCode err = U_ZERO_ERROR;
      do
      {
          err = U_ZERO_ERROR;
          DST* target_start = &target_buffer[0];
          fn(conv, &target_start, target_end, &src_start, src_end, NULL, 1, &err);
          output.insert(output.end(), &target_buffer[0], target_start);
      }
      while(err == U_BUFFER_OVERFLOW_ERROR);
      CHECK_ICU(err);
  }
}

//
//
//
void to_unicode(Char const* enc, char const* src_start, char const* src_end, std::vector<UChar>& output)
{
    to_unicode(create_uconverter(enc).get(),
                src_start, src_end,
                output);
}


//
//
//
void from_unicode(Char const* enc, UChar const* src_start, UChar const* src_end, std::vector<char>& output)
{
    from_unicode(create_uconverter(enc).get(),
                  src_start, src_end,
                  output);
}


//
//
//
void to_unicode(UConverter* conv,
                 char const* src_start, char const* src_end,
                 std::vector<UChar>& output)
{
    unicode_conversion(conv, ucnv_toUnicode, src_start, src_end, output);
}


//
//
//
void from_unicode(UConverter* conv,
                   UChar const* src_start, UChar const* src_end,
                   std::vector<char>& output)
{
    unicode_conversion(conv, ucnv_fromUnicode, src_start, src_end, output);
}


} // namespace jag

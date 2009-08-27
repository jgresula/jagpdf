// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CRT_H_GENERIC_822006
#define __CRT_H_GENERIC_822006

#include <core/jstd/crt_platform.h>
#include <interfaces/stdtypes.h>
#include <limits>


namespace jag {
/// portable crt based functionality
namespace jstd {

/**
 *  safe sprintf formatting, produces zero-terminated string
 *
 *  @param buffer destination
 *  @param count buffer size
 *  @param format formatting string
 *
 *  @return number of characters written (null is not included)
 */
int snprintf(Char* buffer, int count, Char const* format, ...);


/**
 *  Safe sprintf formatting, produces zero-terminated string.
 *
 *  @param buffer destination
 *  @param count buffer size
 *  @param format formatting string
 *
 *  Insufficient buffer length results to an internal error
 *
 *  @return number of characters written (null is not included)
 */
int snprintf_no_fail(Char* buffer, int count, Char const* format, ...);

const int PDF_DOUBLE_MAX_SIZE = std::numeric_limits<double>::digits10 + 10;

///
/// Converts double to string (PDF conformant).
///
/// Must be used for any floating point number which is send to PDF
/// output. PDF_DOUBLE_MAX_SIZE is the maximum size of the string.
///
int snprintf_pdf_double(Char* buffer, int count, double value);

}} //namespace jag:jstd


#endif //__CRT_H__822006

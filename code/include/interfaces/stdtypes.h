// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef STDTYPES_JG2312_H__
#define STDTYPES_JG2312_H__

// Check that the correct boost version is used. The check is here
// because we have prebuilt libraries from that version.
#include <boost/version.hpp>
#if !defined SWIG
# if !defined(BOOST_VERSION) || (BOOST_VERSION < 103500)
//#  error "Boost version 1.35.0 is required."
# endif
#endif

#include <jagpdf/detail/types.h>
#include <jagpdf/detail/errcodes.h>

/// Top level interface.
namespace jag
{
  // import the fundamental types into the jag namespace, they have to have full
  // qualification because of SWIG

  typedef jag::pdf::Int64 Int64;
  typedef jag::pdf::UInt64 UInt64;
  typedef jag::pdf::Char Char;
  typedef jag::pdf::Double Double;
  typedef jag::pdf::Byte Byte;
  typedef jag::pdf::Long Long;
  typedef jag::pdf::ULong ULong;
  typedef jag::pdf::Int Int;
  typedef jag::pdf::UInt UInt;
  typedef jag::pdf::UInt8 UInt8;
  typedef jag::pdf::UInt16 UInt16;


  const jag_Int err_no_error = jag::pdf::err_no_error;
  const jag_Int err_invalid_operation = jag::pdf::err_invalid_operation;
  const jag_Int err_io_error = jag::pdf::err_io_error;
  const jag_Int err_internal_error = jag::pdf::err_internal_error;
  const jag_Int err_invalid_input = jag::pdf::err_invalid_input;
  const jag_Int err_invalid_value = jag::pdf::err_invalid_value;
  const jag_Int err_operation_failed = jag::pdf::err_operation_failed;
}


// API_ATTR allows tagging a type with a string which can be retrieved from
// gccxml
#if defined(__GCCXML__)
 #define API_ATTR(s) __attribute__((gccxml(s)))

 // gccxml maps the following to type attributes as well, but it is not needed
 // as the latest version recognizes gccxml __attribute__ keyword even in msvc
 // mode

 // #define API_ATTR(s) __declspec(gccxml(s))
#else
 #define API_ATTR(s)
#endif

#endif // STDTYPES_JG2312_H__
/** EOF @file */

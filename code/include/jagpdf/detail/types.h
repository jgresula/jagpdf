/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *
 */
#ifndef PUBLICTYPES_H_JG2344__
#define PUBLICTYPES_H_JG2344__

#include <jagpdf/detail/config.h>

#define JAG_STATIC_ASSERT_PUBLIC(expr, msg)  \
    typedef int msg[(expr) ? 1 : -1 ];


JAG_STATIC_ASSERT_PUBLIC(sizeof(void*) == JAG_SIZEOF_VOID_P, incompatible_address_models)

#if (JAG_SIZEOF_VOID_P == 8)
# define JAG_64BIT_ADDRESS_MODEL
#endif



#if defined(_MSC_VER) && _MSC_VER < 1300
  /* msvc 6.0 does not have 'long long' */
  typedef __int64 jag_signed_integer_64;
  typedef unsigned __int64 jag_unsigned_integer_64;
#else
  typedef long long jag_signed_integer_64;
  typedef unsigned long long jag_unsigned_integer_64;
#endif



#if defined(JAG_64BIT_ADDRESS_MODEL)
/* ----------------------*/
/* 64 bits               */
/* ----------------------*/
JAG_STATIC_ASSERT_PUBLIC(sizeof(void*)==8, not_64bit_architecture)
# if defined(_MSC_VER)
   JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_signed_integer_64)==8 && sizeof(long)==sizeof(int) && sizeof(long)==4, not_LLP64)
#  define JAG_LLP64_64
# else
   JAG_STATIC_ASSERT_PUBLIC(sizeof(int)==4 && sizeof(long)==8, not_LP64)
#  define JAG_LP64_64
# endif
#else
/* ----------------------*/
/* 32 bits               */
/* ----------------------*/
JAG_STATIC_ASSERT_PUBLIC(sizeof(void*)==4, not_32bit_architecture)
# if defined(_MSC_VER)
   JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_signed_integer_64)==8 && sizeof(long)==sizeof(int) && sizeof(long)==4, not_LLP64)
#  define JAG_LLP64_32
# else
   JAG_STATIC_ASSERT_PUBLIC(sizeof(int)==4 && sizeof(long)==4, not_LP64)
#  define JAG_LP64_32
# endif
#endif

/*
  the typedefs follow LP64
  i.e. jag_Long, jag_ULong have width corresponding to address model
*/

/* ----------------------*/
#if defined(JAG_LLP64_64)
/* ----------------------*/
 typedef jag_signed_integer_64      jag_Long;
 typedef jag_unsigned_integer_64    jag_ULong;
 typedef jag_signed_integer_64      jag_Int64;
 typedef jag_unsigned_integer_64    jag_UInt64;
/* ----------------------*/
#elif defined(JAG_LLP64_32)
/* ----------------------*/
# if defined(_MSC_VER) && (_MSC_VER>=1300) && (_MSC_VER<1500)
 /* only for 6.0 < msvc < 9.0, _w64 and /Wp64 deprecated in 9.0 */
 /* http://msdn.microsoft.com/en-us/library/s04b5w00.aspx */
 typedef __w64 long                 jag_Long;
 typedef __w64 unsigned long        jag_ULong;
# else
 typedef long                       jag_Long;
 typedef unsigned long              jag_ULong;
# endif
 typedef jag_signed_integer_64      jag_Int64;
 typedef jag_unsigned_integer_64    jag_UInt64;
/* ----------------------*/
#elif defined(JAG_LP64_64)
/* ----------------------*/
 typedef long                       jag_Long;
 typedef unsigned long              jag_ULong;
 typedef long                       jag_Int64;
 typedef unsigned long              jag_UInt64;
/* ----------------------*/
#elif defined(JAG_LP64_32)
/* ----------------------*/
 typedef long                       jag_Long;
 typedef unsigned long              jag_ULong;
 typedef jag_signed_integer_64      jag_Int64;
 typedef jag_unsigned_integer_64    jag_UInt64;
#else
#error internal error - no address model specified
#endif

/* the same width regardless of the address model */
typedef char                jag_Char;
typedef double              jag_Double;
typedef unsigned char       jag_Byte;
typedef int                 jag_Int;
typedef unsigned int        jag_UInt;
typedef unsigned char       jag_UInt8;
typedef unsigned short      jag_UInt16;


#ifdef __cplusplus
namespace jag {
namespace pdf
{
  typedef jag_Int64   Int64;
  typedef jag_UInt64  UInt64;
  typedef jag_Char    Char;
  typedef jag_Double  Double;
  typedef jag_Byte    Byte;
  typedef jag_Long    Long;
  typedef jag_ULong   ULong;
  typedef jag_Int     Int;
  typedef jag_UInt    UInt;
  typedef jag_UInt8   UInt8;
  typedef jag_UInt16  UInt16;
}} /* namespace jag::pdf */
#endif


#ifndef SWIG
/* check type sizes */
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_Char)==1, jag_char_sizeof_problem_detected)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_UInt8)==1, jag_uint8_sizeof_problem_detected)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_UInt16)==2, jag_uint16_sizeof_problem_detected)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_Byte)==1, jag_byte_sizeof_problem_detected)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_Int)==4, jag_int_sizeof_problem_detected)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_UInt)==4, jag_uint_sizeof_problem_detected)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_Double)==8, jag_double_sizeof_problem_detected)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_Int64)==8, jag_int64_sizeof_problem_detected)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_UInt64)==8, jag_uint64_sizeof_problem_detected)

#  ifndef JAG_64BIT_ADDRESS_MODEL
  /* if the following assertions fail it means the we are compiling for a
     64-bit platform but JAG_64BIT_ADDRESS_MODEL is not defined */
  JAG_STATIC_ASSERT_PUBLIC(sizeof(void*)==4, jag_32_platform)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_Long)==4, jag_long_sizeof_problem)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_ULong)==4, jag_ulong_sizeof_problem)
#  else
  /* if the following assertions fail it means the we are compiling for a
     32-bit platform but JAG_64BIT_ADDRESS_MODEL is defined */
  JAG_STATIC_ASSERT_PUBLIC(sizeof(void*)==8, jag_64_bit_platform_not_specified)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_Long)==8, jag_long_sizeof_problem)
  JAG_STATIC_ASSERT_PUBLIC(sizeof(jag_ULong)==8, jag_ulong_sizeof_problem)
#  endif

#endif


#undef JAG_LLP64_64
#undef JAG_LLP64_32
#undef JAG_LP64_64
#undef JAG_LP64_32
#undef JAG_STATIC_ASSERT_PUBLIC

#endif



/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *
 */
#ifndef C_PROLOGUE_JG2219_H__
#define C_PROLOGUE_JG2219_H__

#include <jagpdf/detail/types.h>

#if defined(_MSC_VER)
# define JAG_CALLSPEC __cdecl
# if defined(JAGAPI_BUILDING_CPP)
# define JAG_EXPORT __declspec(dllexport)
# else
#  define JAG_EXPORT __declspec(dllimport)
# endif
#elif defined(__GNUC__)
# define JAG_CALLSPEC __attribute__((cdecl))
# if defined(JAGAPI_BUILDING_CPP)
#  define JAG_EXPORT __attribute__ ((visibility("default")))
# else
#  define JAG_EXPORT
# endif
#else
/* JAG_CALLSPEC should be defined for other compilers than gcc and msvc */
# define JAG_CALLSPEC
# define JAG_EXPORT
#endif

/* TBD: cdecl is not supported on amd64, not sure for IA64 or other platforms*/
#ifdef JAG_64BIT_ADDRESS_MODEL
# undef JAG_CALLSPEC
# define JAG_CALLSPEC
#endif 



#define JAG_GEN_UNIQUE_HANDLE(_name)    \
    typedef struct JAG_Opaque_##_name {int _;} *_name

typedef jag_Int jag_error;

/* other typedefs */
typedef jag_UInt jag_ColorSpace;
typedef jag_UInt jag_Pattern;
typedef jag_UInt jag_ImageMaskID;
typedef jag_UInt jag_Destination;
typedef jag_UInt jag_Function;


typedef struct jag_streamout_tag
{
    jag_Int (JAG_CALLSPEC *write)(void* custom_data, void const*, jag_ULong size);
    jag_Int (JAG_CALLSPEC *close)(void* custom_data);
    void *custom_data;
} jag_streamout;


#ifdef __cplusplus
extern "C"
{
#endif
    /* obj can be 0 */
    JAG_EXPORT void JAG_CALLSPEC jag_release(void* obj);
    JAG_EXPORT void JAG_CALLSPEC jag_addref(void* obj);

    /* params can be 0 */
    JAG_EXPORT const char* JAG_CALLSPEC jag_last_error_msg(jag_error* code);

    JAG_EXPORT const char* JAG_CALLSPEC jag_error_msg();
    JAG_EXPORT jag_error JAG_CALLSPEC jag_error_code();
    JAG_EXPORT void JAG_CALLSPEC jag_error_reset();
#ifdef __cplusplus
}
#endif



#endif
/** EOF @file */

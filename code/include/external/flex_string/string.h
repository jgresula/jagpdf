/**
 * @file string.h
 */
#ifndef __STRING_PDF_PLATFORM_H__
#define __STRING_PDF_PLATFORM_H__

#include "flex_string.h"

/// string: small-buffer (32 bytes) optimization + COW
typedef flex_string<
	char,
	std::char_traits<char>,
	std::allocator<char>,
	SmallStringOpt<CowStringOpt<AllocatorStringStorage<char, std::allocator<char> > >, 32 >
> string_32_cow;


#endif // __STRING_PDF_PLATFORM_H__

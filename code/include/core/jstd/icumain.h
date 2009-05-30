// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __ICUMAIN_H_JAG_1429__
#define __ICUMAIN_H_JAG_1429__

#include <msg_jstd.h>
#include <unicode/utypes.h>
#include <unicode/ucnv.h>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <vector>

#define CHECK_ICU(err)\
    if (U_FAILURE(err)) throw jag::exception_invalid_input(msg_icu_failed(err, u_errorName(err))) << JAGLOC;

#define CHECK_ICU_FATAL(err)\
    if (U_FAILURE(err)) throw jag::exception_internal_error(msg_internal_error_si(u_errorName(err), err)) << JAGLOC;

BOOST_STATIC_ASSERT(sizeof(UChar)==2);

namespace jag {

/**
 * @brief creates icu converter with given name
 *
 * Documentation states that it should be a cheap operation.
 * Converters can't be shared among threads.
 *
 * Throws an exception if the converter can't be created.
 *
 * @return a converter instance
 */
boost::shared_ptr<UConverter> create_uconverter(char const* name);

/**
 * @brief creates icu converter with given name
 *
 * Documentation states that it should be a cheap operation.
 * Converters can't be shared among threads.
 *
 * @return a converter instance, or null if an error occurs
 */
boost::shared_ptr<UConverter> create_uconverter_no_throw(char const* name);


/**
 * @brief converts multibyte string to unicode using given converter
 *
 * @param conv converter to use
 * @param start multibyte multibyte string start
 * @param end  pointer past the last multibyte string character
 * @param unicode unicode string (not zero terminated)
 */
void to_unicode(UConverter* conv, char const* src_start, char const* src_end, std::vector<UChar>& output);
void to_unicode(Char const* enc, char const* src_start, char const* src_end, std::vector<UChar>& output);

void from_unicode(UConverter* conv, UChar const* src_start, UChar const* src_end, std::vector<char>& output);
void from_unicode(Char const* enc, UChar const* src_start, UChar const* src_end, std::vector<char>& output);


} // namespace jag

#endif //__ICUMAIN_H_JAG_1429__

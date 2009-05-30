// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __USEDCHARCODES8_H_JAG_2214__
#define __USEDCHARCODES8_H_JAG_2214__

#include <core/generic/assert.h>
#include <interfaces/stdtypes.h>
#include <utility>
#include <unicode/utypes.h>

namespace jag { namespace pdf
{

/**
 * @brief used characters collector for 8-bit encodings
 *
 * Characters are not decoded at all as they are going to appear in the
 * pdf as they came in.
 */
class UsedCharCodes8
{
public:
    UsedCharCodes8();

public:
    typedef std::pair<UChar32,UChar32> Range;

    void on_char_codes(Char const* seq, size_t byte_length);
    Range whole_range() const;
    bool is_empty() const;

private:
    Range                m_bounds;
};

}} //namespace jag::pdf


#endif //__USEDCHARCODES8_H_JAG_2214__

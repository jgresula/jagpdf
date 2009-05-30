// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CONVERSIONS_H_JG1130__
#define __CONVERSIONS_H_JG1130__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/stdtypes.h>
#include <unicode/utypes.h>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

namespace jag {
namespace jstd {

// inefficient as preflight is used
class FromUTF8
    : public boost::noncopyable
{
public:
    FromUTF8(Char const* src, UInt length = 0);
    UChar const* to_utf16() const { return m_utf16 ? m_utf16.get() : 0; }
private:
    boost::scoped_array<UChar> m_utf16;
};


}} //namespace jag::jstd


#endif //__CONVERSIONS_H_JG1130__

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __UCONVERTERCTRL_JG113_H__
#define __UCONVERTERCTRL_JG113_H__

#include <core/jstd/unicode.h>
#include <boost/scoped_ptr.hpp>
#include <string>

namespace jag {
namespace jstd {

class UConverterCtrl
{
    mutable boost::scoped_ptr<jstd::UnicodeConverter>   m_conv_muster;
    mutable bool                                           m_converter_acquired;
    std::string                                            m_encoding;

public:
    /**
     * @brief Ctor.
     * @param encoding encoding name, if an empty string is passed
     *             then acquire_converter() returns NULL
     */
    UConverterCtrl(Char const* encoding);
    jstd::UnicodeConverter* acquire_converter() const;
    void release_converter() const;
};


}} // namespace jag::jstd

#endif // __UCONVERTERCTRL_JG113_H__
/** EOF @file */

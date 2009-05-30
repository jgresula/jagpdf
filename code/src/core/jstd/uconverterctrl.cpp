// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/uconverterctrl.h>
#include <core/generic/assert.h>
#include <core/generic/stringutils.h>

namespace jag {
namespace jstd {

UConverterCtrl::UConverterCtrl(Char const* encoding)
    : m_converter_acquired(false)
{
    if (!is_empty(encoding))
        m_encoding = encoding;
}


UnicodeConverter* UConverterCtrl::acquire_converter() const
{
    JAG_PRECONDITION(!m_converter_acquired);
    if (!m_conv_muster && !m_encoding.empty())
        m_conv_muster.reset(new UnicodeConverter(m_encoding.c_str()));

    m_converter_acquired = true;

    // recycle converter
    if (m_conv_muster)
        m_conv_muster->reset();

    return m_conv_muster.get();
}



void UConverterCtrl::release_converter() const
{
    JAG_PRECONDITION(m_converter_acquired);
    m_converter_acquired = false;
}


}} // namespace jag::jstd

/** EOF @file */

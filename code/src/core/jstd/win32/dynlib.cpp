// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <windows.h>
#include <stdexcept>
#include <core/generic/assert.h>
#include <core/jstd/dynlib.h>

namespace jag {
namespace jstd {


DynamicLibrary::DynamicLibrary(char const* name)
{
    std::string module(name);
    module += ".dll";
    m_hlib = ::LoadLibrary(module.c_str());
    if (!m_hlib)
        throw std::runtime_error("cannot load library");
}


DynamicLibrary::~DynamicLibrary()
{
    JAG_ASSERT(m_hlib);
    if (!::FreeLibrary(m_hlib))
        JAG_ASSERT(!"free library failed");
}


void* DynamicLibrary::symbol(char const* sym) const
{
    void* symbol = ::GetProcAddress(m_hlib, sym);
    if (!symbol)
        throw std::runtime_error("symbol loading failed");

    return symbol;
}

}} // namespace jag::jstd


/** EOF @file */

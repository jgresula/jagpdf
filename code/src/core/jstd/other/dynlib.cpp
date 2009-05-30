// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <stdexcept>
#include <core/generic/assert.h>
#include <core/jstd/dynlib.h>
#include <dlfcn.h>

namespace jag {
namespace jstd {


DynamicLibrary::DynamicLibrary(char const* name)
{
    std::string module(name);
#ifdef __CYGWIN__
    module += ".dll";
#else
    module += ".so";
#endif
    m_hlib = dlopen(module.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!m_hlib)
        throw std::runtime_error("cannot load library");
}


DynamicLibrary::~DynamicLibrary()
{
    JAG_ASSERT(m_hlib);
    if (dlclose(m_hlib))
        JAG_ASSERT(!"free library failed");
}


void* DynamicLibrary::symbol(char const* sym) const
{
    void* symbol = dlsym(m_hlib, sym);
    if (!symbol)
        throw std::runtime_error("symbol loading failed");

    return symbol;
}


}} // namespace jag::jstd


/** EOF @file */

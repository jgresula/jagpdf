// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/execcontextimpl.h>
#include <interfaces/configinternal.h>
#include <core/generic/checked_cast.h>

using namespace boost;

namespace jag {
namespace jstd {


ExecContextImpl::ExecContextImpl(IProfileInternal const& cfg)
{
    m_config = cfg.clone();
}


ExecContextImpl::~ExecContextImpl()
{
}

}} // namespace jag::jstd

/** EOF @file */

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef EXECCONTEXTIMPL_JG1247_H__
#define EXECCONTEXTIMPL_JG1247_H__

#include <interfaces/execcontext.h>
#include <boost/intrusive_ptr.hpp>

namespace jag {
namespace jstd {

class ExecContextImpl
    : public IExecContext
{
public: //IExecContext
    IProfileInternal const& config() const { return *m_config;  }
    
    // use with care, profile should remain const for most of the options
    IProfileInternal& writable_config() const { return *m_config;  }

public:
    explicit ExecContextImpl(IProfileInternal const& cfg);
    ~ExecContextImpl();

private:
    boost::intrusive_ptr<IProfileInternal> m_config;
};


}} // namespace jag::jstd

#endif // EXECCONTEXTIMPL_JG1247_H__
/** EOF @file */

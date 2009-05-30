// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef EXECONTEXT_JG1238_H__
#define EXECONTEXT_JG1238_H__

#include <boost/noncopyable.hpp>

namespace jag {
class IProfileInternal;

//
//
//
class IExecContext
    : public boost::noncopyable
{
public:
    virtual IProfileInternal const& config() const = 0;

protected:
    ~IExecContext() {}
};


} // namespace jag

#endif // EXECONTEXT_JG1238_H__
/** EOF @file */

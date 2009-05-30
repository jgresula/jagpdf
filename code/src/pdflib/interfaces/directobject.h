// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef DIRECTOBJECT_JG1439_H__
#define DIRECTOBJECT_JG1439_H__

#include <boost/noncopyable.hpp>

namespace jag {
namespace pdf {

class ObjFmt;

class IDirectObject
    : public boost::noncopyable
{
public:
    virtual void output_object(ObjFmt& fmt) = 0;
    virtual ~IDirectObject() {}
};


}} // namespace jag

#endif // DIRECTOBJECT_JG1439_H__
/** EOF @file */

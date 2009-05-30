// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __OBJFMT_REPORT_H__
#define __OBJFMT_REPORT_H__

#include "object_type.h"
#include <boost/noncopyable.hpp>

namespace jag { namespace pdf
{

//fwd
class IIndirectObject;

class IObjFmtReport
    : public boost::noncopyable
{
public:
    virtual void object_start(ObjectType type, IIndirectObject& obj) = 0;
    virtual void object_end(ObjectType type, IIndirectObject& obj) = 0;

protected:
    ~IObjFmtReport() {}
};

}} //namespace jag::pdf

#endif //__OBJFMT_REPORT_H__

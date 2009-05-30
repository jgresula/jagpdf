// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __VISITORNOOP_H__2145__
#define __VISITORNOOP_H__2145__

#include "interfaces/visitor.h"
#include <core/generic/noncopyable.h>


#define PDF_VISITOR_NOOP_METHOD(TYPE) virtual bool visit(TYPE&) { return true; }

namespace jag { namespace pdf
{

class VisitorNoOp
    : public IVisitor
{
public:
    PDF_VISITABLE_OBJECTS(PDF_VISITOR_NOOP_METHOD);
};


}}  // namespace jag::pdf

#undef PDF_VISITOR_NOOP_METHOD

#endif //__VISITORNOOP_H__2145__

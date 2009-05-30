// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __VISITORTHROW_H__2151__
#define __VISITORTHROW_H__2151__

#include "interfaces/visitor.h"
#include <core/generic/noncopyable.h>
#include <msg_pdflib.h>


#define PDF_VISITOR_THROW_METHOD(TYPE)\
    bool visit(TYPE&) { JAG_INTERNAL_ERROR; }

namespace jag { namespace pdf
{

class VisitorThrow
    : public IVisitor
{
public:
    PDF_VISITABLE_OBJECTS(PDF_VISITOR_THROW_METHOD);
};


}}  // namespace jag::pdf

#undef PDF_VISITOR_THROW_METHOD

#endif //__VISITORTHROW_H__2151__

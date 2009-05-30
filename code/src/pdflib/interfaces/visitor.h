// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __VISITOR_H__
#define __VISITOR_H__

#include <core/generic/noncopyable.h>


#define PDF_VISITABLE_OBJECTS(SUBST_MACRO)    \
    SUBST_MACRO(IPdfObject)                   \
    SUBST_MACRO(PageObject)                   \
    SUBST_MACRO(PageTreeNode)                    \
    SUBST_MACRO(TilingPatternImpl)              \
    SUBST_MACRO(ShadingPatternImpl)

#define PDF_PURE_VISIT_METHOD(TYPE) virtual bool visit(TYPE&) = 0;
#define PDF_FORWARD_DECLARATION(TYPE) class TYPE;

namespace jag { namespace pdf
{
//fwd
PDF_VISITABLE_OBJECTS(PDF_FORWARD_DECLARATION);

class IVisitor
    : public noncopyable
{
public:
    PDF_VISITABLE_OBJECTS(PDF_PURE_VISIT_METHOD);
};


}}  // namespace jag::pdf

#undef PDF_PURE_VISIT_METHOD
#undef PDF_FORWARD_DECLARATION

#endif //__VISITOR_H__

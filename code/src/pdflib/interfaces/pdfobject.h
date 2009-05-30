// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PDFOBJECT_H__
#define __PDFOBJECT_H__

#include "visitor.h"
#include <core/generic/noncopyable.h>

namespace jag {
namespace pdf {
// fwd
class IVisitor;


class IPdfObject
    : public noncopyable
{
public:
    virtual bool accept(IVisitor&) = 0;
};

#define DEFINE_VISITABLE \
  bool accept(IVisitor& v) { return v.visit(*this); }


}} //namespace jag::pdf

#endif //__PDFOBJECT_H__

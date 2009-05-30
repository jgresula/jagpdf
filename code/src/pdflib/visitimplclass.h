// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __VISITIMPLCLASS_H_JG2141__
#define __VISITIMPLCLASS_H_JG2141__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "visitorthrow.h"
#include <boost/intrusive_ptr.hpp>

namespace jag { namespace pdf
{

template<class T>
class VisitImplClass
    : public VisitorThrow
{
public:
    explicit VisitImplClass(IPdfObject* obj) {
        obj->accept(*this);
    }


public: //IVisitor
    bool visit(T& obj)
    {
        m_obj = &obj;
        return true;
    }


public: //access
    T* get() const {
        return m_obj.get();
    }

private:
    boost::intrusive_ptr<T>    m_obj;
};


}} //namespace jag::pdf


#endif //__VISITIMPLCLASS_H_JG2141__




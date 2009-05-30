// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __REFCOUNTED_H_JG1516__
#define __REFCOUNTED_H_JG1516__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/stdtypes.h>
#include <core/generic/noncopyable.h>

namespace jag {

#if !defined(SWIG)

/// Inidicates that the object is not reference counted
class INotRefCounted
{
protected:
    ~INotRefCounted() {}
};

/// reference counting
class IRefCounted
    : public noncopyable
{
public:
    virtual void AddRef() API_ATTR("internal") = 0;
    virtual void Release() API_ATTR("internal") = 0;

protected:
    ~IRefCounted() {}
};

#endif

} //namespace jag

namespace boost {

inline void intrusive_ptr_add_ref(jag::IRefCounted* obj)
{
    obj->AddRef();
}

inline void intrusive_ptr_release(jag::IRefCounted* obj)
{
    obj->Release();
}

}





#endif //__REFCOUNTED_H_JG1516__


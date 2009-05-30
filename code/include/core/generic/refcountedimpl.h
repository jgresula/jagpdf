// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

// Portions of this file are
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//
#ifndef __REFCOUNTEDIMPL_H_JG1518__
#define __REFCOUNTEDIMPL_H_JG1518__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <core/generic/assert.h>
#include <interfaces/refcounted.h>

// include platform specific thread-safe counter
//
// This compile-time switch is taken from boost/detail/atomic_count.hpp There
// are more platform/compiler specific implementations in boost. Consider using
// them when porting to other platforms.
//
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
# include <core/generic/refcountedimpl_gcc_x86.h>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
# include <core/generic/refcountedimpl_w32.h>
#else
# include <core/generic/refcountedimpl_pthreads.hpp>
#endif


namespace jag {

//
//
//
class RefCountBase
    : public IRefCounted
{
    int m_count;

protected:
    RefCountBase()
        : m_count(0)
    {}

    virtual ~RefCountBase() {}

public: // jag::IRefCounted
    void AddRef() {
        ++m_count;
    }

    void Release() {
        JAG_ASSERT_MSG(m_count > 0, "inconsistent reference count");
        if (!--m_count)
            delete this;
    }
};


/// implementation of IRefCounted
template<class T, class Counter=RefCountBase>
class RefCountImpl
    : public T
    , public Counter
{
public:
    RefCountImpl() {}

    template<class P1>
    RefCountImpl(P1& p1)
        : T(p1)
    {}

    template<class P1, class P2>
    RefCountImpl(P1& p1, P2& p2)
        : T(p1, p2)
    {}

    template<class P1, class P2, class P3>
    RefCountImpl(P1& p1, P2& p2, P3& p3)
        : T(p1, p2, p3)
    {}

public: // IRefCounted
    void AddRef() {
        Counter::AddRef();
    }

    void Release() {
        Counter::Release();
    }
};


} //namespace jag


#endif //__REFCOUNTEDIMPL_H_JG1518__


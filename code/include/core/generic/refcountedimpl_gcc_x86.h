// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

//  Portions of this file are
//  Copyright (c) 2001, 2002, 2003 Peter Dimov and Multi Media Ltd.
//  Copyright 2004-2005 Peter Dimov

#ifndef REFCOUNTEDIMPL_GCC_X86_JG1541_H__
#define REFCOUNTEDIMPL_GCC_X86_JG1541_H__

#include <interfaces/refcounted.h>
#include <boost/noncopyable.hpp>

namespace jag {
namespace detail {

inline int atomic_exchange_and_add(int * pw, int dv)
{
    // int r = *pw;
    // *pw += dv;
    // return r;

    int r;

    __asm__ __volatile__
    (
        "lock\n\t"
        "xadd %1, %0":
        "=m"(*pw), "=r"(r): // outputs (%0, %1)
        "m"(*pw), "1"(dv): // inputs (%2, %3 == %1)
        "memory", "cc" // clobbers
    );

    return r;
}


inline void atomic_increment(int * pw)
{
    //atomic_exchange_and_add(pw, 1);

    __asm__
    (
        "lock\n\t"
        "incl %0":
        "=m"(*pw): // output (%0)
        "m"(*pw): // input (%1)
        "cc" // clobbers
    );
}

} // namespace detail

/// thread safe implementation of IRefCounted for x86 compiled with gcc
class RefCountMT
    : public IRefCounted
{
    int m_count;

protected:
    RefCountMT()
        : m_count(0)
    {}

    virtual ~RefCountMT() {}

public: // jag::IRefCounted
    void AddRef() {
        detail::atomic_increment(&m_count);
    }

    void Release() {
        if (detail::atomic_exchange_and_add(&m_count, -1) == 1)
            delete this;
    }
};


//
// thread-safe implementation of reference counting; it does not perform any
// action on addref, release; just signals when the counter is addrefed from 0
// to 1 and released from 1 to 0
//
class RefCountMTSignal
    : public boost::noncopyable
{
    int m_count;

public:
    RefCountMTSignal()
        : m_count(0)
    {}

    // returns true if counter goes from 0 -> 1, false otherwise
    bool addref()
    {
        return detail::atomic_exchange_and_add(&m_count, 1) == 0;
    }

    // return true if counter goes from 1 -> 0, false otherwise
    bool release()
    {
        return detail::atomic_exchange_and_add(&m_count, -1) == 1;
    }
};


} // namespace jag

#endif // REFCOUNTEDIMPL_GCC_X86_JG1541_H__
/** EOF @file */

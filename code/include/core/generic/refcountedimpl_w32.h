// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef REFCOUNTEDIMPL_W32_JG1541_H__
#define REFCOUNTEDIMPL_W32_JG1541_H__

#include <interfaces/refcounted.h>
#include <boost/detail/interlocked.hpp>
#include <boost/noncopyable.hpp>

namespace jag {

/// thread safe implementation of IRefCounted for x86 on windows
class RefCountMT
    : public IRefCounted
{
    long m_count;

protected:
    RefCountMT()
        : m_count(0)
    {}

    virtual ~RefCountMT() {}

public: // jag::IRefCounted
    void AddRef() {
        BOOST_INTERLOCKED_INCREMENT(&m_count);
    }

    void Release() {
        if (BOOST_INTERLOCKED_DECREMENT(&m_count)==0)
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
    long m_count;

public:
    RefCountMTSignal()
        : m_count(0)
    {}

    // returns true if counter goes from 0 -> 1, false otherwise
    bool addref()
    {
        return BOOST_INTERLOCKED_INCREMENT(&m_count) == 1;
    }

    // return true if counter goes from 1 -> 0, false otherwise
    bool release()
    {
        return BOOST_INTERLOCKED_DECREMENT(&m_count) == 0;
    }
};


} // namespace jag

#endif // REFCOUNTEDIMPL_W32_JG1541_H__
/** EOF @file */

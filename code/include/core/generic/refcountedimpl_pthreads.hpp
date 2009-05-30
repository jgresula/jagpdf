// Copyright (c) 2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

// Portions of this code are
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//
#ifndef REFCOUNTEDIMPL_PTHREADS_JG818_HPP__
#define REFCOUNTEDIMPL_PTHREADS_JG818_HPP__

#include <core/generic/assert.h>
#include <pthread.h>
#include <interfaces/refcounted.h>
#include <boost/noncopyable.hpp>

namespace jag {
namespace detail {

//
//
//
class scoped_lock
{
public:
    scoped_lock(pthread_mutex_t & m)
        : m_(m)
    {
        pthread_mutex_lock(&m_);
    }

    ~scoped_lock()
    {
        pthread_mutex_unlock(&m_);
    }

private:
    pthread_mutex_t & m_;
};

} // namespace detail


//
//
//
class RefCountMT
    : public IRefCounted
{
public:
    RefCountMT()
        : value_(0)
    {
        pthread_mutex_init(&mutex_, 0);
    }

    virtual ~RefCountMT()
    {
        pthread_mutex_destroy(&mutex_);
    }

    void AddRef()
    {
        detail::scoped_lock lock(mutex_);
        ++value_;
    }

    void Release()
    {
        detail::scoped_lock lock(mutex_);
        JAG_ASSERT(value_ > 0);
        if (0 == --value_)
            delete this;
    }

private:
    pthread_mutex_t mutex_;
    long value_;
};


//
// thread-safe implementation of reference counting; it does not perform any
// action on addref, release; just signals when the counter is addrefed from 0
// to 1 and released from 1 to 0
//
class RefCountMTSignal
    : public boost::noncopyable
{
public:
    RefCountMTSignal()
        : value_(0)
    {
        pthread_mutex_init(&mutex_, 0);
    }

    ~RefCountMTSignal()
    {
        pthread_mutex_destroy(&mutex_);
    }

    // returns true if counter goes from 0 -> 1, false otherwise
    bool addref()
    {
        detail::scoped_lock lock(mutex_);
        return 1 == ++value_;
    }

    // return true if counter goes from 1 -> 0, false otherwise
    bool release()
    {
        detail::scoped_lock lock(mutex_);
        return 0 == --value_;
    }

private:
    pthread_mutex_t mutex_;
    long value_;
};

} // namespace jag::detail

#endif // REFCOUNTEDIMPL_PTHREADS_JG818_HPP__

/** EOF @file */

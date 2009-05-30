// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef APISTRUCTURES_JG1950_H__
#define APISTRUCTURES_JG1950_H__

#include <interfaces/stdtypes.h>
#include <boost/noncopyable.hpp>
#include <interfaces/refcounted.h>
#include <core/generic/macros.h>
#include <core/generic/refcountedimpl.h>

// temporary
#include <stdio.h>
// #include <assert.h>

namespace jag {

namespace detail {

#if !defined(SWIG)

// we need to throw outside this file in order to get rid of dependency on
// msg_errlib.h - this file is parsed by gccxml and thus it would not be
// possible to build documentation from sources without building the sources
// first.
void throw_virtual_not_implemented(char const* fun_desc);


// This interface exists, because StreamOut::check_error() must be virtual,
// otherwise it would be an unresolved symbol. The method is implemented in a
// SWIG module.
struct IErrorChecking
{
    virtual void check_error() = 0;
};

#endif //!SWIG
} // namespace detail



namespace apiinternal {

/// This is an interface that can be extended by other languages.
///
/// It should not be used internally.
class StreamOut
    : public IRefCounted
    , public detail::IErrorChecking
{
public:
    // do not remove because of swig
    StreamOut() { /*printf("CTOR: %d\n", ++s_counter);*/ }
    virtual ~StreamOut() { /*printf("DTOR: %d\n", --s_counter);*/ }

public:
    // to be implemented in the target language; the default implementation
    // throws an exception

    // do not change argument names as they used in SWIG typemaps
    virtual void write(void const* data_in, jag::ULong length) {
        JAG_UNUSED_FUNCTION_ARGUMENT(data_in);
        JAG_UNUSED_FUNCTION_ARGUMENT(length);
        detail::throw_virtual_not_implemented("StreamOut::write()");
    }

    virtual void close() {
        detail::throw_virtual_not_implemented("StreamOut::close()");
    }

// to be implemented in SWIG module
public: //IRefCounted
    void AddRef();
    void Release();
public:
    //
    void check_error();

private:
    // thread-safe counter that can be used by AddRef/Release implementations
    RefCountMTSignal m_counter;
};


}} // namespace jag

#endif // APISTRUCTURES_JG1950_H__
/** EOF @file */

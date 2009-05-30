// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#ifndef __SMARTPTRUTILS_H_JAG_2146__
#define __SMARTPTRUTILS_H_JAG_2146__

#include <core/generic/assert.h>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>


namespace jag {

template<class T> struct intrusive_deleter
{
    void operator()(T * p)
    {
        if(p) intrusive_ptr_release(p);
    }
};



template <class T>
boost::shared_ptr<T> make_shared_from_intrusive(T * p)
{
    JAG_PRECONDITION(p);
    if(p)
        intrusive_ptr_add_ref(p);
    boost::shared_ptr<T> px(p, intrusive_deleter<T>());
    return px;
}


/**
 * @brief Adapter among operator(shared_ptr<T>) and operator(T&)
 *
 * Defines unary operator() taking shared_ptr<T> const&, dereferences it
 * and passes it to A's operator()
 *
 * @param T operand
 * @param A adaptable, defines operator(T&)
 */
template<class T, class A>
struct shared_ptr_unfold_unary
    : public std::unary_function<boost::shared_ptr<T> const&, typename A::result_type>
{
    typename A::result_type operator()(boost::shared_ptr<T> const& obj) const
    {
        return A()(*obj);
    }
};


/**
 * @brief Adapter among operator(shared_ptr<T>, shared_ptr<T>) and operator(T&, T&)
 *
 * Defines binary operator() taking shared_ptr<T> const&, dereferences them
 * and passes them to A's operator()
 *
 * @param T operand
 * @param A adaptable, defines operator(T&, T&)
 */
template<class T, class A>
struct shared_ptr_unfold_binary
    : public std::binary_function<
          boost::shared_ptr<T> const&
        , boost::shared_ptr<T> const&
        , typename A::result_type
    >
{
    typename A::result_type operator()(boost::shared_ptr<T> const& lhs, boost::shared_ptr<T> const& rhs) const
    {
        return A()(*lhs, *rhs);
    }
};



/**
 * @brief Adapter among operator(intrusive_ptr<T>) and operator(T&)
 *
 * Defines unary operator() taking intrusive_ptr<T> const&, dereferences it
 * and passes it to A's operator()
 *
 * @param T operand
 * @param A adaptable, defines operator(T&)
 */
template<class T, class A>
struct intrusive_ptr_unfold_unary
    : public std::unary_function<boost::intrusive_ptr<T> const&, typename A::result_type>
{
    typename A::result_type operator()(boost::intrusive_ptr<T> const& obj) const
    {
        return A()(*obj);
    }
};


/**
 * @brief Adapter among operator(intrusive_ptr<T>, intrusive_ptr<T>) and operator(T&, T&)
 *
 * Defines binary operator() taking intrusive_ptr<T> const&, dereferences them
 * and passes them to A's operator()
 *
 * @param T operand
 * @param A adaptable, defines operator(T&, T&)
 */
template<class T, class A>
struct intrusive_ptr_unfold_binary
    : public std::binary_function<
          boost::intrusive_ptr<T> const&
        , boost::intrusive_ptr<T> const&
        , typename A::result_type
    >
{
    typename A::result_type
    operator()(boost::intrusive_ptr<T> const& lhs, boost::intrusive_ptr<T> const& rhs) const
    {
        return A()(*lhs, *rhs);
    }
};


} // namespace jag

#endif //__SMARTPTRUTILS_H_JAG_2146__


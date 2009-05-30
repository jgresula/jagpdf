// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "objfmtbasic.h"
#include "graphicsstatedictionary.h"
#include <core/generic/floatpointtools.h>
#include <boost/functional/hash.hpp>
#include "boost/tuple/tuple_comparison.hpp"

namespace tuples = boost::tuples;


namespace jag { namespace pdf
{

///ctor
GraphicsStateDictionary::GraphicsStateDictionary()
    : m_alpha_is_shape(false)
    , m_stroking_alpha(1.0)
    , m_nonstroking_alpha(1.0)
{
}

//////////////////////////////////////////////////////////////////////////
bool operator==(GraphicsStateDictionary const& lhs, GraphicsStateDictionary const& rhs)
{
    return
            lhs.m_alpha_is_shape == rhs.m_alpha_is_shape
        &&  equal_doubles(lhs.m_stroking_alpha, rhs.m_stroking_alpha)
        &&  equal_doubles(lhs.m_nonstroking_alpha, rhs.m_nonstroking_alpha)
        &&  lhs.m_transfer_fn == rhs.m_transfer_fn
    ;
}

//////////////////////////////////////////////////////////////////////////
bool operator<(GraphicsStateDictionary const& lhs, GraphicsStateDictionary const& rhs)
{
    return
        tuples::tie(lhs.m_alpha_is_shape, lhs.m_stroking_alpha, lhs.m_nonstroking_alpha, lhs.m_transfer_fn)
        <
        tuples::tie(rhs.m_alpha_is_shape, rhs.m_stroking_alpha, rhs.m_nonstroking_alpha, rhs.m_transfer_fn)
    ;
}

//////////////////////////////////////////////////////////////////////////
size_t hash_value(GraphicsStateDictionary const& gs_dict)
{
    size_t seed = 0;
    boost::hash_combine(seed, gs_dict.m_param_changed.to_ulong());
    boost::hash_combine(seed, gs_dict.m_alpha_is_shape);
    boost::hash_combine(seed, gs_dict.m_stroking_alpha);
    boost::hash_combine(seed, gs_dict.m_nonstroking_alpha);
    boost::hash_combine(seed, gs_dict.m_transfer_fn.id());

    return seed;
}


}} //namespace jag::pdf

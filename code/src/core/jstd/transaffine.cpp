// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/transaffine.h>
#include <core/generic/assert.h>
#include <string.h>
#include <math.h>

namespace jag {
namespace jstd {


//
//
//
trans_affine_t::trans_affine_t(Double const* data)
{
    memcpy(m_data, data, 6 * sizeof(Double));
}

//
//
//
trans_affine_t::trans_affine_t(Double a, Double b, Double c,
                               Double d, Double e, Double f)
{
    m_data[0] = a;
    m_data[1] = b;
    m_data[2] = c;
    m_data[3] = d;
    m_data[4] = e;
    m_data[5] = f;
}


//
//
//
void trans_affine_t::set(Double a, Double b, Double c,
                         Double d, Double e, Double f)
{
    m_data[0] = a;
    m_data[1] = b;
    m_data[2] = c;
    m_data[3] = d;
    m_data[4] = e;
    m_data[5] = f;
}


///
/// Premultiplies the matrix with other (i.e other * this)
///
trans_affine_t& trans_affine_t::operator*=(trans_affine_t const& other)
{
    trans_affine_t this_(*this);

    m_data[0] = other.m_data[0] * this_.m_data[0] + other.m_data[1] * this_.m_data[2];
    m_data[1] = other.m_data[0] * this_.m_data[1] + other.m_data[1] * this_.m_data[3];
    m_data[2] = other.m_data[2] * this_.m_data[0] + other.m_data[3] * this_.m_data[2];
    m_data[3] = other.m_data[2] * this_.m_data[1] + other.m_data[3] * this_.m_data[3];
    m_data[4] = other.m_data[4] * this_.m_data[0] + other.m_data[5] * this_.m_data[2] + this_.m_data[4];
    m_data[5] = other.m_data[4] * this_.m_data[1] + other.m_data[5] * this_.m_data[3] + this_.m_data[5];

    return *this;
}

trans_affine_t& trans_affine_t::translate(Double tx, Double ty)
{
    trans_affine_t this_(*this);
    m_data[4] = tx * this_.m_data[0] + ty * this_.m_data[2] + this_.m_data[4];
    m_data[5] = tx * this_.m_data[1] + ty * this_.m_data[3] + this_.m_data[5];
    return *this;
}

trans_affine_t& trans_affine_t::scale(Double sx, Double sy)
{
    trans_affine_t this_(*this);
        
    m_data[0] = sx * this_.m_data[0];
    m_data[1] = sx * this_.m_data[1];
    m_data[2] = sy * this_.m_data[2];
    m_data[3] = sy * this_.m_data[3];
    return *this;
}

///
/// Transforms a vector.
///
void trans_affine_t::transform(Double *x, Double *y) const
{
    JAG_PRECONDITION(x);
    JAG_PRECONDITION(y);

    Double x_ = *x;
    *x = m_data[0] * x_ + m_data[2] * *y + m_data[4];
    *y = m_data[1] * x_ + m_data[3] * *y + m_data[5];
}


//
//
//
trans_affine_t trans_affine_rotation(Double angle)
{
    Double cosa = cos(angle);
    Double sina = sin(angle);

    return trans_affine_t(cosa, sina, -sina, cosa, 0, 0);
}

//
//
//
trans_affine_t trans_affine_translation(Double tx, Double ty)
{
    return trans_affine_t(1.0, 0.0, 0.0, 1.0, tx, ty);
}

//
//
// 
trans_affine_t trans_affine_scaling(Double sx, Double sy)
{
        return trans_affine_t(sx, 0.0, 0.0, sy, 0, 0);
}

}} // namespace jag::jstd

/** EOF @file */

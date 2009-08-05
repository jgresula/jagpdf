// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef TRANS_AFFINE_JG859_H__
#define TRANS_AFFINE_JG859_H__

#include <interfaces/stdtypes.h>

namespace jag {
namespace jstd {

/// Represents affine transformations
class trans_affine_t
{
    Double m_data[6];

public: // construction/modification
    trans_affine_t(Double const* data);
    trans_affine_t(Double a, Double b, Double c,
                   Double d, Double e, Double f);
    void set(Double a, Double b, Double c,
             Double d, Double e, Double f);


public: // operations
    void transform(Double *x, Double *y) const;
    trans_affine_t& operator*=(trans_affine_t const& other);
    trans_affine_t& scale(Double sx, Double sy);
    trans_affine_t& translate(Double tx, Double ty);

public:
    Double const* data() const { return m_data; }
};


// creators
trans_affine_t trans_affine_rotation(Double angle);
trans_affine_t trans_affine_translation(Double tx, Double ty);
trans_affine_t trans_affine_scaling(Double sx, Double sy);


}} // namespace jag::jstd

#endif // TRANS_AFFINE_JG859_H__
/** EOF @file */

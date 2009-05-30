// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PATTERNCOLORSPACE_JG2254_H__
#define __PATTERNCOLORSPACE_JG2254_H__

#include <resources/resourcebox/colorspacehelpers.h>
#include <resources/othermanagers/colorspacevisitor.h>
#include <core/generic/assert.h>
#include <core/generic/refcountedimpl.h>
#include <resources/interfaces/resourcehandle.h>
#include <resources/interfaces/colorspaces.h>

namespace jag {
namespace pdf {


/**
 * @brief IColorSpace implementation of pattern.
 */
class PatternColorSpace
    : public IColorSpace
{
    ColorSpaceHandle m_cs;
public:
    PatternColorSpace(ColorSpaceHandle cs): m_cs(cs) {
        JAG_PRECONDITION(!resources::is_pattern_color_space(cs));
    }

    ColorSpaceHandle cs_handle() const { return m_cs; }

public: // IColorSpace
    JAG_VISITABLE_COLOR_SPACE;
    void check_validity() const {}
    ColorSpaceType color_space_type() const {
        return static_cast<ColorSpaceType>(CS_PATTERN|resources::color_space_type(m_cs));
    }
    int num_components() const { JAG_PRECONDITION(!"should not get called"); return 1; }

    boost::intrusive_ptr<IColorSpace> clone() const {
        return boost::intrusive_ptr<PatternColorSpace>(new RefCountImpl<PatternColorSpace>(m_cs));
    }
};




}} // namespace jag::pdf

#endif // __PATTERNCOLORSPACE_JG2254_H__
/** EOF @file */

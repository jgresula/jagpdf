// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __COLORSPACEMANIMPLL_H_JG_2355__
#define __COLORSPACEMANIMPLL_H_JG_2355__

#include <resources/interfaces/colorspaceman.h>
#include <resources/interfaces/colorspaces.h>
#include <resources/utils/resourcetable.h>

namespace jag {
namespace resources {

/**
 * @brief Manager of color spaces.
 *
 * As of now, it is not a genuine resource manager as it
 * - does not map the same color spaces to the same handle
 *   (but that could be done using hashing in the future)
 *
 * When a color space is being register then its clone is
 * stored within the manager, so the space can be reused for
 * further registrations (e.g. slightly modified color space).
 *
 * Color handle is a combination of several parts. The lower
 * bits of the handle are used carry the color space type. The
 * rest is an internal counter.
 *
 **/
class ColorSpaceManImpl
    : public IColorSpaceMan
{
public:
    ColorSpaceManImpl();

public: //IColorSpaceMan
    ColorSpaceHandle register_palette(ColorSpaceHandle cs, Byte const* palette, UInt byte_size);

    cs_handle_pair_t color_space_load(Char const* spec);
    boost::intrusive_ptr<ICIELab> define_cielab() const;
    boost::intrusive_ptr<ICIECalGray> define_calgray() const;
    boost::intrusive_ptr<ICIECalRGB> define_calrgb() const;
    boost::intrusive_ptr<IICCBased> define_iccbased() const;
    boost::intrusive_ptr<IPalette> define_indexed() const;

    ColorSpaceHandle color_space_load(boost::intrusive_ptr<IColorSpace> cs);
    boost::intrusive_ptr<IColorSpace> const& color_space(ColorSpaceHandle cs) const;

    int num_components(ColorSpaceHandle csh) const;

private:
    ColorSpaceHandle register_icc_from_memory(Byte const* mem, size_t size, int nr_components);

private:
    ResourceTable<ColorSpaceHandle,boost::intrusive_ptr<IColorSpace> > m_cs_table;
};


}} // namespace jag::resources

#endif //__COLORSPACEMANIMPLL_H_JG_2355__























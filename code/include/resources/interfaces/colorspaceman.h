// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __COLORSPACEMAN_H_JG_2355__
#define __COLORSPACEMAN_H_JG_2355__

#include <resources/interfaces/resourcehandle.h>
#include <core/generic/noncopyable.h>
#include <boost/shared_array.hpp>
#include <boost/intrusive_ptr.hpp>

namespace jag {

class IColorSpace;
class ICIELab;
class ICIECalGray;
class ICIECalRGB;
class IICCBased;
class IPalette;

class IColorSpaceMan
    : public noncopyable
{
public:
    typedef std::pair<ColorSpaceHandle,ColorSpaceHandle> cs_handle_pair_t;
    /**
     * @brief Registers a color space defined by a string.
     *
     * Spec is a semicolon separated list of key=val pairs.
     *  - type ... calgray, calrgb, cielab, icc, [palette]
     *  - white   = x, z
     *  - black   = x, y, z
     *  - range   = amin, amax, bmin, bmax (cielab)
     *  - gamma   = val [,val, val]        (calgray [,calrgb])
     *  - matrix  = 9 vals                 (calrgb)
     *  - profile = file-path              (icc based)
     *  - name    = sRGB  .. some predefined name
     *
     * @param spec  color space definition
     * @return color first - color space handle
     *               second - inferior color space if first is indexed
     */
    virtual cs_handle_pair_t color_space_load(Char const* spec) = 0;

    virtual ColorSpaceHandle register_palette(ColorSpaceHandle cs, Byte const* palette, UInt byte_size) = 0;
    virtual boost::intrusive_ptr<ICIELab> define_cielab() const = 0;
    virtual boost::intrusive_ptr<ICIECalGray> define_calgray() const = 0;
    virtual boost::intrusive_ptr<ICIECalRGB> define_calrgb() const = 0;
    virtual boost::intrusive_ptr<IICCBased> define_iccbased() const = 0;
    virtual boost::intrusive_ptr<IPalette> define_indexed() const = 0;

    virtual ColorSpaceHandle color_space_load(boost::intrusive_ptr<IColorSpace> lab) = 0;
    virtual boost::intrusive_ptr<IColorSpace> const& color_space(ColorSpaceHandle cs) const = 0;

    virtual int num_components(ColorSpaceHandle csh) const = 0;

protected:
    ~IColorSpaceMan() {}
};

} // namespace jag

#endif //__COLORSPACEMAN_H_JG_2355__


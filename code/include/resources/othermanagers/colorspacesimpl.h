// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __COLORSPACESIMPL_JG2227_H__
#define __COLORSPACESIMPL_JG2227_H__

#include <resources/othermanagers/colorspacevisitor.h>
#include <resources/interfaces/colorspaces.h>
#include <resources/interfaces/resourcehandle.h>
#include <core/generic/assert.h>
#include <boost/array.hpp>
#include <memory>
#include <vector>

namespace jag {
namespace resources {


class PaletteImpl
    : public IPalette
{
public: // IColorSpace
    JAG_VISITABLE_COLOR_SPACE;
    void check_validity() const;
    ColorSpaceType color_space_type() const { return CS_INDEXED; }
    int num_components() const { JAG_PRECONDITION(!"should not get called"); return 1; }
    boost::intrusive_ptr<IColorSpace> clone() const;

public: // IPalette
    void set(ColorSpace cs, Byte const* palette, UInt byte_size);

public:
    PaletteImpl();
    Byte const* data() const;
    size_t length() const;
    ColorSpaceHandle color_space() const { return m_color_space; }
    void drop_data();

private:
    ColorSpaceHandle   m_color_space;
    std::vector<Byte>  m_palette;
};




/**
 * @brief Common attributes of CIE based color spaces.
 */
class CIEBase
{
public:
    typedef boost::array<Double,2> white_point_t;
    typedef white_point_t const& white_point_ref_t;
    typedef boost::array<Double,3> black_point_t;
    typedef black_point_t const& black_point_ref_t;

private:
    white_point_t m_white_xz;
    black_point_t m_black_xyz;

public:
    CIEBase();
    void check_validity() const;
    void white_point(Double xw, Double zw);
    void black_point (Double xb, Double yb, Double zb);
    white_point_ref_t white_point() const { return m_white_xz; }
    black_point_ref_t black_point() const;
    bool is_black_point_valid() const;

};



/// CalGray color space implementation
class CIECalGrayImpl
    : public ICIECalGray
{
public: // IColorSpace
    JAG_VISITABLE_COLOR_SPACE;
    void check_validity() const { m_cie_base.check_validity(); }
    ColorSpaceType color_space_type() const { return CS_CALGRAY; }
    int num_components() const { JAG_PRECONDITION(!"should not get called"); return 1; }
    boost::intrusive_ptr<IColorSpace> clone() const;

public: // ICIECalGray
    void white_point(Double xw, Double zw) { m_cie_base.white_point(xw, zw); }
    void black_point (Double xb, Double yb, Double zb) { m_cie_base.black_point(xb, yb, zb); }
    void gamma(Double val);

public:
    CIECalGrayImpl();
    CIEBase const& cie_base() const { return m_cie_base; }
    bool is_gamma_valid() const;
    Double gamma() const;


private:
    Double  m_gamma;
    CIEBase m_cie_base;
};




/// CalRGB color space implementation
class CIECalRGBImpl
    : public ICIECalRGB
{
public: // IColorSpace
    JAG_VISITABLE_COLOR_SPACE;
    void check_validity() const { m_cie_base.check_validity(); }
    ColorSpaceType color_space_type() const { return CS_CALRGB; }
    int num_components() const { JAG_PRECONDITION(!"should not get called"); return 3; }
    boost::intrusive_ptr<IColorSpace> clone() const;

public: // ICIECalRGB
    void white_point(Double xw, Double zw) { m_cie_base.white_point(xw, zw); }
    void black_point (Double xb, Double yb, Double zb) { m_cie_base.black_point(xb, yb, zb); }
    void gamma(Double gr, Double gg, Double gb);
    void matrix(
                Double xa, Double ya, Double za,
                Double xb, Double yb, Double zb,
                Double xc, Double yc, Double zc);

public:
    typedef boost::array<Double,3> gamma_t;
    typedef gamma_t const& gamma_ref_t;
    typedef boost::array<Double,9> matrix_t;
    typedef matrix_t const& matrix_ref_t;

public:
    CIECalRGBImpl();
    CIEBase const& cie_base() const { return m_cie_base; }
    bool is_gamma_valid() const;
    bool is_matrix_valid() const;
    gamma_ref_t gamma() const;
    matrix_ref_t matrix() const;

private:
    CIEBase   m_cie_base;
    gamma_t   m_gamma;
    matrix_t  m_matrix;
};





/// CIE L*a*b color space implementation
class CIELabImpl
    : public ICIELab
{
public: // IColorSpace
    JAG_VISITABLE_COLOR_SPACE;
    void check_validity() const { m_cie_base.check_validity(); }
    ColorSpaceType color_space_type() const { return CS_CIELAB; }
    int num_components() const { JAG_PRECONDITION(!"should not get called"); return 3; }
    boost::intrusive_ptr<IColorSpace> clone() const;

public: // ICIELab
    void white_point(Double xw, Double zw) { m_cie_base.white_point(xw, zw); }
    void black_point (Double xb, Double yb, Double zb) { m_cie_base.black_point(xb, yb, zb); }
    void range(Double amin, Double amax, Double bmin, Double bmax);

public:
    CIELabImpl();
    typedef boost::array<Double,4> range_t;
    typedef range_t const& range_ref_t;

    CIEBase const& cie_base() const { return m_cie_base; }
    range_ref_t range() const;
    bool is_range_valid() const;

private:
    CIEBase m_cie_base;
    range_t m_range_ab;
};



class ICCBasedImpl
    : public IICCBased
{
public: // IColorSpace
    JAG_VISITABLE_COLOR_SPACE;
    void check_validity() const;
    ColorSpaceType color_space_type() const { return CS_ICCBASED; }
    int num_components() const { return m_num_components; }
    boost::intrusive_ptr<IColorSpace> clone() const;

public: // ICCBased
    void num_components(Int val);
    void icc_profile(Char const* file_path);
    void icc_profile_stream(std::auto_ptr<ISeqStreamInput> stream);
    void alternate(ColorSpace csh);

public:
    ICCBasedImpl();
    boost::shared_ptr<ISeqStreamInput> icc_profile() const;
    ColorSpaceHandle alternate() const { return m_alternate; }

private:
    int                                      m_num_components;
    ColorSpaceHandle                         m_alternate;
    std::string                              m_file_path;
    mutable std::auto_ptr<ISeqStreamInput>   m_stream;
#ifdef _DEBUG
    mutable bool                             m_stream_retrieved;
#endif
};



}} // namespace jag::resources

#endif // __COLORSPACESIMPL_JG2227_H__
/** EOF @file */

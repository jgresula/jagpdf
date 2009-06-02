// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/othermanagers/colorspacesimpl.h>
#include <resources/resourcebox/colorspacehelpers.h>
#include <core/generic/floatpointtools.h>
#include <core/generic/containerhelpers.h>
#include <core/generic/refcountedimpl.h>
#include <core/generic/null_deleter.h>
#include <core/errlib/errlib.h>
#include <core/jstd/file_stream.h>
#include <core/jstd/fileso.h>
#include <msg_resources.h>
#include <string.h>

using namespace jag::jstd;

namespace jag {
namespace resources {


///////////////////////////////////////////////////////////////////////////
// PaletteImpl
///////////////////////////////////////////////////////////////////////////
PaletteImpl::PaletteImpl()
{}



void PaletteImpl::check_validity() const
{
    if (!is_valid(m_color_space))
        throw exception_invalid_value(msg_invalid_cs_spec()) << JAGLOC;
}



void PaletteImpl::set(ColorSpace cs, Byte const* palette, UInt byte_size)
{
    if (!byte_size)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;


    m_color_space = handle_from_id<RESOURCE_COLOR_SPACE>(cs);
    UInt cs_type = resources::color_space_type(m_color_space);
    if (cs_type == CS_INDEXED || is_pattern_color_space(m_color_space))
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_palette.resize(byte_size);
    memcpy(&m_palette[0], palette, byte_size);
}



Byte const* PaletteImpl::data() const
{
    JAG_PRECONDITION(length());
    return address_of(m_palette);
}



size_t PaletteImpl::length() const
{
    return m_palette.size();
}



/// Upon finishing execution of this method the object becomes a zombie.
void PaletteImpl::drop_data()
{
    std::vector<Byte>().swap(m_palette);
}



boost::intrusive_ptr<IColorSpace> PaletteImpl::clone() const
{
    boost::intrusive_ptr<PaletteImpl> cloned(new RefCountImpl<PaletteImpl>());
    cloned->m_color_space = m_color_space;
    cloned->m_palette     = m_palette;
    return cloned;
}



///////////////////////////////////////////////////////////////////////////
// class CIELabImpl
///////////////////////////////////////////////////////////////////////////

CIEBase::CIEBase()
{
    // white point in pdf spec (D65) - x=0.9505, y=1.0890
    set_invalid_double(m_white_xz[0]);
    set_invalid_double(m_black_xyz[0]);
}


void CIEBase::check_validity() const
{
    if (is_invalid_double(m_white_xz[0]))
        throw exception_invalid_value(msg_invalid_cs_spec()) << JAGLOC;
}


void CIEBase::white_point(Double xw, Double zw)
{
    if (xw < 1e-8 || zw < 1e-8)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_white_xz[0] = xw;
    m_white_xz[1] = zw;
}


void CIEBase::black_point (Double xb, Double yb, Double zb)
{
    if (xb<0 || yb<0 || zb<0)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_black_xyz[0] = xb;
    m_black_xyz[1] = yb;
    m_black_xyz[2] = zb;
}

CIEBase::black_point_ref_t CIEBase::black_point() const
{
    JAG_PRECONDITION(is_black_point_valid());
    return m_black_xyz;
}

bool CIEBase::is_black_point_valid() const
{
    return !is_invalid_double(m_black_xyz[0]);
}




///////////////////////////////////////////////////////////////////////////
// CIECalGrayImpl
///////////////////////////////////////////////////////////////////////////

CIECalGrayImpl::CIECalGrayImpl()
{
    set_invalid_double(m_gamma);
}



void CIECalGrayImpl::gamma(Double val)
{
    if (val < 1e-8)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_gamma = val;
}



bool CIECalGrayImpl::is_gamma_valid() const
{
    return !is_invalid_double(m_gamma);
}



Double CIECalGrayImpl::gamma() const
{
    JAG_PRECONDITION(is_gamma_valid());
    return m_gamma;
}



boost::intrusive_ptr<IColorSpace> CIECalGrayImpl::clone() const
{
    boost::intrusive_ptr<CIECalGrayImpl> cloned(new RefCountImpl<CIECalGrayImpl>());
    cloned->m_gamma    = m_gamma;
    cloned->m_cie_base = m_cie_base;
    return cloned;
}



///////////////////////////////////////////////////////////////////////////
// CIECalRGBImpl
///////////////////////////////////////////////////////////////////////////

CIECalRGBImpl::CIECalRGBImpl()
{
    set_invalid_double(m_gamma[0]);
    set_invalid_double(m_matrix[0]);
}



void CIECalRGBImpl::gamma(Double gr, Double gg, Double gb)
{
    if (gr < 1e-8 || gg < 1e-8 || gb < 1e-8)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_gamma[0] = gr;
    m_gamma[1] = gg;
    m_gamma[2] = gb;
}



void CIECalRGBImpl::matrix(
                Double xa, Double ya, Double za,
                Double xb, Double yb, Double zb,
                Double xc, Double yc, Double zc)
{
    m_matrix[0] = xa;
    m_matrix[1] = ya;
    m_matrix[2] = za;
    m_matrix[3] = xb;
    m_matrix[4] = yb;
    m_matrix[5] = zb;
    m_matrix[6] = xc;
    m_matrix[7] = yc;
    m_matrix[8] = zc;
}



CIECalRGBImpl::gamma_ref_t CIECalRGBImpl::gamma() const
{
    JAG_ASSERT(is_gamma_valid());
    return m_gamma;
}



CIECalRGBImpl::matrix_ref_t CIECalRGBImpl::matrix() const
{
    JAG_ASSERT(is_matrix_valid());
    return m_matrix;
}



bool CIECalRGBImpl::is_gamma_valid() const
{
    return !is_invalid_double(m_gamma[0]);
}



bool CIECalRGBImpl::is_matrix_valid() const
{
    return !is_invalid_double(m_matrix[0]);
}



boost::intrusive_ptr<IColorSpace> CIECalRGBImpl::clone() const
{
    boost::intrusive_ptr<CIECalRGBImpl> cloned(new RefCountImpl<CIECalRGBImpl>());
    cloned->m_cie_base = m_cie_base;
    cloned->m_gamma = m_gamma;
    cloned->m_matrix = m_matrix;
    return cloned;
}





///////////////////////////////////////////////////////////////////////////
// CIELabImpl
///////////////////////////////////////////////////////////////////////////

CIELabImpl::CIELabImpl()
{
    set_invalid_double(m_range_ab[0]);
}



void CIELabImpl::range(Double amin, Double amax, Double bmin, Double bmax)
{
    if (amin>amax || bmin>bmax)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_range_ab[0] = amin;
    m_range_ab[1] = amax;
    m_range_ab[2] = bmin;
    m_range_ab[3] = bmax;
}



CIELabImpl::range_ref_t CIELabImpl::range() const
{
    JAG_PRECONDITION(is_range_valid());
    return m_range_ab;
}



bool CIELabImpl::is_range_valid() const
{
    return !is_invalid_double(m_range_ab[0]);
}



boost::intrusive_ptr<IColorSpace> CIELabImpl::clone() const
{
    boost::intrusive_ptr<CIELabImpl> cloned(new RefCountImpl<CIELabImpl>());
    cloned->m_cie_base = m_cie_base;
    cloned->m_range_ab = m_range_ab;
    return cloned;
}




//////////////////////////////////////////////
// ICCBasedImpl
//////////////////////////////////////////////

ICCBasedImpl::ICCBasedImpl()
    : m_num_components(-1)
#ifdef _DEBUG
    , m_stream_retrieved(false)
#endif
{
}



void ICCBasedImpl::check_validity() const
{
    if (m_num_components==-1
        || (!is_file(m_file_path.c_str()) && !m_stream.get()))
    {
        throw exception_invalid_value(msg_invalid_cs_spec()) << JAGLOC;
    }

    if (is_valid(m_alternate))
    {
        // ?number of components of the alternate cs does not match
        // we do not have color space manager pointer here so we can only
        // verify color spaces that has fixed number of components
        int nc = resources::num_components(m_alternate, 0, true);
        if (nc && nc != m_num_components)
            throw exception_invalid_value(msg_invalid_cs_spec()) << JAGLOC;
    }
}


void ICCBasedImpl::num_components(Int val)
{
    if (val<=0 || val>4 || val==2)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_num_components = val;
}



void ICCBasedImpl::icc_profile(Char const* file_path)
{
    if (!is_file(file_path))
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_stream.reset();
    m_file_path.assign(file_path);
}



void ICCBasedImpl::alternate(ColorSpace cs)
{
    ColorSpaceHandle csh(handle_from_id<RESOURCE_COLOR_SPACE>(cs));
    if (!is_valid(csh) || is_pattern_color_space(csh))
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_alternate = csh;
}



boost::intrusive_ptr<IColorSpace> ICCBasedImpl::clone() const
{
    boost::intrusive_ptr<ICCBasedImpl> cloned(new RefCountImpl<ICCBasedImpl>());
    cloned->m_num_components = m_num_components;
    cloned->m_alternate = m_alternate;
    cloned->m_file_path = m_file_path;
    cloned->m_stream = m_stream;
    return cloned;
}



/**
 * @brief Provides stream containing the profile.
 *
 * @todo return a stream proxy instead of the member
 * @return icc profile stream
 */
boost::shared_ptr<ISeqStreamInput> ICCBasedImpl::icc_profile() const
{
    if (m_stream.get())
    {
#ifdef _DEBUG
        JAG_ASSERT(!m_stream_retrieved && "stream retrieved more than once");
        m_stream_retrieved = 1;
#endif
        return boost::shared_ptr<ISeqStreamInput>(m_stream.get(), null_deleter);
    }

    return boost::shared_ptr<ISeqStreamInput>(
        new FileStreamInput(m_file_path.c_str()));
}



void ICCBasedImpl::icc_profile_stream(std::auto_ptr<ISeqStreamInput> stream)
{
    m_stream = stream;
    m_file_path.clear();
}

}} // namespace jag::resources

/** EOF @file */

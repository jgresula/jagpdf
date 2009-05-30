// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/imageman/imagespecimpl.h>
#include <resources/imageman/imagemanimpl.h>
#include "imagemanip.h"
#include <msg_resources.h>
#include <core/errlib/errlib.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/file_stream.h>
#include <core/jstd/streamhelpers.h>
#include <core/generic/floatpointtools.h>
#include <resources/resourcebox/colorspacehelpers.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>
#include <core/generic/checked_cast.h>

using namespace boost;
using namespace jag::resources;

namespace jag {
namespace resources {

//////////////////////////////////////////////////////////////////////////
ImageSpecImpl::ImageSpecImpl()
    : m_type(IMAGE_FORMAT_AUTO)
    , m_width(0)
    , m_height(0)
    , m_bpc(0)
    , m_dpi_x(0)
    , m_dpi_y(0)
    , m_gamma(0)
    , m_rendering_intent(RI_UNDEFINED)
    , m_interpolate(INTERPOLATE_UNDEFINED)
{
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::check_consistency(IExecContext const& exec_ctx)
{
    if (m_file_name.empty() && !m_image_data)
        throw exception_invalid_value(msg_image_data_not_specified()) << JAGLOC;

    // image format autodetection
    if (m_type == IMAGE_FORMAT_AUTO)
    {
        m_type = recognize_image_format(*data_stream());
        if (m_type == IMAGE_FORMAT_UNRECOGNIZED)
            throw exception_invalid_value(msg_unknown_image_type()) << JAGLOC;
    }


    if (m_type == IMAGE_FORMAT_NATIVE)
    {
        ColorSpaceType cs_type(color_space_type(m_color_space));
        if (cs_type==CS_PATTERN)
            throw exception_invalid_value(msg_pattern_cs_used_for_image()) << JAGLOC;

        if (!m_width || !m_height)
            throw exception_invalid_value(msg_image_dimension_not_defined()) << JAGLOC;

        if (!is_valid(m_color_space) )
            throw exception_invalid_value(msg_no_color_space_spec()) << JAGLOC;

        if (cs_type==CS_INDEXED)
        {
            m_bpc = 8; //?
        }
        else if (!m_bpc)
            throw exception_invalid_value(msg_image_bpc_not_defined()) << JAGLOC;

        if (equal_to_zero(m_dpi_x) || equal_to_zero(m_dpi_y))
        {
            double default_dpi = exec_ctx.config().get_int("images.default_dpi");
             if (equal_to_zero(m_dpi_x))
                m_dpi_x = default_dpi;

            if (equal_to_zero(m_dpi_y))
                m_dpi_y = default_dpi;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::format(ImageFormat type)
{
    m_type = type;
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::dimensions(UInt width, UInt height)
{
    m_width = width;
    m_height = height;
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::color_space(ColorSpace cs)
{
    m_color_space.reset(handle_from_id<RESOURCE_COLOR_SPACE>(cs));
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::bits_per_component(UInt bpc)
{
    if (bpc != 1 && bpc != 2 && bpc != 4 && bpc != 8 && bpc != 16)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_bpc = bpc;
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::dpi(Double x, Double y)
{
    m_dpi_x = x;
    m_dpi_y = y;
}



void ImageSpecImpl::gamma(Double val)
{
    if (val<1e-8)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    m_gamma = val;
}



Double ImageSpecImpl::gamma() const
{
    JAG_PRECONDITION(has_gamma());
    return m_gamma;
}



Int ImageSpecImpl::has_gamma() const
{
    return equal_to_zero(m_gamma) ? 0 : 1;
}




//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::rendering_intent(RenderingIntentType intent)
{
    m_rendering_intent = intent;
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::interpolate(Int val)
{
    m_interpolate = val ? INTERPOLATE_YES : INTERPOLATE_NO ;
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::decode(Double const* decode, UInt length)
{
    m_decode.reset(decode, length);
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::color_key_mask(UInt const* mask, UInt length)
{
    m_color_key_mask.reset(ColorKeyMaskArray(mask, length));
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::image_mask(ImageMaskID image_mask)
{
    m_mask = handle_from_id<RESOURCE_IMAGE_MASK>(image_mask);
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::alternate_for_printing(IImage* image)
{
    m_alternate = checked_static_cast<IImageData*>(image)->handle();
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::file_name(Char const* file_name)
{
    m_file_name = file_name;
    m_image_data.reset();
}

//////////////////////////////////////////////////////////////////////////
void ImageSpecImpl::data(Byte const* data, UInt length)
{
    if (!m_image_data)
        m_image_data.reset(new jstd::MemoryStreamOutput);

    m_image_data->write(data, length);
    std::string().swap(m_file_name);
}

//////////////////////////////////////////////////////////////////////////
ImageMaskType ImageSpecImpl::image_mask_type() const
{
    if (is_valid(m_mask))
        return IMT_IMAGE;

    if (is_valid(m_color_key_mask))
        return IMT_COLOR_KEY;

    return IMT_NONE;
}




//////////////////////////////////////////////////////////////////////////
bool ImageSpecImpl::output_image(ISeqStreamOutput& dest, unsigned max_bits) const
{
    shared_ptr<IStreamInput> in_stream(data_stream());

    ColorSpaceType cs_type(color_space_type(m_color_space));
    if(cs_type!=CS_INDEXED && m_bpc>max_bits)
    {
        JAG_PRECONDITION(m_bpc==16 && max_bits==8);
        downsample_big16_to_8(*in_stream, dest);
    }
    else
    {
        jstd::copy_stream(*in_stream, dest);
    }
    m_image_data.reset();

    return true;
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<IStreamInput> ImageSpecImpl::data_stream() const
{
    if (!m_file_name.empty())
    {
        return shared_ptr<IStreamInput>(
            new jstd::FileStreamInput(m_file_name.c_str())
           );
    }
    else
    {
        return shared_ptr<IStreamInput>(
            new jstd::MemoryStreamInputFromOutput(m_image_data)
           );
    }
}

}} // namespace jag::resources

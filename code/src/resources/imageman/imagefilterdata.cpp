// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "imagefilterdata.h"
#include <core/generic/floatpointtools.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>
#include <resources/imageman/imagemanimpl.h>
#include <resources/interfaces/imagemaskdata.h>
#include <resources/interfaces/resourcectx.h>
#include <core/generic/assert.h>

using namespace boost;

namespace jag {
namespace resources {

//////////////////////////////////////////////////////////////////////////
ImageFilterData::ImageFilterData(
    IImageData const* img_data
    , shared_ptr<IResourceCtx> res_ctx
    , IExecContext const& exec_ctx
)
    : m_img_filter(create_image_filter(img_data, res_ctx, exec_ctx).release())
    , m_width(m_img_filter->width())
    , m_height(m_img_filter->height())
    , m_color_space(is_valid(img_data->color_space()) ? img_data->color_space() : m_img_filter->color_space())
    , m_bits_per_component(m_img_filter->bits_per_component())
    , m_img_type(img_data->format())
    , m_alternate(img_data->alternate_for_printing())
    , m_rendering_intent(img_data->rendering_intent())
    , m_decode(m_img_filter->decode() ? *m_img_filter->decode() : img_data->decode())
    , m_interpolate(img_data->interpolate())
    , m_gamma(img_data->has_gamma() ? img_data->gamma() : m_img_filter->gamma())
    , m_res_ctx(res_ctx)
{
    // handle dpi
    double default_dpi = exec_ctx.config().get_int("images.default_dpi");

    m_dpi_x = equal_to_zero(img_data->dpi_x()) ? m_img_filter->dpi_x() : img_data->dpi_x();
    m_dpi_y = equal_to_zero(img_data->dpi_y()) ? m_img_filter->dpi_y() : img_data->dpi_y();
    if (equal_to_zero(m_dpi_x))
         m_dpi_x = default_dpi;

    if (equal_to_zero(m_dpi_y))
         m_dpi_y = default_dpi;


    // handle masks
    if (!apply_explicit_mask(img_data))
        apply_intrinsic_mask();
}



/// applies image intrinsic mask
void ImageFilterData::apply_intrinsic_mask()
{
    switch (m_img_filter->image_mask_type())
    {
    case IMT_COLOR_KEY:
        m_color_key_mask.reset(m_img_filter->color_key_mask());
        break;

    case IMT_IMAGE:
    {
        intrusive_ptr<IImageMaskData> image_mask (m_img_filter->create_mask());
        m_image_mask_handle.reset(
            resource_ctx()->image_man()->register_image_mask(image_mask));
    }

    default:
        ;
    }
}


/// returns true if an explicit mask has been applied
bool ImageFilterData::apply_explicit_mask(IImageData const* img_data)
{
    bool explicit_mask = true;
    switch (img_data->image_mask_type())
    {
    case IMT_COLOR_KEY:
        m_color_key_mask.reset(img_data->color_key_mask());
        break;

    case IMT_IMAGE:
        m_image_mask_handle.reset(img_data->image_mask());
        break;

    case IMT_NONE:
        explicit_mask = false;
        break;
    }

    return explicit_mask;
}

ImageMaskType ImageFilterData::image_mask_type() const
{
    if (is_valid(m_color_key_mask))
        return IMT_COLOR_KEY;

    if (is_valid(m_image_mask_handle))
        return IMT_IMAGE;

    return IMT_NONE;


}


//////////////////////////////////////////////////////////////////////////
bool ImageFilterData::output_image(ISeqStreamOutput& out, unsigned max_bits) const
{
    JAG_ASSERT_MSG(m_img_filter, "image_bits() already invoked");
    bool result = m_img_filter->output_image(out, max_bits);
    m_img_filter.reset();
    return result;
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<IStreamInput> ImageFilterData::data_stream() const
{
    JAG_ASSERT_MSG(m_img_filter, "image_bits() already invoked");
    return m_img_filter->data_stream();
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<IResourceCtx> ImageFilterData::resource_ctx()
{
    return shared_ptr<IResourceCtx>(m_res_ctx);
}



Double ImageFilterData::gamma() const
{
    JAG_ASSERT(has_gamma());
    return m_gamma;
}



Int ImageFilterData::has_gamma() const
{
    return m_gamma<1e-8 ? false : true;
}



}} //namespace jag::resources

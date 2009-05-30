// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEFILTERDATA_H_JAG_1008__
#define __IMAGEFILTERDATA_H_JAG_1008__

#include "interfaces/imagefilter.h"
#include <resources/resourcebox/resourcepodarrays.h>
#include <resources/interfaces/imagedata.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace jag {
class IResourceCtx;
class IExecContext;

namespace resources
{

/**
 * @brief Implements IImageData for standard image formats.
 *
 * <b>Image Mask</b>. If the filter itself provides some mask then
 * it is used. Otherwise the mask from IImageData is used (if any).
 * That allows transparency for standard image formats even if
 * it is not specified in the actual image data.
 */
class ImageFilterData
    : public IImageData
{
public: //IImageData
    UInt width() const { return m_width; }
    UInt height() const { return m_height; }
    ColorSpaceHandle color_space() const { return m_color_space; }
    UInt bits_per_component() const { return m_bits_per_component; }
    Double dpi_x() const { return m_dpi_x; }
    Double dpi_y() const { return m_dpi_y; }

    ImageMaskType image_mask_type() const;
    ColorKeyMaskArray const& color_key_mask() const { return m_color_key_mask; }
    ImageMaskHandle image_mask() const { return m_image_mask_handle; }
    bool output_image(ISeqStreamOutput& fout, unsigned max_bits) const;
    ImageFormat format() const { return m_img_type; }
    boost::shared_ptr<IStreamInput> data_stream() const;

    ImageHandle alternate_for_printing() const { return m_alternate; }
    RenderingIntentType rendering_intent() const { return m_rendering_intent; }
    DecodeArray const& decode() const { return m_decode; }
    InterpolateType interpolate() const { return m_interpolate; }

    Double gamma() const;
    Int has_gamma() const;
    ImageHandle handle() const { JAG_ASSERT(is_valid(m_handle)); return m_handle; }
    void handle(ImageHandle handle) { m_handle=handle; }

public:
    ImageFilterData(
        IImageData const* img_spec
        , boost::shared_ptr<IResourceCtx> res_ctx
        , IExecContext const& exec_ctx
   );

private:
    bool apply_explicit_mask(IImageData const* img_spec);
    void apply_intrinsic_mask();
    boost::shared_ptr<IResourceCtx> resource_ctx();

private:
    mutable boost::scoped_ptr<IImageFilter> m_img_filter;
    UInt              m_width;
    UInt              m_height;
    ColorSpaceHandle    m_color_space;
    UInt              m_bits_per_component;
    Double              m_dpi_x;
    Double              m_dpi_y;
    ColorKeyMaskArray    m_color_key_mask;
    ImageFormat         m_img_type;

    ImageHandle         m_alternate;
    RenderingIntentType m_rendering_intent;
    DecodeArray            m_decode;
    InterpolateType        m_interpolate;
    Double              m_gamma;
    ImageHandle         m_handle;

private:
    boost::weak_ptr<IResourceCtx>  m_res_ctx;
    ImageMaskHandle                m_image_mask_handle;
};


}} //namespace jag::resources

#endif //__IMAGEFILTERDATA_H_JAG_1008__






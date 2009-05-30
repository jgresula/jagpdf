// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGESPECIMPL_H_JG_1117_
#define __IMAGESPECIMPL_H_JG_1117_

#include <resources/interfaces/imagedata.h>
#include <resources/interfaces/resourceconstants.h>
#include <resources/resourcebox/resourcepodarrays.h>
#include <resources/interfaces/resourcehandle.h>
#include <resources/interfaces/imagespec.h>
#include <boost/shared_ptr.hpp>

#include <boost/intrusive_ptr.hpp>
#include <boost/intrusive/list.hpp>
#include <vector>
#include <string>

namespace jag {
// fwd
class IStreamInput;
class IExecContext;
namespace jstd { class MemoryStreamOutput; }

namespace resources
{


class ImageSpecImpl
    : public IImageDef
    , public IImageData
      // hook for an intrusive list in image manager
    , public boost::intrusive::list_base_hook<>
{
public: //IImageDef
    void format(ImageFormat type);
    void dimensions(UInt width, UInt height);
    void color_space(ColorSpace cs);
    void bits_per_component(UInt bpc);
    void dpi(Double x, Double y);
    void rendering_intent(RenderingIntentType intent);
    void interpolate(Int val);
    void decode(Double const* decode, UInt length);
    void color_key_mask(UInt const* mask, UInt length);
    void image_mask(ImageMaskID image_mask);
    void file_name(Char const* file_name);
    void data(Byte const* data, UInt length);
    void alternate_for_printing(IImage* image);
    void gamma(Double val);

public: //IImageData
    UInt width() const { return m_width; }
    UInt height() const { return m_height; }
    ColorSpaceHandle color_space() const { return m_color_space; }
    UInt bits_per_component() const { return m_bpc; }
    Double dpi_x() const { return m_dpi_x; }
    Double dpi_y() const { return m_dpi_y; }
    ImageMaskType image_mask_type() const;
    ColorKeyMaskArray const& color_key_mask() const { return m_color_key_mask; }
    ImageMaskHandle image_mask() const { return m_mask; }
    bool output_image(ISeqStreamOutput& fout, unsigned max_bits) const;
    ImageFormat format() const { return m_type; }
    Double gamma() const;
    Int has_gamma() const;

    ImageHandle alternate_for_printing() const { return m_alternate; }
    RenderingIntentType rendering_intent() const { return m_rendering_intent; }
    DecodeArray const& decode() const { return m_decode; }
    InterpolateType interpolate() const { return m_interpolate; }
    ImageHandle handle() const { JAG_ASSERT(is_valid(m_handle)); return m_handle; }
    void handle(ImageHandle handle) { m_handle=handle; }

public:
    ImageSpecImpl();
    void check_consistency(IExecContext const& exec_ctx);

private:
    boost::shared_ptr<IStreamInput> data_stream() const;

private: // Impl
    ImageFormat     m_type;
    UInt         m_width;
    UInt         m_height;
    ColorSpaceHandle m_color_space;
    UInt         m_bpc;
    Double         m_dpi_x;
    Double         m_dpi_y;
    Double       m_gamma;
    RenderingIntentType    m_rendering_intent;

    DecodeArray    m_decode;

    InterpolateType                  m_interpolate;
    ImageHandle                      m_alternate;

    // transparency variant
    ColorKeyMaskArray        m_color_key_mask;
    ImageMaskHandle            m_mask;
    ImageHandle             m_handle;

    // data source variant
    mutable boost::shared_ptr<jstd::MemoryStreamOutput>    m_image_data;
    std::string                                                m_file_name;
};


}} // namespace jag::resources


#endif //__IMAGESPECIMPL_H_JG_1117_

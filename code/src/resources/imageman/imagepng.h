// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEPNG_H_JG1028__
#define __IMAGEPNG_H_JG1028__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "interfaces/imagefilter.h"
#include <resources/resourcebox/resourcepodarrays.h>
#include <core/errlib/errlib.h>
#include <boost/weak_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <png.h>

// ICC
// GAMMA


namespace jag
{
class IStreamInput;
class ISeqStreamOutput;
class IResourceCtx;
class IImageData;
class IExecContext;

// PNG image type       Colour type     Allowed bit depths     Interpretation
// Greyscale               0         1, 2, 4, 8, 16       Each pixel is a greyscale sample
// Truecolour             2         8, 16               Each pixel is an R,G,B triple
// Indexed-colour           3         1, 2, 4, 8           Each pixel is a palette index; a PLTE chunk shall appear.
// Greyscale with alpha   4         8, 16               Each pixel is a greyscale sample followed by an alpha sample.
// Truecolour with alpha  6         8, 16               Each pixel is an R,G,B triple followed by an alpha sample.

namespace resources
{
/**
 * @brief IImageFilter implementation of PNG format
 *
 * In constructor, only image properties are read.
 *
 * The image data are read when output_image_data() is invoked and are released
 * immediately after this call has finished. If the image has a soft mask then
 * the image data are released after the soft mask data has been outputted.
 *
 * Image data can be read only once (that can be subject to change). Image
 * properties are available during whole lifetime of this object.
 *
 * On request 16bit channels are converted to 8bit channels.
 *
 *
 * Color Management.
 *  - if iCCP chunk present then use it
 *  - if sRGB chunk present then use it
 *  - if cHRM -> convert to CalRGB
 *     (conversion described at the end of 4.5.4/CalRGB Color Spaces chapter)
 *  - if gAMA and cHRM -> convert to CalRGB + gamma
 *  - if gAMA alone - transfer function
 *  - if icc not supported by client (pdf < 1.3) - not implemented
 *    - ignore iCCP chunk this step
 *    - sRGB -> convert to CalRGB (values given in png specs, 11.3.3.5)
 */
class ImagePNG
    : public IImageFilter
{
public:
    ImagePNG(IImageData const* img_spec,
             boost::shared_ptr<IResourceCtx> res_ctx,
             IExecContext const& exec_ctx);
    ~ImagePNG();
    static bool ping(IStreamInput& in_stream);

public: // IImageFilter
    UInt width() const { return m_width; }
    UInt height() const { return m_height; }
    ColorSpaceHandle color_space() const { return m_color_space; }
    UInt bits_per_component() const { return m_bits_per_component; }
    Double dpi_x() const { return m_dpi_x; }
    Double dpi_y() const { return m_dpi_y; }
    DecodeArray const* decode() const { return 0; }
    Double gamma() const { return m_gamma < 1e-8 ? 0.0 : m_gamma; }

    ImageMaskType image_mask_type() const;
    ColorKeyMaskArray const& color_key_mask() const { return m_color_key_mask; }

    boost::intrusive_ptr<IImageMaskData> create_mask() const;

    bool output_image(ISeqStreamOutput& dest, unsigned max_bits) const;
    boost::shared_ptr<IStreamInput> data_stream() const { JAG_ASSERT(!"suspicious"); return m_in_stream; }

public:
    struct PNGData;
    int png_color_type() const { return m_png_color_type; }
    boost::shared_ptr<PNGData> const& get_image_data() const;


private:
    void release_lib_data();

    void read_image_info();
    void read_data() const;

    // invoked from read_image_info()
    void read_resolution();
    void read_color_key_mask(png_uint_32 valid);
    void read_color_space();
    void create_soft_mask();
    void read_interlacing();

    void read_color_space_srgb();
    void read_color_space_iccp();
    void read_color_space_chrm();

    void rgb_from_rgba(ISeqStreamOutput& dest, png_byte const* data, bool convert16to8) const;
    void report_error() const;


private: // lpng related
    static void user_error_handler(png_structp png_struct, png_const_charp msg);
    static void user_warning_handler(png_structp png_struct, png_const_charp msg);
    static void user_read_data(png_structp png_struct, png_bytep data, png_size_t length);

private:
    static const int    nr_ping_bytes = 8;
    boost::shared_ptr<IResourceCtx> resource_ctx();

    // set in constructor
    png_structp            m_png;
    png_infop            m_png_info;


    // set by read_image_info()
    png_uint_32            m_dpi_x;
    png_uint_32            m_dpi_y;
    UInt            m_width;
    UInt            m_height;
    ColorSpaceHandle        m_color_space;
    int                m_png_color_type;
    UInt            m_bits_per_component;
    int                m_number_of_passes;
    ColorKeyMaskArray    m_color_key_mask;
    bool            m_has_soft_mask;
    double          m_gamma;

    // set by read_data()
    mutable boost::shared_ptr<PNGData>    m_png_data;

private:
    boost::shared_ptr<IStreamInput>    m_in_stream;
    boost::weak_ptr<IResourceCtx> m_res_ctx;
    IExecContext const&  m_exec_ctx;

    std::string                   m_error;
    mutable boost::scoped_ptr<exception>  m_exception;
};


}} //namespace jag::resources


#endif //__IMAGEPNG_H_JG1028__























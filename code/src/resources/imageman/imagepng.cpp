// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "imagepng.h"
#include "gilext.h"
#include <resources/interfaces/imagedata.h>
#include <resources/interfaces/imagemaskdata.h>
#include <resources/interfaces/resourcectx.h>
#include <resources/interfaces/colorspaceman.h>
#include <resources/interfaces/colorspaces.h>
#include <resources/resourcebox/binresources.h>
#include <msg_resources.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/streamhelpers.h>
#include <core/jstd/tracer.h>
#include <core/errlib/msg_writer.h>
#include <core/errlib/msgcodes.h>
#include <core/errlib/errlib.h>
#include <core/generic/refcountedimpl.h>
#include <core/generic/floatpointtools.h>
#include <interfaces/streams.h>
#include <interfaces/execcontext.h>
#include <interfaces/configinternal.h>

#include <boost/scoped_array.hpp>
#include <utility>

using namespace jag::jstd;
using namespace boost;
using namespace boost::integer;

namespace jag {
namespace resources {

namespace
{
  const Double g_matte_1[4] = { 0.0, 0.0, 0.0, 0.0 };
}


struct ImagePNG::PNGData
{
    UInt                    m_rowbytes;
    scoped_array<png_byte>    m_image_data;
    scoped_array<png_bytep>    m_rows;
};


//////////////////////////////////////////////////////////////////////////
// SoftMaskPNG
//////////////////////////////////////////////////////////////////////////

/// IImageSoftMask implementation for PNG format
class SoftMaskPNG
    : public IImageMaskData
{
    mutable shared_ptr<ImagePNG::PNGData>    m_png_data;
    const UInt                            m_width;
    const UInt                            m_height;
    const UInt                            m_bits_per_component;
    const UInt                             m_image_byte_size;
    const int                                m_png_color_type;
    DecodeArray                                m_decode_array;
    ColorComponents                            m_matte;

public: // IImageMaskData
    UInt width() const { return m_width; }
    UInt height() const { return m_height; }
    UInt bits_per_component() const { return m_bits_per_component; }
    ColorComponents const& matte() const { return m_matte; }
    bool output_mask(ISeqStreamOutput& fout, unsigned max_bits) const;
    DecodeArray const& decode() const { return m_decode_array; };
    InterpolateType interpolate() const { return INTERPOLATE_UNDEFINED; }

private:
    template<class Pix16, class Pix8>
    void output_alpha(ISeqStreamOutput& dest, void const* data, int channel, bool convert16to8) const;

public:
    SoftMaskPNG(ImagePNG const& image_png)
        : m_png_data(image_png.get_image_data())
        , m_width(image_png.width())
        , m_height(image_png.height())
        , m_bits_per_component(image_png.bits_per_component())
        , m_image_byte_size(image_png.height()*m_png_data->m_rowbytes)
        , m_png_color_type(image_png.png_color_type())
    {
        // provide matte to remedy problems with 16bit depth
//         m_matte.reset(
//             g_matte_1,
//             m_png_color_type==PNG_COLOR_TYPE_RGB_ALPHA ? 3 : 1);
    }
};


//////////////////////////////////////////////////////////////////////////
bool SoftMaskPNG::output_mask(ISeqStreamOutput& dest, unsigned max_bits) const
{
    JAG_PRECONDITION(
        m_png_color_type == PNG_COLOR_TYPE_RGB_ALPHA
        || m_png_color_type == PNG_COLOR_TYPE_GRAY_ALPHA
   );

    png_byte const*const data = m_png_data->m_image_data.get();
    bool convert16to8 = max_bits==8 && m_bits_per_component>8;

    if (m_png_color_type==PNG_COLOR_TYPE_RGB_ALPHA)
    {
        output_alpha<rgba_big16_pixel_t,gil::rgba8_pixel_t>(dest, data, 3, convert16to8);
    }
    else
    {
        JAG_ASSERT(m_png_color_type==PNG_COLOR_TYPE_GRAY_ALPHA);
        output_alpha<graya_big16_pixel_t,gil::dev2n8_pixel_t>(dest, data, 1, convert16to8);
    }

    m_png_data.reset();
    return true;
}


template<class Pix16, class Pix8>
void SoftMaskPNG::output_alpha(ISeqStreamOutput& dest, void const* data, int channel, bool convert16to8) const
{
    output_channel<Pix16,Pix8>::apply(
        dest
        , img_data_dict_t(
              m_width
            , m_height
            , data
            , m_png_data->m_rowbytes
            , m_bits_per_component)
        , channel
        , convert16to8);
}




//////////////////////////////////////////////////////////////////////////
//  ImagePNG
//////////////////////////////////////////////////////////////////////////


// libpng error handling
// ~~~~~~~~~~~~~~~~~~~~~
//
// An exception cannot be thrown from the user supplied error function as it is
// invoked from a C code. It would cause that the stack unwinding fails which
// results into process termination on some platforms.
//
// Unfortunatelly, the only way how to reasonably/portably handle errors
// reported by libpng is setjmp/longjmp. Using setjmp/longjmp in C++ is
// dangerous as presence of an auto object with destructor on the stack between
// setjmp and longjmp calls results into undefined behavior.
//
// So the rules are:
// - setjmp() must be called whenever a client invokes a public method calling
//   into libpng
// - no auto variable with destructor is allowed before a call to libpng in the
//   same or a superior code block
//
// There is no way how to enforce these rules so check them manually by
// traversing and verifying all pnglib calls. In Emacs, it is: C+A S and type
// '[^a-z_]png_.*('

#ifdef _MSC_VER
#pragma warning(disable:4611) //interaction between '_setjmp' and C++ object destruction is non-portable
#endif /* VC++ */



//
//
//
ImagePNG::ImagePNG(IImageData const* img_data,
                   shared_ptr<IResourceCtx> res_ctx,
                   IExecContext const& exec_ctx)
    : m_png(0)
    , m_png_info(0)
    , m_number_of_passes(1)
    , m_has_soft_mask(false)
    , m_gamma(-1.0)
    , m_png_data(new PNGData)
    , m_in_stream(img_data->data_stream())
    , m_res_ctx(res_ctx)
    , m_exec_ctx(exec_ctx)
{
    {
        JAG_PRECONDITION(img_data->format() == IMAGE_FORMAT_PNG);

        if (!ImagePNG::ping(*m_in_stream))
            throw exception_invalid_input(msg_not_png_format()) << JAGLOC;

        m_png = png_create_read_struct(PNG_LIBPNG_VER_STRING
                                        , this
                                        , &ImagePNG::user_error_handler
                                        , &ImagePNG::user_warning_handler);
        if (!m_png) {
            JAG_INTERNAL_ERROR_MSG("PNGlib initialization failed");
        }
    }

    if (setjmp(m_png->jmpbuf)) {
        release_lib_data();
        report_error();
    }

    m_png_info = png_create_info_struct(m_png);
    if (!m_png_info) {
        JAG_INTERNAL_ERROR_MSG("PNGlib initialization failed");
    }

    png_set_sig_bytes(m_png, ImagePNG::nr_ping_bytes);
    png_set_read_fn(m_png, m_in_stream.get(), &ImagePNG::user_read_data);
    png_read_info(m_png, m_png_info);

    read_image_info();
}

//////////////////////////////////////////////////////////////////////////
void ImagePNG::read_image_info()
{
    JAG_PRECONDITION(m_png && m_png_info);

    png_uint_32 valid = png_get_valid(m_png, m_png_info, ~0U);

    m_width = png_get_image_width(m_png, m_png_info);
    m_height = png_get_image_height(m_png, m_png_info);
    m_bits_per_component = png_get_bit_depth(m_png, m_png_info);

    read_resolution();
    read_interlacing();
    read_color_space();
    read_color_key_mask(valid);
}



/**
 * @brief detects interlacing
 *
 * fills in m_number_of_passes
 */
void ImagePNG::read_interlacing()
{
    png_byte interlace_type = png_get_interlace_type(m_png, m_png_info);
    if (interlace_type != 0)
    {
        if (interlace_type == PNG_INTERLACE_ADAM7)
        {
            m_number_of_passes = png_set_interlace_handling(m_png);
        }
        else
        {
            throw exception_invalid_input(msg_png_interlacing_other_than_adam7_not_supported()) << JAGLOC;
        }
    }
}



void ImagePNG::read_color_space_srgb()
{
    int srgb_intent = 0;
    if (png_get_sRGB(m_png, m_png_info, &srgb_intent))
    {
        boost::shared_ptr<IResourceCtx> res_ctx(m_res_ctx);
        intrusive_ptr<IICCBased> spec(
            res_ctx->color_space_man()->define_iccbased());

        spec->num_components(3);
        std::auto_ptr<ISeqStreamInput> mem_stream(
            create_stream_from_memory(binres::icc_srgb, binres::icc_srgb_size));

        spec->icc_profile_stream(mem_stream);
        m_color_space = res_ctx->color_space_man()->color_space_load(spec);
    }
}



void ImagePNG::read_color_space_iccp()
{
    // iCCP
    png_charp name(0);
    int compression_type(1);
    png_charp profile(0);
    png_uint_32 proflen(0);
    if (png_get_iCCP(m_png, m_png_info, &name, &compression_type, &profile, &proflen ))
    {
        if (!compression_type)
        {
            boost::shared_ptr<IResourceCtx> res_ctx(m_res_ctx);
            intrusive_ptr<IICCBased> spec(
                res_ctx->color_space_man()->define_iccbased());

            switch(m_png_color_type)
            {
            case PNG_COLOR_TYPE_PALETTE:
            case PNG_COLOR_TYPE_RGB_ALPHA:
            case PNG_COLOR_TYPE_RGB:
                spec->num_components(3);
                break;

            case PNG_COLOR_TYPE_GRAY_ALPHA:
            case PNG_COLOR_TYPE_GRAY:
                spec->num_components(1);
                break;

            default:
                throw exception_invalid_input(msg_error_unsupported_color_type_in_png(m_png_color_type)) << JAGLOC;
            }

            std::auto_ptr<ISeqStreamInput> profile_stream(
                new MemoryStreamInput(profile, proflen));

            spec->icc_profile_stream(profile_stream);
            m_color_space = res_ctx->color_space_man()->color_space_load(spec);
        }
    }
}



/**
 * @brief Processes cHRM chunk.
 *
 *
 */
void ImagePNG::read_color_space_chrm()
{
    // gAMA
    if (!png_get_gAMA(m_png, m_png_info, &m_gamma))
        m_gamma = -1.0;

    // cHRM
    double xw=0.0, yw=0.0;
    double xr=0.0, yr=0.0;
    double xg=0.0, yg=0.0;
    double xb=0.0, yb=0.0;
    if (png_get_cHRM(m_png, m_png_info, &xw, &yw, &xr,
                       &yr, &xg, &yg, &xb, &yb))
    {
        double z = yw*((xg-xb)*yr - (xr-xb)*yg + (xr-xg)*yb);
        double Ya = yr * ((xg-xb)*yw - (xw-xb)*yg + (xw-xg)*yb)/z;
        double Xa = Ya*xr/yr;
        double Za = Ya*(((1-xr)/yr)-1);

        double Yb = -yg * ((xr-xb)*yw - (xw-xb)*yr + (xw-xr)*yb)/z;
        double Xb = Yb*xg/yg;
        double Zb = Yb*(((1-xg)/yg)-1);

        double Yc =  yb * ((xr-xg)*yw - (xw-xg)*yr + (xw-xr)*yg)/z;
        double Xc = Yc*xb/yb;
        double Zc = Yc*(((1-xb)/yb)-1);

        double Xw = Xa + Xb + Xc;
        double Yw = Ya + Yb + Yc;
        double Zw = Za + Zb + Zc;

        if (equal_doubles(Yw, 1.0))
        {
            boost::shared_ptr<IResourceCtx> res_ctx(m_res_ctx);
            intrusive_ptr<ICIECalRGB> spec(
                res_ctx->color_space_man()->define_calrgb());
            spec->white_point(Xw, Zw);
            spec->matrix(Xa, Ya, Za, Xb, Yb, Zb, Xc, Yc, Zc);
            if (m_gamma > 0.0)
            {
                spec->gamma(m_gamma, m_gamma, m_gamma);
                // do not return valid value in gamma()
                // as it is encoded into the color space
                m_gamma = -1.0;
            }
            m_color_space = res_ctx->color_space_man()->color_space_load(spec);
        }
    }
}




/**
 * @brief reads color space
 *
 * fills in m_png_color_type, m_color_space, possibly m_softmask
 */
void ImagePNG::read_color_space()
{
    // read color space type from PNG here, iccp detection relies on it
    m_png_color_type= png_get_color_type(m_png, m_png_info);

    IProfileInternal const& cfg = m_exec_ctx.config();
    if (cfg.get_int("images.png_advanced_color")) {
        // gAMA, cHRM, sRGB and iCCP chunk processing is done according to spec
        // any of read_color_space_xxx() sets m_color_space if the chunk is found
        read_color_space_srgb();
        
        if (!is_valid(m_color_space))
            read_color_space_iccp();
        
        if (!is_valid(m_color_space))
            read_color_space_chrm();
    }

    switch(m_png_color_type)
    {
    case PNG_COLOR_TYPE_PALETTE:
        {
            int nr_colors;
            png_colorp png_palette;
            png_get_PLTE(m_png, m_png_info, &png_palette, &nr_colors);
            if (!nr_colors)
                throw exception_invalid_input(msg_png_empty_palette()) << JAGLOC;

            std::vector<Byte> palette;
            palette.resize(nr_colors*3);

            Byte *curr = &palette[0];
            for(int i=0; i<nr_colors; ++i)
            {
                *curr++ = png_palette[i].red;
                *curr++ = png_palette[i].green;
                *curr++ = png_palette[i].blue;
            }

            ColorSpaceHandle inferior_handle(
                is_valid(m_color_space)
                ? m_color_space
                : ColorSpaceHandle(CS_DEVICE_RGB));

            ColorSpaceHandle palette_handle
                = resource_ctx()->color_space_man()->register_palette(
                    inferior_handle, &palette[0], nr_colors*3);

            m_color_space.reset(palette_handle);
        }
        break;

    case PNG_COLOR_TYPE_RGB_ALPHA:
        m_has_soft_mask = true;

    case PNG_COLOR_TYPE_RGB:
        if (!is_valid(m_color_space))
            m_color_space.reset(ColorSpaceHandle(CS_DEVICE_RGB));
        break;

    case PNG_COLOR_TYPE_GRAY_ALPHA:
        m_has_soft_mask = true;

    case PNG_COLOR_TYPE_GRAY:
        if (!is_valid(m_color_space))
            m_color_space.reset(ColorSpaceHandle(CS_DEVICE_GRAY));
        break;


    default:
        throw exception_invalid_input(msg_error_unsupported_color_type_in_png(m_png_color_type)) << JAGLOC;
    }
}


/**
 * @brief reads color key mask
 * @param valid png info about read chunks
 *
 * fills in m_color_key_mask, m_color_key_mask_size
 */
void ImagePNG::read_color_key_mask(png_uint_32 valid)
{
    if (valid & PNG_INFO_tRNS)
    {
        png_bytep trans_entries=0;
        int num_trans=0;
        png_color_16p trans_values=0;
        png_get_tRNS(m_png, m_png_info, &trans_entries, &num_trans, &trans_values);

        if (num_trans)
        {
            UInt mask[ColorKeyMaskArray::MAX_ITEMS];
            size_t mask_len = 0;

            switch (m_png_color_type)
            {
            case PNG_COLOR_TYPE_RGB:
                mask[mask_len++] = trans_values[0].red;
                mask[mask_len++] = trans_values[0].red;
                mask[mask_len++] = trans_values[0].green;
                mask[mask_len++] = trans_values[0].green;
                mask[mask_len++] = trans_values[0].blue;
                mask[mask_len++] = trans_values[0].blue;
                m_color_key_mask.reset(mask, mask_len);
                break;

            case PNG_COLOR_TYPE_GRAY:
                mask[mask_len++] = trans_values[0].gray;
                mask[mask_len++] = trans_values[0].gray;
                m_color_key_mask.reset(mask, mask_len);
                break;

            case PNG_COLOR_TYPE_PALETTE: {
                // For each palette entry there is a value from {0,..,255}. 0
                // means fully off, 255 means fully on. If there are multiple
                // 0's or a value from {1,..,254} then a full alpha channel is
                // needed (not implemented).
                enum { INDEX_INIT = -1, NEEDS_ALPHA = -2 };
                int index = INDEX_INIT;
                for (int i=0; i<num_trans; ++i) {
                    if (trans_entries[i] == 0) { // fully off
                        if (index == INDEX_INIT) {
                            index = i;
                        }
                        else {
                            TRACE_WRN << "Not implemented: alpha channel from a color key mask.";
                            break;
                        }
                    } else if (trans_entries[i] != 255) { // not fully on
                        index = NEEDS_ALPHA;
                        break;
                    }
                }
                if (index >= 0)
                {
                    // a color key mask can be used
                    mask[mask_len++] = index;
                    mask[mask_len++] = index;
                    m_color_key_mask.reset(mask, mask_len);
                }
            }
                break;
            }
        }
    }
}

/**
 * @brief reads image resolution
 *
 * fills in m_dpi_x, m_dpi_y
 */
void ImagePNG::read_resolution()
{
    int unit_type=0;
    png_uint_32 dpi_x=0, dpi_y=0;
    png_get_pHYs(m_png, m_png_info, &dpi_x, &dpi_y, &unit_type);
    if (PNG_RESOLUTION_METER == unit_type)
    {
        m_dpi_x = static_cast<int>(dpi_x * 2.54/100 + 0.5);
        m_dpi_y = static_cast<int>(dpi_y * 2.54/100 + 0.5);
    }
    else
    {
        m_dpi_x = m_dpi_y = 0;
    }
}


//////////////////////////////////////////////////////////////////////////
void ImagePNG::release_lib_data()
{
    if (m_png)
    {
        png_destroy_read_struct(
              &m_png
            , m_png_info ? &m_png_info : 0
            , 0
       );

        // delete image data as well
        m_png_data.reset();
    }

    m_png_info = 0;
    m_png = 0;
}


//////////////////////////////////////////////////////////////////////////
ImagePNG::~ImagePNG()
{
    release_lib_data();
}

//////////////////////////////////////////////////////////////////////////
void ImagePNG::read_data() const
{
    if (m_png_data->m_image_data)
        return; //already read

    if (setjmp(m_png->jmpbuf)) {
        report_error();
    }

    png_start_read_image(m_png);

    m_png_data->m_rowbytes = png_get_rowbytes(m_png, m_png_info);

    const int image_byte_size = m_height*m_png_data->m_rowbytes;
    m_png_data->m_image_data.reset(new png_byte[image_byte_size]);
    m_png_data->m_rows.reset(new png_bytep[m_height]);

    // ------ testing
    // reference files should be generated with the following line
    //memset(m_png_data->m_image_data.get(), 0xff, image_byte_size);
    // difference ranges should be generated with the following line
    //memset(m_png_data->m_image_data.get(), 0, image_byte_size);
    // ------ testing


    for (UInt i=0; i<m_height; ++i)
        m_png_data->m_rows[i] = m_png_data->m_image_data.get() + i*m_png_data->m_rowbytes;

    for (int i=0; i<m_number_of_passes; ++i)
    {
        // "sparkle" effect"
        png_read_rows(m_png, m_png_data->m_rows.get(), NULL, m_height);
    }

    png_read_end(m_png, m_png_info);
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<ImagePNG::PNGData> const& ImagePNG::get_image_data() const
{
    read_data();
    return m_png_data;
}

//////////////////////////////////////////////////////////////////////////
ImageMaskType ImagePNG::image_mask_type() const
{
    if (m_has_soft_mask)
        return IMT_IMAGE;

    if (is_valid(m_color_key_mask))
        return IMT_COLOR_KEY;

    return IMT_NONE;
}

//////////////////////////////////////////////////////////////////////////
intrusive_ptr<IImageMaskData> ImagePNG::create_mask() const
{
    return intrusive_ptr<IImageMaskData>(
        new RefCountImpl<SoftMaskPNG>(*this));
}


//////////////////////////////////////////////////////////////////////////
bool ImagePNG::output_image(ISeqStreamOutput& dest, unsigned max_bits) const
{
    read_data();

    png_byte const*const image_data = m_png_data->m_image_data.get();
    UInt image_byte_size = m_height * m_png_data->m_rowbytes;
    bool convert16to8 = max_bits==8 && m_bits_per_component==16;

    switch (m_png_color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        if (convert16to8)
        {
            output_view(
                dest,
                gil::color_converted_view<gil::rgb8_pixel_t>(
                      gil::interleaved_view(
                          m_width
                        , m_height
                        , (rgb_big16_pixel_cptr_t)image_data
                        , m_png_data->m_rowbytes)
                    , my_color_converter())
           );
            return true;
        }

    case PNG_COLOR_TYPE_GRAY:
        if (convert16to8)
        {
            output_view(
                dest,
                gil::color_converted_view<gil::gray8_pixel_t>(
                      gil::interleaved_view(
                          m_width
                        , m_height
                        , (gray_big16_pixel_cptr_t)image_data
                        , m_png_data->m_rowbytes)
                    , my_color_converter())
           );
            return true;
        }
        break;



    case PNG_COLOR_TYPE_RGB_ALPHA:
        rgb_from_rgba(dest, image_data, convert16to8);
        return true;


    case PNG_COLOR_TYPE_GRAY_ALPHA:
        output_channel<graya_big16_pixel_t,gil::dev2n8_pixel_t>::apply(
            dest
            , img_data_dict_t(
                m_width
                , m_height
                , image_data
                , m_png_data->m_rowbytes
                , m_bits_per_component)
            , 0
            , convert16to8);
        return true;
    }

    dest.write(image_data, image_byte_size);
    return true;
}




/**
 * @brief Converts RGBA to RGB and sends it an output stream.
 *
 * @param dest  output stream
 * @param data  image raw data
 * @param convert16to8  whether to decrease bit depth from 16 to 8
 */
void ImagePNG::rgb_from_rgba(ISeqStreamOutput& dest, png_byte const* data, bool convert16to8) const
{
    if (convert16to8)
    {
        output_view(
            dest,
            gil::color_converted_view<gil::rgb8_pixel_t>(
                gil::interleaved_view(
                    m_width
                    , m_height
                    , (rgba_big16_pixel_cptr_t)data
                    , m_png_data->m_rowbytes)
                , my_color_converter())
           );
    }
    else
    {
        if (m_bits_per_component==16)
        {
            // 16 bits
            output_view(
                dest,
                gil::color_converted_view<rgb_big16_pixel_t>(
                    gil::interleaved_view(
                        m_width
                        , m_height
                        , (rgba_big16_pixel_cptr_t)data
                        , m_png_data->m_rowbytes)
                    , my_color_converter())
               );
        }
        else
        {
            // 8 bits
            JAG_ASSERT_MSG(m_bits_per_component==8, "defined by PNG spec");
            output_view(
                dest,
                gil::color_converted_view<gil::rgb8_pixel_t>(
                    gil::interleaved_view(
                        m_width
                            , m_height
                        , (const gil::rgba8_pixel_t *)data
                        , m_png_data->m_rowbytes)
                    , my_color_converter())
               );
        }
    }
}



//////////////////////////////////////////////////////////////////////////
bool ImagePNG::ping(IStreamInput& in_stream)
{
    in_stream.seek(0, OFFSET_FROM_BEGINNING);
    png_byte header[ImagePNG::nr_ping_bytes];
    in_stream.read(header, ImagePNG::nr_ping_bytes);
    return  !png_sig_cmp(header, 0, ImagePNG::nr_ping_bytes);
}

//////////////////////////////////////////////////////////////////////////
shared_ptr<IResourceCtx> ImagePNG::resource_ctx()
{
    return shared_ptr<IResourceCtx>(m_res_ctx);
}


//
//
//
void ImagePNG::report_error() const
{
    JAG_PRECONDITION(m_exception);
    if (!m_exception) {
        JAG_INTERNAL_ERROR;
    }

    exception_operation_failed exc=exception_operation_failed(msg_failed_to_load_png());
    exc << JAGLOC;
    exc.set_next(*m_exception);
    m_exception.reset();
    throw exc;
}

//////////////////////////////////////////////////////////////////////////
void ImagePNG::user_read_data(png_structp png_struct, png_bytep data, png_size_t length)
{
    // Do not allow an exception to propagate from this function as this
    // function is called from a C code so stack unwinding stops there.
    try
    {
        IStreamInput* stream_in = reinterpret_cast<IStreamInput*>(png_get_io_ptr(png_struct));
        stream_in->read(data, length);
    }
    catch(exception const& exc)
    {
        {
            ImagePNG* this_ = reinterpret_cast<ImagePNG*>(png_get_error_ptr(png_struct));
            std::auto_ptr<exception> cloned(exc.clone());
            this_->m_exception.reset(cloned.release());
        }
        longjmp(png_struct->jmpbuf, 0);
    }
    catch(std::exception const& exc)
    {
        {
            ImagePNG* this_ = reinterpret_cast<ImagePNG*>(png_get_error_ptr(png_struct));
            this_->m_exception.reset(new exception_invalid_input(msg_error_while_processing_png_format(exc.what())));
            (*this_->m_exception) << JAGLOC;
        }
        longjmp(png_struct->jmpbuf, 0);
    }
}

//////////////////////////////////////////////////////////////////////////
void ImagePNG::user_error_handler(png_structp png_struct, png_const_charp msg)
{
    {
        ImagePNG* this_ = reinterpret_cast<ImagePNG*>(png_get_error_ptr(png_struct));
        this_->m_exception.reset(new exception_invalid_input(msg_error_while_processing_png_format(msg)));
        (*this_->m_exception) << JAGLOC;
    }
    longjmp(png_struct->jmpbuf, 0);
}

//////////////////////////////////////////////////////////////////////////
void ImagePNG::user_warning_handler(png_structp /*png_struct*/, png_const_charp msg)
{
    write_message(WRN_WARNING_WHILE_PROCESSING_PNG_FORMAT_w, msg);
}


}} //namespace jag::resources






















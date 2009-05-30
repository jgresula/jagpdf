// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGESPEC_H_1108__
#define __IMAGESPEC_H_1108__

#include <interfaces/stdtypes.h>
#include <interfaces/refcounted.h>
#include <interfaces/constants.h>

namespace jag
{
class IImage;

/// Specifies an image to be loaded.
///
/// Besides the standard formats, it is also possible to supply image data in a
/// PDF native format. Note that in such case the 16-bit samples must be
/// big-endian ordered.
///
/// See section [ref_guide_images].
///
class IImageDef
    : public INotRefCounted
{
public:

    /**
     * @name Image data source
     *
     * Either of these methods must be called.
     */
    //@{
    /// Specifies a path to an image file.
    ///
    /// Either this method or data() is mandatory.
    ///
    virtual void file_name(Char const* file_name) = 0;


    /// Specifies image data in memory.
    ///
    /// Either this method or file_name() is mandatory.
    ///
    /// @param array_in image data
    /// @param length image data byte size
    ///
    virtual void data(Byte const* array_in, UInt length) = 0;
    //@}


    /**
     * @name Mandatory for IMAGE_FORMAT_NATIVE
     *
     * These methods must be called in order to define an image.  Has no effect
     * for images of type other than IMAGE_FORMAT_NATIVE.
     */
    //@{

    /// Specifies the image dimensions.
    ///
    /// Has effect only for the native format.
    ///
    virtual void dimensions(UInt width, UInt height) = 0;

    /// Specifies the image color space.
    ///
    /// Mandatory for the native format. Can be set for other image formats as
    /// well providing that the number of color components matches the image.
    ///
    virtual void color_space(ColorSpace cs) = 0;

    /// Specifies number of bits per color component.
    ///
    /// Mandatory for the native format except it has an indexed color
    /// space. Has no effect for other image formats.
    ///
    /// @param bpc allowed values: 1, 2, 4, 8 and (in PDF 1.5) 16
    ///
    virtual void bits_per_component(UInt bpc) = 0;
    //@}


    /// Specifies the image format.
    ///
    /// If not specified then [lib] attempts to automatically recognize the
    /// image format.
    ///
    /// @param format
    ///  - `IMAGE_FORMAT_NATIVE`
    ///  - `IMAGE_FORMAT_PNG`
    ///  - `IMAGE_FORMAT_JPEG`
    ///
    virtual void format(ImageFormat format) = 0;


    /// Specifies the image resolution in DPI.
    ///
    /// If not specified then the value of [^images.default_dpi] profile option
    /// is used for the native format. For other image formats it overrides the
    /// image's intrinsic DPI.
    ///
    virtual void dpi(Double xdpi, Double ydpi) = 0;


    /// Specifies whether to interpolate the image.
    ///
    /// If not specified then the value of [^images.interpolated] profile option
    /// is used.  Note that interpolation may increase the time required to
    /// render the image.
    ///
    /// @param flag non-zero value enables interpolation
    ///
    /// @see [url_pdfref_chapter Graphics | Images | Image Dictionaries | Image
    /// Interpolation].
    ///
    virtual void interpolate(Int flag) = 0;


    /// Maps image samples into the specified range.
    ///
    /// Can be used for all image formats.
    ///
    /// There is a special case when [lib] uses the decode array itself: Image
    /// samples in CMYK JPEGs are inverted and the decode function is used for
    /// inverting them back.
    ///
    /// @param array_in  decode values
    /// @param length number of values (must be twice the number of color
    ///             components)
    ///
    /// @see [url_pdfref_chapter Graphics | Images | Image Dictionaries | Decode
    /// Arrays].
    ///
    virtual void decode(Double const* array_in, UInt length) = 0;

    /**
     * @name Masks
     *
     * If multiple masks are set, then only one is used (order: soft, color key, hard).
     */
    //@{

    /// Specifies the color mask applied to the image.
    ///
    /// Can be specified for all image formats. For formats other than the
    /// native it overrides the intrinsic mask.
    ///
    /// @param array_in color key mask values
    /// @param length number of values (must be twice the number of color
    ///             components)
    ///
    /// @see [url_pdfref_chapter Graphics | Images | Masked Images | Color Key
    /// Masking].
    ///
    virtual void color_key_mask(UInt const* array_in, UInt length) = 0;


    //@}

    /// Specifies the gamma correction applied to the image.
    ///
    /// The default value is 1. Can be specified for all image formats. For
    /// formats other than the native it overrides the intrinsic gamma value.
    ///
    /// * @param val gamma value for all color components (must be>0)
    ///
    virtual void gamma(Double val) = 0;


    /// Specifies an image alternative designed for printing.
    ///
    /// The alternative representation of the image may differ, for example, in
    /// resolution or in color space. The primary goal is to reduce the need to
    /// maintain separate versions of a PDF document for low-resolution
    /// on-screen viewing and high-resolution printing.
    ///
    /// @jag_pdfversion 1.3
    ///
    virtual void alternate_for_printing(IImage* image) = 0;

public:
    /**
     * @brief Mask applied to the image.
     *
     * For types other than IMAGE_FORMAT_NATIVE overrides the intrinsic mask.
     *
     * @param image_mask    hard mask to use
     */
    virtual void image_mask(ImageMaskID image_mask) API_ATTR("undocumented") = 0;

    /// Rendering intent used for drawing the image.
    ///
    virtual void rendering_intent(RenderingIntentType intent) API_ATTR("undocumented") = 0;



protected:
    ~IImageDef() {}
};


} // namespace jag

#endif //__IMAGESPEC_H_1108__

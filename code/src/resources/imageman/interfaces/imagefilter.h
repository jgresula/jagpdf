// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEFILTERDATA_H_JAG_1245__
#define __IMAGEFILTERDATA_H_JAG_1245__

#include <resources/resourcebox/resourcepodarrays.h>
#include <resources/interfaces/resourceconstants.h>
#include <resources/interfaces/resourcehandle.h>
#include <core/generic/noncopyable.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace jag {
class IImageMaskData;
class ISeqStreamOutput;
class IStreamInput;

namespace resources {

/**
 * @brief Interface abstracting an image input.
 *
 * Image filter can also provide a mask for the image. This
 * way various modes of transparency can be achived.
 */
class IImageFilter
    : public noncopyable
{
public:
    /// Image width.
    virtual UInt width() const = 0;
    /// Image height.
    virtual UInt height() const = 0;

    /// Retrieves image's color space. Must be valid handle.
    virtual ColorSpaceHandle color_space() const = 0;

    /// Number of bits per color components.
    virtual UInt bits_per_component() const = 0;

    /// Retrieves image horizontal resolution; 0 in case this information is not available.
    virtual Double dpi_x() const = 0;

    /// Retrieves image vertical resolution; 0 in case this information is not available.
    virtual Double dpi_y() const = 0;

    /// Decode array for each component
    virtual DecodeArray const* decode() const = 0;

    /// Retrieves gamma correction or value<=0.0 when no correction in place
    virtual Double gamma() const = 0;

    /// Retrieves type of the associated image mask (if any)
    virtual ImageMaskType image_mask_type() const = 0;

    /**
     * @brief Image's color key mask.
     *
     * @pre    image_mask_type() == IMT_COLOR_KEY
     * @return mask
     */
    virtual ColorKeyMaskArray const& color_key_mask() const = 0;

    /**
     * @brief Image's mask.
     *
     * @pre    image_mask_type() == IMT_HARD
     * @return mask
     */
    virtual boost::intrusive_ptr<IImageMaskData> create_mask() const = 0;

    /// Outputs image data in form recognizable by a PDF consumer
    virtual bool output_image(ISeqStreamOutput& dest, unsigned max_bits) const = 0;

    /// Provides the whole image stream
    virtual boost::shared_ptr<IStreamInput> data_stream() const = 0;

    virtual ~IImageFilter() {}
};

}} //namespace jag::resources

#endif //__IMAGEFILTERDATA_H_JAG_1245__

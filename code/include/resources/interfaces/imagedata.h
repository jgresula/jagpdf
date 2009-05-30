// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEDATA_H_JG2219__
#define __IMAGEDATA_H_JG2219__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <resources/interfaces/imageproperties.h>
#include <resources/resourcebox/resourcepodarrays.h>
#include <resources/interfaces/resourcehandle.h>
#include <resources/interfaces/resourceconstants.h>
#include <interfaces/constants.h>
#include <boost/shared_ptr.hpp>

namespace jag
{
class ISeqStreamOutput;
class IStreamInput;

//////////////////////////////////////////////////////////////////////////
class IImageData
    : public IImage
{
public:
    virtual ~IImageData() {}
    virtual ImageFormat format() const = 0;
    virtual ColorSpaceHandle color_space() const = 0;

    virtual ImageHandle alternate_for_printing() const = 0;
    virtual RenderingIntentType rendering_intent() const = 0;
    virtual DecodeArray const& decode() const = 0;
    virtual InterpolateType interpolate() const = 0;
    virtual Double gamma() const = 0;
    virtual Int has_gamma() const = 0;

    virtual ImageMaskType image_mask_type() const = 0;
    virtual ColorKeyMaskArray const& color_key_mask() const = 0;
    virtual ImageMaskHandle image_mask() const = 0;

    /**
     * @brief provides image data in pdf format
     *
     * Based on configuration it might happen that this method can be called
     * only once. Any further calls result to an error.
     *
     * return true on success, false if the required precision can't be provided
     */
    virtual bool output_image(ISeqStreamOutput& fout, unsigned max_bits) const = 0;

    // provides raw image data stream, might be gone if the image has been already outputted
    virtual boost::shared_ptr<IStreamInput> data_stream() const = 0;

public:
    virtual ImageHandle handle() const = 0;
    virtual void handle(ImageHandle handle) = 0;
};

} //namespace jag


#endif //__IMAGEDATA_H_JG2219__


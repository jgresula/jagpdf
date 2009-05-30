// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef IMAGEMASK_JG1628_H__
#define IMAGEMASK_JG1628_H__

#include <interfaces/stdtypes.h>
#include <interfaces/refcounted.h>

namespace jag {

/**
 */
class API_ATTR("undocumented") IImageMask
    : public IRefCounted
{
public:
    /**
     * @name Image data source
     *
     * Either of these methods must be called.
     */
    //@{
    /**
     * @brief Path to a file containing the mask.
     *
     * Either this method or data() is mandatory.
     * In case of 16bit mask bytes are big-endian ordered.
     *
     * @param file_name    file with data
     */
    virtual void file_name(Char const* file_name) = 0;
    /**
     * @brief Specifies mask data.
     *
     * Either this method or file_name() is mandatory.
     * In case of 16bit mask bytes are big-endian ordered.
     *
     * @param array_in image samples
     * @param length length of the array
     */
    virtual void data(Byte const* array_in, UInt length) = 0;
    //@}

    /**
     * @name Mandatory
     */
    //@{
    /// Mask dimensions
    virtual void dimensions(UInt width, UInt height) = 0;
    /// Mask bit depth
    virtual void bit_depth(UInt bps) = 0;
    //@}


    /**
     * @brief Maps mask samples into the specified range.
     *
     * @param lbound    range lower bound
     * @param ubound    range upper bound
     */
    virtual void decode(Double lbound, Double ubound) = 0;

    /// Whether to interpolate the Mask.
    virtual void interpolate(Int val) = 0;

    /**
     * @brief Matte color.
     *
     * Color with which the image data in the parent image
     * has been preblended.
     *
     * If specified then the mask must have the same
     * dimensions as the parent image.
     *
     * @param array_in  color components in the parent image color space
     * @param length    must correspond to number of componets of parent
     *                  image color space
     */
    virtual void matte(Double const* array_in, UInt length) = 0;

protected:
    ~IImageMask() {}
};


} // namespace jag

#endif // IMAGEMASK_JG1628_H__
/** EOF @file */

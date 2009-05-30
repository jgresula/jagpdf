// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEPROPERTIES_H_JG_1513__
#define __IMAGEPROPERTIES_H_JG_1513__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/refcounted.h>

namespace jag
{

/// Represents an image.
///
/// Uniquely identifies a image and provides information about the image
class IImage
    : public INotRefCounted
{
public:
    /// Retrieves width of the image in pixels.
    virtual UInt width() const = 0;
    /// Retrieves height of the image in pixels.
    virtual UInt height() const = 0;
    /// Retrieves number of bits per color component.
    virtual UInt bits_per_component() const = 0;
    /// Retrieves horizontal resolution in DPI.
    virtual Double dpi_x() const = 0;
    /// Retrieves vertical resolution in DPI.
    virtual Double dpi_y() const = 0;

protected:
    ~IImage() {}
};


} //namespace jag


#endif //__IMAGEPROPERTIES_H_JG_1513__



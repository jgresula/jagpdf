// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEMAN_H_JAG_1158__
#define __IMAGEMAN_H_JAG_1158__

#include <resources/interfaces/resourcehandle.h>
#include <core/generic/noncopyable.h>
#include <interfaces/constants.h>
#include <boost/intrusive_ptr.hpp>

namespace jag {

class IImageDef;
class IImageMask;
class IImageMaskData;
class IImageSoftMaskSpec;
class IImageData;
class IExecContext;


class IImageMan
    : public noncopyable
{
public:
    virtual IImageData const* image_load_file(Char const* image_file_path, ImageFormat image_type, IExecContext const& exec_ctx) = 0;
    virtual IImageDef* image_definition() const = 0;
    virtual IImageData const* image_load(IImageDef* image, IExecContext const& exec_ctx) = 0;
    virtual IImageData const* image_data(ImageHandle image_id) const = 0;

    virtual boost::intrusive_ptr<IImageMask> define_image_mask() const = 0;
    virtual ImageMaskHandle register_image_mask(boost::intrusive_ptr<IImageMask> image_mask) = 0;
    virtual ImageMaskHandle register_image_mask(boost::intrusive_ptr<IImageMaskData> image_mask) = 0;
    virtual IImageMaskData const& image_mask_data(ImageMaskHandle imagemask) const = 0;

protected:
    ~IImageMan() {}
};

} // namespace jag


#endif //__IMAGEMAN_H_JAG_1158__


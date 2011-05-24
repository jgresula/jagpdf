// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/imageman/imagemanimpl.h>
#include <resources/imageman/imagespecimpl.h>
#include "imagemaskimpl.h"
#include "imagefilterdata.h"
#include "imagepng.h"
#include "imagejpeg.h"
#include <msg_resources.h>
#include <resources/interfaces/imagemaskdata.h>
#include <core/generic/refcountedimpl.h>
#include <core/generic/checked_cast.h>
#include <core/errlib/errlib.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/checked_delete.hpp>

using namespace boost;

namespace jag {
namespace resources {


//////////////////////////////////////////////////////////////////////////
ImageManImpl::ImageManImpl(
      weak_ptr<IResourceCtx> resource_ctx
)
    : m_resource_ctx(resource_ctx)
{
}

//
//
//
ImageManImpl::~ImageManImpl()
{
}



//////////////////////////////////////////////////////////////////////////
IImageData const* ImageManImpl::image_load_file(Char const* image_file_path, ImageFormat image_format, IExecContext const& exec_ctx)
{
    IImageDef* img_spec(image_definition());

    img_spec->format(image_format);
    img_spec->file_name(image_file_path);
    return image_load(img_spec, exec_ctx);
}



//////////////////////////////////////////////////////////////////////////
IImageDef* ImageManImpl::image_definition() const
{
    std::auto_ptr<ImageSpecImpl>
        new_one(new ImageSpecImpl);

    return m_image_spec_list.add(new_one);
}



//////////////////////////////////////////////////////////////////////////
IImageData const* ImageManImpl::image_load(IImageDef* image, IExecContext const& exec_ctx)
{
    if (!image)
        throw exception_invalid_value(msg_null_pointer()) << JAGLOC;

    ImgSpecList::Iterator it(
        m_image_spec_list.lookup(
            checked_static_cast<ImageSpecImpl*>(image)));

    it->check_consistency(exec_ctx);

    // based on spec create IImageData and put it to the resource table
    IImageData* result = 0;
    ImageHandle handle;
    if (IMAGE_FORMAT_NATIVE == it->format())
    {
        // just upcast for native image and move ownership form spec list to the
        // resource table.
        result = &*it;
        handle = m_images.add(m_image_spec_list.detach(it).release());
    }
    else
    {
        result = new ImageFilterData(
                &*it,
                shared_ptr<IResourceCtx>(m_resource_ctx),
                exec_ctx);
        handle = m_images.add(result);
        m_image_spec_list.detach(it); // releases the definition
    }

    JAG_ASSERT(result);
    JAG_ASSERT(is_valid(handle));
    result->handle(handle);
    return result;
}



//////////////////////////////////////////////////////////////////////////
IImageData const* ImageManImpl::image_data(ImageHandle image_id) const
{
    return &m_images.lookup(image_id);
}





//-------------------------------------------------------------------------
//                                MASKS
//

//////////////////////////////////////////////////////////////////////////
intrusive_ptr<IImageMask> ImageManImpl::define_image_mask() const
{
    return intrusive_ptr<IImageMask>(new RefCountImpl<ImageMaskImpl>);
}


///////////////////////////////////////////////////////////////////////////
ImageMaskHandle ImageManImpl::register_image_mask(intrusive_ptr<IImageMask> image_mask)
{
    if (!image_mask)
        throw exception_invalid_value(msg_null_pointer()) << JAGLOC;

    ImageMaskImpl* base(
        checked_static_cast<ImageMaskImpl*>(image_mask.get())
   );

    base->check_consistency();

    return m_image_masks.add(boost::intrusive_ptr<IImageMaskData>(base));
}

//////////////////////////////////////////////////////////////////////////
ImageMaskHandle ImageManImpl::register_image_mask(intrusive_ptr<IImageMaskData> image_mask)
{
    return m_image_masks.add(image_mask);
}

//////////////////////////////////////////////////////////////////////////
IImageMaskData const& ImageManImpl::image_mask_data(ImageMaskHandle imagemask) const
{
    return *m_image_masks.lookup(imagemask);
}


// --------------------------------------------------------------------
//                      Free functions


/// Attempts to detect image format in the stream.
///
/// Returns IMAGE_FORMAT_UNRECOGNIZED if the format is not recognized.
///
ImageFormat recognize_image_format(IStreamInput& img_stream)
{
    if (ImageJPEG::ping(img_stream)) {
        return IMAGE_FORMAT_JPEG;
    }
    else if (ImagePNG::ping(img_stream)) {
        return IMAGE_FORMAT_PNG;
    }
    return IMAGE_FORMAT_UNRECOGNIZED;
}

///
///
///
std::auto_ptr<IImageFilter>
create_image_filter(
    IImageData const* img_data
    , shared_ptr<IResourceCtx> resource_ctx
    , IExecContext const& exec_context
)
{
    JAG_ASSERT(img_data->format() != IMAGE_FORMAT_AUTO);

    switch (img_data->format())
    {
    case IMAGE_FORMAT_PNG:
        return std::auto_ptr<IImageFilter>(new ImagePNG(img_data, resource_ctx, exec_context));

    case IMAGE_FORMAT_JPEG:
        return std::auto_ptr<IImageFilter>(new ImageJPEG(img_data, resource_ctx));

    default:
        // it could be possible to support IMAGE_FORMAT_UNKNOWN and find out the
        // the actual format by pinging the known ones
        throw exception_invalid_input(msg_unsupported_image_format()) << JAGLOC;
    }
}


}} // namespace jag::resources

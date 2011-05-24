// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEMANIMPL_H_JAG_1211__
#define __IMAGEMANIMPL_H_JAG_1211__

#include <resources/utils/resourcetable.h>
#include <resources/interfaces/imageman.h>
#include <resources/interfaces/imagedata.h>
#include <resources/interfaces/imagemaskdata.h>
#include <resources/imageman/imagespecimpl.h>

#include <boost/weak_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace jag {
class IResourceCtx;
class IExecContext;

namespace resources {
class IImageFilter;

class ImageManImpl
    : public IImageMan
{
public: //IImageMan
    IImageData const* image_load_file(Char const* image_file_path, ImageFormat image_type, IExecContext const& exec_ctx);
    IImageDef* image_definition() const;
    IImageData const* image_load(IImageDef* image, IExecContext const& exec_ctx);
    IImageData const* image_data(ImageHandle image_id) const;

    boost::intrusive_ptr<IImageMask> define_image_mask() const;
    ImageMaskHandle register_image_mask(boost::intrusive_ptr<IImageMask> image_mask);
    ImageMaskHandle register_image_mask(boost::intrusive_ptr<IImageMaskData> image_mask);
    IImageMaskData const& image_mask_data(ImageMaskHandle imagemask) const;

public:
    ImageManImpl(boost::weak_ptr<IResourceCtx> resource_ctx);
    ~ImageManImpl();


private:
    // list of all issued definitions that have not been loaded so far
    typedef IssuedResDefList<ImageSpecImpl> ImgSpecList;
    mutable ImgSpecList m_image_spec_list;

    ResourceTable<ImageHandle, IImageData, boost::ptr_vector<IImageData> > m_images;
    ResourceTable<ImageMaskHandle,boost::intrusive_ptr<IImageMaskData> > m_image_masks;

    boost::weak_ptr<IResourceCtx> m_resource_ctx;
};


// ---------------------------------------------------------------
//                      Free functions
//
ImageFormat recognize_image_format(IStreamInput& img_stream);

std::auto_ptr<IImageFilter> create_image_filter(
      IImageData const* img_spec
    , boost::shared_ptr<IResourceCtx> resource_ctx
    , IExecContext const& exec_context      
);



}} // namespace jag::resources

#endif //__IMAGEMANIMPL_H_JAG_1211__


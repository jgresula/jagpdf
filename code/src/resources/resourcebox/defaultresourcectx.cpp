// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/resourcebox/resourcectxfactory.h>
#include <resources/imageman/imagemanimpl.h>
#include <resources/typeman/typemanimpl.h>
#include <resources/othermanagers/colorspacemanimpl.h>
#include <resources/interfaces/imageman.h>
#include <resources/interfaces/resourcectx.h>
#include <boost/intrusive_ptr.hpp>

using namespace boost;

namespace jag { namespace resources
{

//////////////////////////////////////////////////////////////////////////
shared_ptr<IResourceCtx> create_default_resource_ctx()
{
    shared_ptr<IResourceCtx> res_ctx(create_resource_ctx());
    shared_ptr<IImageMan> img_man(new ImageManImpl(res_ctx));
    shared_ptr<IColorSpaceMan> color_space_man(new ColorSpaceManImpl);
    shared_ptr<ITypeMan> type_man(new TypeManImpl);

    res_ctx->set_type_man(type_man);
    res_ctx->set_image_man(img_man);
    res_ctx->set_color_space_man(color_space_man);
    return res_ctx;
}

}} // namespace jag::resources

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/interfaces/resourcectx.h>

using namespace boost;

namespace jag {
namespace resources {

class ResourceCtxImpl
    : public IResourceCtx
{
public:
    shared_ptr<IImageMan> const& image_man() const {
        return m_image_man;
    }

    void set_image_man(shared_ptr<IImageMan> image_man) {
        m_image_man = image_man;
    }

    shared_ptr<IColorSpaceMan> const& color_space_man() const {
        return m_color_space_man;
    }

    void set_color_space_man(shared_ptr<IColorSpaceMan> color_space_man) {
        m_color_space_man = color_space_man;
    }

    shared_ptr<ITypeMan> const& type_man() const {
        return m_type_man;
    }

    void set_type_man(shared_ptr<ITypeMan> type_man) {
        m_type_man = type_man;
    }

    ~ResourceCtxImpl() {}


private:
    shared_ptr<IImageMan>      m_image_man;
    shared_ptr<IColorSpaceMan> m_color_space_man;
    shared_ptr<ITypeMan>       m_type_man;
};


//////////////////////////////////////////////////////////////////////////
shared_ptr<IResourceCtx> create_resource_ctx()
{
    return shared_ptr<IResourceCtx>(new ResourceCtxImpl);
}

}} // namespace jag::resources






















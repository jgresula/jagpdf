// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCECTX_H_JAG_1131__
#define __RESOURCECTX_H_JAG_1131__

#include <core/generic/noncopyable.h>
#include <boost/shared_ptr.hpp>

namespace jag {
class IImageMan;
class IColorSpaceMan;
class ITypeMan;


/// keeps resource managers
class IResourceCtx
    : public noncopyable
{
public:
    virtual boost::shared_ptr<IImageMan> const& image_man() const = 0;
    virtual void set_image_man(boost::shared_ptr<IImageMan> image_man) = 0;

    virtual boost::shared_ptr<IColorSpaceMan> const& color_space_man() const = 0;
    virtual void set_color_space_man(boost::shared_ptr<IColorSpaceMan> cs_man) = 0;

    virtual boost::shared_ptr<ITypeMan> const& type_man() const = 0;
    virtual void set_type_man(boost::shared_ptr<ITypeMan> type_man) = 0;

protected:
    ~IResourceCtx() {}
};

} // namespace jag


#endif //__RESOURCECTX_H_JAG_1131__






















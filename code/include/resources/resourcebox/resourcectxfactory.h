// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCECTXFACTORY_H_JAG_1148__
#define __RESOURCECTXFACTORY_H_JAG_1148__

#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

namespace jag {
class IResourceCtx;

namespace resources {

/// creates an empty resource context
boost::shared_ptr<IResourceCtx> create_resource_ctx();

/// creates a default resource context
boost::shared_ptr<IResourceCtx> create_default_resource_ctx();


}} // namespace jag::resources

#endif //__RESOURCECTXFACTORY_H_JAG_1148__




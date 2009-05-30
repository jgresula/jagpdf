// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <boost/shared_array.hpp>

#ifndef __SHAREDARRAY_H_JAG_1238__
#define __SHAREDARRAY_H_JAG_1238__

namespace jag
{

typedef std::pair<boost::shared_array</*const*/ Byte>,std::size_t> SharedArray;

} //namespace jag

#endif //__SHAREDARRAY_H_JAG_1238__

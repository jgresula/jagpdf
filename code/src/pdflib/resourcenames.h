// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCENAMES_H_JG2254__
#define __RESOURCENAMES_H_JG2254__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <resources/interfaces/resourcehandle.h>
#include <boost/static_assert.hpp>
#include <algorithm>

namespace jag {
namespace pdf {

class FontDictionary;

typedef std::pair<char const*,unsigned int> ResNameInfo;

template<class T>
ResNameInfo get_resource_name_record()
{
    BOOST_STATIC_ASSERT(sizeof(T)==0); // unknown resource type
    return ResNameInfo();
}

//////////////////////////////////////////////////////////////////////////
template<>
inline ResNameInfo get_resource_name_record<PatternHandle>()
{
    return std::make_pair("/pt", 3);
}

//////////////////////////////////////////////////////////////////////////
template<>
inline ResNameInfo get_resource_name_record<ColorSpaceHandle>()
{
    return std::make_pair("/cs", 3);
}

//////////////////////////////////////////////////////////////////////////
template<>
inline ResNameInfo get_resource_name_record<ImageHandle>()
{
    return std::make_pair("/im", 3);
}

//////////////////////////////////////////////////////////////////////////
template<>
inline ResNameInfo get_resource_name_record<GraphicsStateHandle>()
{
    return std::make_pair("/gs", 3);
}

//////////////////////////////////////////////////////////////////////////
template<>
inline ResNameInfo get_resource_name_record<FontDictionary>()
{
    return std::make_pair("/fn", 3);
}

//////////////////////////////////////////////////////////////////////////
template<>
inline ResNameInfo get_resource_name_record<ShadingHandle>()
{
    return std::make_pair("/sh", 3);
}


}} //namespace jag::pdf


#endif //__RESOURCENAMES_H_JG2254__


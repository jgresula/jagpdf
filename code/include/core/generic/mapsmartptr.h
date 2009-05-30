// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __MAPSMARTPTR_H_JAG_1825__
#define __MAPSMARTPTR_H_JAG_1825__

#include <core/generic/smartptrutils.h>
#include <map>

namespace jag
{

/**
 * @brief map keyed by a value of object held in shared pointer
 *
 * @param K key type, must define operator<
 * @param V value
 */
template <class K, class V, class Pred=std::less<K> >
struct map_shared_ptr
{
    typedef std::map<
          boost::shared_ptr<K>
        , V
        , shared_ptr_unfold_binary<K, Pred >
    >  type;
};

} // namespace jag

#endif //__MAPSMARTPTR_H_JAG_1825__

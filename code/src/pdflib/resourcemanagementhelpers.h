// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCEMANAGEMENT_H_JG1157__
#define __RESOURCEMANAGEMENT_H_JG1157__

#include "indirectobjectref.h"
#include "interfaces/indirect_object.h"
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>

namespace jag {
namespace pdf {

namespace
{

  /// retrieves object reference from resource handler (for handlers derived from IIndirectObject)
  template<class T>
  IndirectObjectRef res_to_objref(typename boost::enable_if<boost::is_base_of<IIndirectObject,T>,T>::type const& obj)
  {
      return IndirectObjectRef(obj);
  }

  /// retrieves object reference from resource handler (for handlers exposing its reference through ref())
  template<class T>
  IndirectObjectRef res_to_objref(typename boost::disable_if<boost::is_base_of<IIndirectObject,T>,T>::type const& obj)
  {
      return obj.ref();
  }


 /**
  *  @brief generic resource output and reference provider
  *
  *  @param Handler object performing object output and providing reference to it
  *  @param handle resource handle
  *  @param res_map handle to object reference mapping
  *  @param to_resource function object transforming handle to a resource
  *  @param doc document
  */
  template<class Handler, class ToResource, class Handle, class Map>
  IndirectObjectRef const& reference_provider(
        Handle handle
      , Map& res_map
      , ToResource const& to_resource
      , DocWriterImpl& doc
 )
  {
      typename Map::iterator it = res_map.find(handle);
      if (it == res_map.end())
      {
          typename ToResource::result_type resource(to_resource(handle));
          Handler handler(doc, resource);
          handler.output_definition();
          it = res_map.insert(std::make_pair(handle, res_to_objref<Handler>(handler))).first;
      }

      return it->second;
  }



  template<class T>
  struct identity
      : public std::unary_function<T,T>
  {
      T operator()(T const& t) const
      {
          return t;
      }
  };


} // anonymous namespace

}} //namespace jag::pdf


#endif //__RESOURCEMANAGEMENT_H_JG1157__
























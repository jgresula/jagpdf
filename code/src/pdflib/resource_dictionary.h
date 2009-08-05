// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __RESOURCE_DICTIONARY_H__
#define __RESOURCE_DICTIONARY_H__

#include "indirectobjectimpl.h"
#include "indirectobjectref.h"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace jag {
namespace pdf {

class DocWriterImpl;
class ResourceList;
class ObjFmt;

/// represents resource dictionary
class ResourceDictionary
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE;
    explicit ResourceDictionary(DocWriterImpl& doc,
                                double page_height);
    void add_resources(boost::shared_ptr<ResourceList> const& resources);

private: // IndirectObjectImpl
    bool on_before_output_definition();
    void on_output_definition();

private:
    void form_resource_list();


private:
    typedef std::vector<boost::shared_ptr<ResourceList> > ResourceVec;
    ResourceVec m_resources;
    double m_page_height;

    // cached, resource list actually used, needed during output
    boost::shared_ptr<ResourceList> m_compiled_res_list;
};


/**
 * @brief outputs resource dictionary (if needed)
 *
 * @param doc document context
 * @param res_list list of resources to output (can be NULL or empty)
 *
 * @param Reference to resource dictionary object (an invalid one in case no
 *        resource dictionary was outputted.
 */
IndirectObjectRef output_resource_dictionary(
    DocWriterImpl& doc,
    boost::shared_ptr<ResourceList> res_list,
    double page_height = 0.0);


/**
 * @brief outputs an item referring to the resource dictionary
 *
 * @param resource_dict reference to indirect object representing resource dictionary
 *                      if the reference is invalid then nothing is outputted
 * @param fmt where to output the item
 */
void output_resource_dictionary_ref(IndirectObjectRef const& resource_dict, ObjFmt& fmt);


}} //namespace jag::pdf

#endif //__RESOURCE_DICTIONARY_H__

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "resource_dictionary.h"
#include "docwriterimpl.h"
#include "objfmt.h"
#include "resourcelist.h"
#include "resourcenames.h"
#include "resourcemanagement.h"
#include "defines.h"
#include <core/generic/assert.h>
#include <boost/bind.hpp>

namespace jag {
namespace pdf {

namespace
{

//////////////////////////////////////////////////////////////////////////
// 'before' functors
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
template<class Handle, class HandleToRef>
class FnBeforeOutput
{
public:
    FnBeforeOutput(HandleToRef const& handle_to_ref)
        : m_handle_to_ref(handle_to_ref)
    {}

    void operator()(Handle const& val) const {
        m_handle_to_ref(val);
    }

private:
    HandleToRef const& m_handle_to_ref;
};


/// convenient FnBeforeOutput creator
template<class Handle, class HandleToRef>
FnBeforeOutput<Handle,HandleToRef> make_before_output_fn(HandleToRef const& handle_to_ref)
{
    return FnBeforeOutput<Handle,HandleToRef>(handle_to_ref);
}



//////////////////////////////////////////////////////////////////////////
// 'output' functors
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
template <class Handle, class HandleToRef>
class FnGenericResourceWriter
{
public:
    FnGenericResourceWriter(ObjFmt& writer, HandleToRef const& handle_to_ref)
        : m_writer(writer)
        , m_handle_to_ref(handle_to_ref)
    {}

    void operator()(Handle const& val) const {
        m_writer
            .output_resource(val)
            .space()
            .ref(m_handle_to_ref(val))
            ;
    }
protected:
    ObjFmt&             m_writer;
    HandleToRef const&  m_handle_to_ref;
};


/// convenient FnGenericResourceWriter maker
template <class Handle, class HandleToRef>
FnGenericResourceWriter<Handle,HandleToRef>
make_resource_writer(ObjFmt& writer, HandleToRef const& handle_to_ref)
{
    return FnGenericResourceWriter<Handle,HandleToRef>(writer, handle_to_ref);
}



//////////////////////////////////////////////////////////////////////////
class ResWriterBase
{
public:
    ResWriterBase(ResourceManagement& res_mgm, ObjFmt& writer)
        : m_res_mgm(res_mgm)
        , m_writer(writer)
    {}

protected:
    ResourceManagement& m_res_mgm;
    ObjFmt&                m_writer;
};



//////////////////////////////////////////////////////////////////////////
// generic output functions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
template<class T, class FN>
void output_generic(char const* name, ObjFmt& writer, T const& range, FN const& fn)
{
    if (range.first==range.second)
        return;

    writer
        .dict_key(name)
        .space()
        .dict_start()
    ;

    std::for_each(range.first, range.second, fn);
    writer.dict_end();
}

//////////////////////////////////////////////////////////////////////////
template<class T, class FN>
void before_output_generic(T const& range, FN const& fn)
{
    if (range.first==range.second)
        return;

    std::for_each(range.first, range.second, fn);
}


} // anonymous


//////////////////////////////////////////////////////////////////////////
// class ResourceDictionary
//////////////////////////////////////////////////////////////////////////


/// Constructor.
ResourceDictionary::ResourceDictionary(DocWriterImpl& doc,
                                       double page_height)
    : IndirectObjectImpl(doc)
    , m_page_height(page_height)
{
}

//////////////////////////////////////////////////////////////////////////
void ResourceDictionary::form_resource_list()
{
    if (m_resources.size() > 1)
    {
        JAG_TBD;
        // union of all resource lists
    }
    else
    {
        m_compiled_res_list = m_resources[0];
    }
}

//////////////////////////////////////////////////////////////////////////
bool ResourceDictionary::on_before_output_definition()
{
    // eventually output any resources mentioned in this dictionary
    JAG_PRECONDITION(!m_resources.empty());
    form_resource_list();

    ResourceManagement& res_mgm = doc().res_mgm();
    before_output_generic(
        m_compiled_res_list->patterns(),
        make_before_output_fn<PatternHandle>(
            boost::bind(&ResourceManagement::pattern_ref, &res_mgm, _1, m_page_height))
   );

    before_output_generic(
        m_compiled_res_list->images()
        , make_before_output_fn<ImageHandle>(boost::bind(&ResourceManagement::image_reference, &res_mgm, _1))
   );

    before_output_generic(
        m_compiled_res_list->color_spaces()
        , make_before_output_fn<ColorSpaceHandle>(boost::bind(&ResourceManagement::color_space_ref, &res_mgm, _1))
   );

    before_output_generic(
        m_compiled_res_list->graphics_states()
        , make_before_output_fn<GraphicsStateHandle>(boost::bind(&ResourceManagement::graphics_state_reference, &res_mgm, _1))
   );

    before_output_generic(
        m_compiled_res_list->shadings()
        , make_before_output_fn<ShadingHandle>(boost::bind(&ResourceManagement::shading_ref, &res_mgm, _1))
   );
    return true;
}


/**
 * @brief outputs the resource dictionary
 */
void ResourceDictionary::on_output_definition()
{
    JAG_PRECONDITION(m_compiled_res_list);
    form_resource_list();

    ObjFmt& writer = object_writer();
    writer.dict_start();
    ResourceManagement& res_mgm = doc().res_mgm();

    output_generic(
        "Pattern"
        , writer
        , m_compiled_res_list->patterns()
        , make_resource_writer<PatternHandle>(
            writer,
            boost::bind(&ResourceManagement::pattern_ref, &res_mgm, _1, m_page_height))
   );

    output_generic(
        "XObject"
        , writer
        , m_compiled_res_list->images()
        , make_resource_writer<ImageHandle>(
            writer, boost::bind(&ResourceManagement::image_reference, &res_mgm, _1))
   );


    output_generic(
        "ColorSpace"
        , writer
        , m_compiled_res_list->color_spaces()
        , make_resource_writer<ColorSpaceHandle>(
            writer, boost::bind(&ResourceManagement::color_space_ref, &res_mgm, _1))
   );


    output_generic(
        "ExtGState"
        , writer
        , m_compiled_res_list->graphics_states()
        , make_resource_writer<GraphicsStateHandle>(
            writer, boost::bind(&ResourceManagement::graphics_state_reference, &res_mgm, _1))
   );

    output_generic(
        "Shading"
        , writer
        , m_compiled_res_list->shadings()
        , make_resource_writer<ShadingHandle>(
            writer, boost::bind(&ResourceManagement::shading_ref, &res_mgm, _1))
   );

    FontManagement& font_mgm = res_mgm.fonts();
    output_generic(
        "Font"
        , writer
        , m_compiled_res_list->fonts()
        , make_resource_writer<FontDictionary>(
            writer, boost::bind(&FontManagement::font_ref, &font_mgm, _1))
   );


    writer.dict_end();
    m_compiled_res_list.reset();
}



//////////////////////////////////////////////////////////////////////////
void ResourceDictionary::add_resources(boost::shared_ptr<ResourceList> const& resources)
{
    m_resources.push_back(resources);
}





//////////////////////////////////////////////////////////////////////////
IndirectObjectRef output_resource_dictionary(
    DocWriterImpl& doc,
    boost::shared_ptr<ResourceList> res_list,
    double page_height)
{
    if (res_list && !res_list->is_empty())
    {
        ResourceDictionary res_dict(doc, page_height);
        res_dict.add_resources(res_list);
        res_dict.output_definition();
        return IndirectObjectRef(res_dict);
    }
    return IndirectObjectRef();
}

//////////////////////////////////////////////////////////////////////////
void output_resource_dictionary_ref(IndirectObjectRef const& resource_dict, ObjFmt& fmt)
{
    fmt.dict_key("Resources");
    if (is_valid(resource_dict))
    {
        fmt.space().ref(resource_dict);
    }
    else
    {
        fmt.dict_start().dict_end();
    }
}


}} //namespace jag::pdf

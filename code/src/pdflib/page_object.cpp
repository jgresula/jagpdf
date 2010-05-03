// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//



#include "precompiled.h"
#include "page_object.h"
#include "objfmt.h"
#include "resourcelist.h"
#include "docwriterimpl.h"
#include "contentstream.h"
#include "resource_dictionary.h"
#include "annotationimpl.h"
#include "destination.h"
#include <core/errlib/errlib.h>
#include <core/jstd/tracer.h>
#include <core/generic/floatpointtools.h>
#include <core/generic/containerhelpers.h>
#include <interfaces/execcontext.h>
#include <interfaces/configinternal.h>


using namespace boost;
using namespace jag::jstd;

namespace jag {
namespace pdf {


//////////////////////////////////////////////////////////////////////////
PageObject::PageObject(DocWriterImpl& doc)
    : TreeNodeImpl(doc)
{
    set_invalid_double(m_dimension[0]);
}


//////////////////////////////////////////////////////////////////////////
bool PageObject::on_before_output_definition()
{
    JAG_ASSERT(!is_invalid_double(m_dimension[0]));

    if (m_content_streams.size())
    {
        ResourceListPtr const& res_list(m_content_streams[0].second);
        double page_height = doc().is_topdown() ? m_dimension[1] : 0.0;
        m_resource_dictionary_ref =
            output_resource_dictionary(doc(), res_list, page_height);
    }

    // output annotations
    if (!m_annotations.empty())
    {
        TRACE_DETAIL << "Writing indirect annotations.";
        JAG_ASSERT(m_annotation_refs.empty());
        m_annotation_refs.reserve(m_annotations.size());
        for(int i=static_cast<int>(m_annotations.size()); i--;)
        {
            m_annotation_refs.push_back(IndirectObjectRef(m_annotations[i]));
            m_annotations[i].output_definition();
        }
        m_annotations.clear();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
void PageObject::on_output_definition()
{
    ObjFmt& writer = object_writer();
    writer
        .dict_start()
        .dict_key("Type").output("Page")
        .dict_key("Parent").space().ref(*parent())
        .dict_key("MediaBox")
        .array_start()
            .output(0).space()
            .output(0).space()
            .output(m_dimension[0]).space()
            .output(m_dimension[1])
        .array_end()
    ;

    if (!m_content_streams.empty())
    {
        writer.dict_key("Contents").space().ref(m_content_streams[0].first);
        //drop content
        ContenStreamVec().swap(m_content_streams);
    }

    output_resource_dictionary_ref(m_resource_dictionary_ref, writer);

    if (const size_t num_annotations = m_annotation_refs.size())
    {
        TRACE_INFO << "Writing annotations.";
        writer
            .dict_key("Annots")
            .ref_array(address_of(m_annotation_refs), num_annotations);
    }

    writer.dict_end();
}

/**
 * @brief adds a content stream to this page
 *
 * content streams are outputted in the same order as they
 * were added
 *
 * @param content_stream content stream reference
 * @param resource_list list of resources used in the content stream
 */
void PageObject::add_content_stream(
      IndirectObjectRef const& content_stream
    , shared_ptr<ResourceList> const& resource_list
)
{
    m_content_streams.push_back(
        ContentStreamRec(content_stream, resource_list)
   );
}


//////////////////////////////////////////////////////////////////////////
void PageObject::set_dimension(Double width, double height)
{
    m_dimension[0] = width;
    m_dimension[1] = height;
}


template<class AW, class WP>
void PageObject::create_annotation(
    Double x, Double y, Double width, Double height,
    Char const* style, WP& worker_param)
{
    std::auto_ptr<IAnnotationType> ann_worker(
        new AW(worker_param));

    if (doc().is_topdown())
        y = m_dimension[1] - y - height;

    std::auto_ptr<AnnotationImpl> ann(
        new AnnotationImpl(doc(),
                            x, y, width, height, style, ann_worker));

    m_annotations.push_back(ann.release());
}


void PageObject::annotation_uri(
    Double x, Double y, Double width, Double height,
    Char const* uri,
    Char const* style)
{
    if (x < 0 || y < 0)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    create_annotation<AnnotationURI>(x, y, width, height, style, uri);
}


void PageObject::annotation_goto(
    Double x, Double y, Double width, Double height,
    Char const* destination, Char const* style)
{
    std::auto_ptr<DestinationDef> dest(new DestinationDef(doc(), destination));
    create_annotation<AnnotationGoto>(
        x, y, width, height, style, dest);
}


void PageObject::annotation_goto_obj(
    Double x, Double y, Double width, Double height,
    Destination id, Char const* style)
{
    IndirectObjectRef dest(doc().destination_ref(id));
    create_annotation<AnnotationIndirectGoto>(
        x, y, width, height,
        style, dest);
}


//
//
//
ICanvas* PageObject::canvas()
{
    // topdown & multiple canvases (future feature)
    //  
    //  It must be ensured that the first operation in the first canvas is the
    //  transformation to the topdown mode.
    //  
    //  - if the first canvas is the one created by this function than the
    //    operation is specified here
    //    
    //  - if the first canvas is supplied by the client (and thus without the
    //    topdown transformation), then a special purpose canvas consisting only
    //    of the transformation must be inserted at postion 0
    if (!m_canvas)
    {
        m_canvas.reset(doc().create_canvas_impl().release());
        if (doc().is_topdown())
            m_canvas->transform(1, 0, 0, -1, 0, m_dimension[1]);
    }

    // REF
    return m_canvas.get();
}

//
//
//
void PageObject::page_end()
{
    if (m_canvas && ! m_canvas->content_stream().is_empty())
    {
        m_canvas->output_definition();
        add_content_stream(
            IndirectObjectRef(m_canvas->content_stream()),
            m_canvas->resource_list());

        m_canvas.reset();
    }
}


}} //namespace jag::pdf

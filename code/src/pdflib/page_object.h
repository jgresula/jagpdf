// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//



#ifndef __PAGE_OBJECT_H__2721839
#define __PAGE_OBJECT_H__2721839

#include "indirectobjectimpl.h"
#include "indirectobjectref.h"
#include "treenodeimpl.h"
#include "annotationimpl.h"
#include "canvasimpl.h"
#include <pdflib/interfaces/pdfpage.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>

namespace jag {
namespace pdf {

//fwd
class DocWriterImpl;
class ResourceDictionary;
class ResourceList;

/// leaf of page page tree, represents a page
class PageObject
    : public TreeNodeImpl
    , public IPage
{
public:
    typedef Double Dimension[2];

    DEFINE_VISITABLE
    explicit PageObject(DocWriterImpl& doc);
    ~PageObject()
    {
    };
    void add_annotations(ObjectRefs& annotations);
    void set_dimension(Double width, double height);
    Dimension const& dimension() const { return m_dimension; }
    void page_end();

public: //IPage
    void annotation_uri(Double x, Double y, Double width, Double height, Char const* uri, Char const* style);
    void annotation_goto(Double x, Double y, Double width, Double height, Char const* destination, Char const* style);
    void annotation_goto_obj(Double x, Double y, Double width, Double height, Destination id, Char const* style);
    ICanvas* canvas();


private: // IndirectObjectImpl
    bool on_before_output_definition();
    void on_output_definition();

private:
    void add_content_stream(
          IndirectObjectRef const& content_stream
        , boost::shared_ptr<ResourceList> const& resource_list
   );
    template<class AW, class WP>
    void create_annotation(
        Double x, Double y, Double width, Double height,
        Char const* style, WP& worker_param);


private:
    typedef boost::shared_ptr<ResourceList> ResourceListPtr;
    typedef std::pair<IndirectObjectRef,ResourceListPtr>    ContentStreamRec;
    typedef std::vector<ContentStreamRec> ContenStreamVec;
    std::vector<ContentStreamRec> m_content_streams;
    IndirectObjectRef    m_resource_dictionary_ref;
    Dimension m_dimension;

    // this is the current canvas, can be null
    boost::scoped_ptr<CanvasImpl> m_canvas;

    boost::ptr_vector<AnnotationImpl>   m_annotations;
    ObjectRefs                          m_annotation_refs;
};

// inline void intrusive_ptr_add_ref(PageObject* obj)
// {
//     static_cast<IPage*>(obj)->AddRef();
// }
//
// inline void intrusive_ptr_release(PageObject* obj)
// {
//     static_cast<IPage*>(obj)->Release();
// }


}} //namespace jag::pdf

#endif // __PAGE_OBJECT_H__2721839

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "annotationimpl.h"
#include "docwriterimpl.h"
#include "objfmt.h"
#include "destination.h"


namespace jag {
namespace pdf {

///////////////////////////////////////////////////////////////////////////
// class AnnotationImpl
AnnotationImpl::AnnotationImpl(
    DocWriterImpl& doc,
    Double x, Double y, Double width, Double height,
    Char const* /*style*/,
    std::auto_ptr<IAnnotationType> worker
)
    : IndirectObjectImpl(doc)
    , m_llx(x)
    , m_lly(y)
    , m_urx(x+width)
    , m_ury(y+height)
    , m_worker(worker.release())
{
    JAG_PRECONDITION(m_worker);
}


void AnnotationImpl::on_output_definition()
{
    ObjFmt& fmt(doc().object_writer());

    fmt.dict_start();
    fmt.dict_key("Type").output("Annot"); // optional
    fmt.dict_key("Rect").rectangle(m_llx, m_lly, m_urx, m_ury);

    // use Border instead of BS as it seems more compatible (e.g. the
    // Preview.app ignores BS)
    fmt.dict_key("Border");
    fmt
        .array_start()
        .output(0).space()
        .output(0).space()
        .output(0)
        .array_end();


    m_worker->output(fmt);
    fmt.dict_end();
}




///////////////////////////////////////////////////////////////////////////
// class AnnotationURI
AnnotationURI::AnnotationURI(Char const* uri)
    : m_uri(uri)
{
}

void AnnotationURI::output(ObjFmt& fmt)
{
    fmt.dict_key("Subtype").output("Link");
    fmt.dict_key("A")
        .dict_start()
        .dict_key("S").output("URI")
        .dict_key("URI").text_string(m_uri.c_str(), m_uri.size())
        .dict_end()
    ;
}

///////////////////////////////////////////////////////////////////////////
// class AnnotationIndirectGoto
AnnotationIndirectGoto::AnnotationIndirectGoto(IndirectObjectRef const& ref)
    : m_ref(ref)
{
}


void AnnotationIndirectGoto::output(ObjFmt& fmt)
{
    fmt.dict_key("Subtype").output("Link");
    fmt.dict_key("Dest").space().ref(m_ref);
}


///////////////////////////////////////////////////////////////////////////
// class AnnotationGoto
AnnotationGoto::AnnotationGoto(std::auto_ptr<DestinationDef> dest)
    : m_dest(dest.release())
{
}


void AnnotationGoto::output(ObjFmt& fmt)
{
    fmt.dict_key("Subtype").output("Link");
    fmt.dict_key("Dest");
    m_dest->output_object(fmt);
}




}} // namespace jag

/** EOF @file */

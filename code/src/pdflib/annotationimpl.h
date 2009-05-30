// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef ANNOTATIONIMPL_JG1240_H__
#define ANNOTATIONIMPL_JG1240_H__

#include "indirectobjectimpl.h"
#include "indirectobjectref.h"
#include <boost/scoped_ptr.hpp>
#include <memory>
#include <string>

namespace jag {
namespace pdf {

class DocWriterImpl;
class ObjFmt;
class IndirectObjectRef;
class DestinationDef;

/// Interface representing an annotation type.
class IAnnotationType
    : public noncopyable
{
public:
    virtual void output(ObjFmt& fmt) = 0;
    virtual ~IAnnotationType() {}
};


/// Represents an annotation
class AnnotationImpl
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE
    explicit AnnotationImpl(
        DocWriterImpl& doc,
        Double x, Double y, Double width, Double height,
        Char const* style,
        std::auto_ptr<IAnnotationType> worker);


private: // IndirectObjectImpl
    void on_output_definition();
//    bool on_before_output_definition();

private:
    Double m_llx;
    Double m_lly;
    Double m_urx;
    Double m_ury;
    boost::scoped_ptr<IAnnotationType> m_worker;
};



/// Reperesents a link annotation with a URI action.
class AnnotationURI
    : public IAnnotationType
{
public:
    explicit AnnotationURI(Char const* uri);
    void output(ObjFmt& fmt);

private:
    std::string  m_uri;
};


class AnnotationGoto
    : public IAnnotationType
{
public:
    explicit AnnotationGoto(std::auto_ptr<DestinationDef> dest);
    void output(ObjFmt& fmt);

private:
    boost::scoped_ptr<DestinationDef> m_dest;
};


class AnnotationIndirectGoto
    : public IAnnotationType
{
public:
    explicit AnnotationIndirectGoto(IndirectObjectRef const& ref);
    void output(ObjFmt& fmt);

private:
    IndirectObjectRef m_ref;
};


}} // namespace jag

#endif // ANNOTATIONIMPL_JG1240_H__
/** EOF @file */

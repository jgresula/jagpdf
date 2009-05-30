// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __INDIRECT_OBJECT_IMPL_H__1822145
#define __INDIRECT_OBJECT_IMPL_H__1822145

#include "interfaces/indirect_object.h"
#include "interfaces/visitor.h"
#include "interfaces/object_type.h"
#include <core/generic/macros.h>


namespace jag
{
class ISeqStreamOutput;

namespace pdf
{

class DocWriterImpl;
class ObjFmt;

/**
 * @brief Default implementation of IIndirectObject interface.
 *
 * The derivees have to implement on_output_definition() and can also
 * implement on_before_output_definition().
 */
class IndirectObjectImpl
    : public IIndirectObject
{
public: //IIndirectObject
    void output_definition();
    Int object_number() const;
    Int generation_number() const;
    ~IndirectObjectImpl() {};

protected:
    explicit IndirectObjectImpl(DocWriterImpl& pdf_body);
    ObjFmt& object_writer() const;

protected:
    /**
     * @brief Invoked when the IIndirectObject::output_definition() object is
     *        invoked on IIndirectObject interface.
     *
     * The derivee has to completely output itself to the Body.stream()
     */
    virtual void on_output_definition() = 0;

    /**
     * @brief Invoked right before on_output_definition is invoked
     *
     * This is needed in case the derivee needs to output some objects
     * just right before it outputs itself. This cannot be done within on_output_definition()
     * as IndirectObjectImpl flanks this call by writing prologue/epilogue to the stream
     *
     * @param  seq_stream the stream which is used for output
     *
     * @return false if the derivee does not want to write anything, true otherwise
     */
    virtual bool on_before_output_definition();

    //////////////////////////////////////////////////////////////////////////
    virtual ObjectType object_type() const {
        return PDFOBJ_UNKNOWN;
    }

protected:
    DocWriterImpl& doc() const;

private:
    DocWriterImpl&    m_doc;
    mutable Int     m_object_number;
#ifdef JAG_DEBUG
    bool            m_object_outputted;
#endif //JAG_DEBUG
};

}}  // namespace jag::pdf

#endif //__INDIRECT_OBJECT_IMPL_H__1822145

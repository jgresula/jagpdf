// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "indirectobjectimpl.h"
#include "docwriterimpl.h"
#include "objfmt.h"
#include <core/jstd/crt.h>
#include <interfaces/streams.h>
#include <core/generic/assert.h>

namespace jag { namespace pdf
{

//////////////////////////////////////////////////////////////////////////
IndirectObjectImpl::IndirectObjectImpl(DocWriterImpl& pdf_body)
    : m_doc(pdf_body)
    , m_object_number(-1)
#ifdef JAG_DEBUG
    , m_object_outputted(false)
#endif //JAG_DEBUG
{
}

//////////////////////////////////////////////////////////////////////////
void IndirectObjectImpl::output_definition()
{
#ifdef JAG_DEBUG
    JAG_ASSERT_MSG(!m_object_outputted, "object already outputted");
    m_object_outputted = true;
#endif //JAG_DEBUG

    if (on_before_output_definition())
    {
        ULong stream_offset = object_writer().object_start(object_type(), *this);
        on_output_definition();
        object_writer().object_end(object_type(), *this);

        // add object to body in order to be listed in cross ref table
        m_doc.add_indirect_object(object_number(), generation_number(), static_cast<Int>(stream_offset));
    }
    else
    {
        JAG_ASSERT_MSG(-1==m_object_number, "object referenced, but not outputted");
    }
}

//////////////////////////////////////////////////////////////////////////
Int IndirectObjectImpl::object_number() const
{
    if (-1 == m_object_number)
        m_object_number = m_doc.assign_next_object_number();

    return m_object_number;
}

//////////////////////////////////////////////////////////////////////////
Int IndirectObjectImpl::generation_number() const
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////
bool IndirectObjectImpl::on_before_output_definition()
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
DocWriterImpl& IndirectObjectImpl::doc() const
{
    return m_doc;
}

ObjFmt& IndirectObjectImpl::object_writer() const
{
    return m_doc.object_writer();
}

}}  // namespace jag::pdf

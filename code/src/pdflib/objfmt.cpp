// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "objfmt.h"
#include "indirectobjectref.h"
#include "interfaces/objfmt_report.h"
#include "interfaces/indirect_object.h"
#include <core/jstd/crt.h>
#include <core/jstd/streamhelpers.h>
#include <interfaces/streams.h>
#include <core/generic/macros.h>
#include <core/generic/assert.h>


using namespace jag::jstd;

namespace jag {
namespace pdf {

ObjFmt::ObjFmt(ISeqStreamOutput& stream, IObjFmtReport* report, UnicodeConverterStream&  utf8_to_16be)
    : m_stream(&stream )
    , m_fmt_basic(stream, utf8_to_16be)
    , m_report(report)
{
}

//////////////////////////////////////////////////////////////////////////
ObjFmt& ObjFmt::object_end(ObjectType type, IIndirectObject& obj)
{
    if (m_report)
        m_report->object_end(type, obj);

    jstd::WriteStringToSeqStream(*m_stream, "\nendobj\n\n");
    return *this;
}

//////////////////////////////////////////////////////////////////////////
ULong ObjFmt::object_start(ObjectType type, IIndirectObject& obj)
{
    if (m_report)
        m_report->object_start(type, obj);

    // store the offset, in the future that should be handled by
    // reporting to a superior object
    ULong stream_offset = m_stream->tell();

    // write prologue
    const int buffer_length = 30;
    char buffer[buffer_length];
    int writen = jstd::snprintf(
        buffer, buffer_length, "%d %d obj",
        obj.object_number(), obj.generation_number());
    m_stream->write(buffer, writen);
    return stream_offset;
}

/**
*  @brief Outputs a reference to an indirect object
*
*  @param iobject indirect object to output
*/
ObjFmt& ObjFmt::ref(IIndirectObject const& iobject)
{
    Char buffer[INDIRECT_OBJECT_REFERENCE_MAX_TEXT_SIZE];
    int written = make_ref(iobject.object_number(), iobject.generation_number(), buffer);
    m_stream->write(buffer, written);
    return *this;
}

//////////////////////////////////////////////////////////////////////////
ObjFmt& ObjFmt::ref(IndirectObjectRef const& object)
{
    JAG_PRECONDITION(is_valid(object));
    Char buffer[INDIRECT_OBJECT_REFERENCE_MAX_TEXT_SIZE];
    int written = make_ref(object.object_number(), object.generation_number(), buffer);
    m_stream->write(buffer, written);
    return *this;
}


ObjFmt& ObjFmt::ref_array(IndirectObjectRef const* objects, size_t num_objects )
{
    JAG_PRECONDITION(objects);
    JAG_PRECONDITION(num_objects);

    m_fmt_basic.array_start();
    const size_t last_index = num_objects-1;
    for(size_t i=0; i<last_index; ++i)
    {
        ref(objects[i]);
        m_fmt_basic.space();
    }
    ref(objects[last_index]);
    m_fmt_basic.array_end();

    return *this;
}

//////////////////////////////////////////////////////////////////////////
int make_ref(Int object_nr, Int generation_nr, char* buffer)
{
    JAG_ASSERT(object_nr>0 && generation_nr>=0);

    return jstd::snprintf(
        buffer, INDIRECT_OBJECT_REFERENCE_MAX_TEXT_SIZE, "%d %d R",
        object_nr,
        generation_nr);
}


}} //namespace jag::pdf

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "crossrefsection.h"
#include <core/jstd/crt.h>
#include <interfaces/streams.h>
#include <core/generic/assert.h>

namespace jag { namespace pdf
{

/// ctor
CrossReferenceSection::CrossReferenceSection()
    : m_stream_offset(0)
{
    m_objects.reserve(100);
    m_objects.push_back(Entry(65535, 0, 'f'));
}

/**
* @brief adds an indirect object
*
* @param object_number to be added
* @param generation_number generation number
* @param stream_offset position in the stream where the object is defined
*
*/
void CrossReferenceSection::add_indirect_object(
      Int object_number
    , Int generation_number
    , Int stream_offset
)
{
    JAG_ASSERT(object_number >= 0);

    if (object_number >= static_cast<Int>(m_objects.size()))
        m_objects.resize(object_number + 1);

    JAG_ASSERT(!m_objects[object_number].m_valid);

    m_objects[object_number] = Entry(generation_number, stream_offset);
}

/**
 * @brief outputs the crossreference section to the stream
 *
 * @param seq_stream output storage
 */
void CrossReferenceSection::output(ISeqStreamOutput& seq_stream)
{
    const int buffer_length = 40;
    Char buffer[buffer_length];

    JAG_ASSERT_MSG(!m_stream_offset, "we have been already here!");
    m_stream_offset = seq_stream.tell();

    // prologue
    int written = jstd::snprintf(buffer, buffer_length, "xref\n0 %d\n", m_objects.size());
    seq_stream.write(buffer, written);


#    ifdef JAG_DEBUG
    unsigned int dbg_object_nr = 0;
#    endif

    // objects
    for(Entries::const_iterator it = m_objects.begin();
        it != m_objects.end();
        ++it)
    {
    JAG_ASSERT_MSG(it->m_valid, "page object registered to cross-ref table, but not outputted");
    written = jstd::snprintf(buffer, buffer_length, "%010d %05d %c \n",
        it->m_stream_offset,
        it->m_generation_number,
        it->m_type);
    seq_stream.write(buffer, written);

#    ifdef JAG_DEBUG
    ++dbg_object_nr;
#    endif

    }
}


/**
* @return number of entries in the table
*/
int CrossReferenceSection::num_entries() const
{
    return static_cast<int>(m_objects.size());
}

/**
* @return offset in the stream
*/
UInt CrossReferenceSection::stream_offset() const
{
    JAG_ASSERT_MSG(m_stream_offset, "crossref table has not been written yet!");
    return static_cast<UInt>(m_stream_offset);
}

}} //namespace jag::pdf


// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/memory_stream.h>
#include <core/generic/assert.h>
#include <core/generic/minmax.h>
#include <string.h>

namespace jag {    namespace jstd
{

//////////////////////////////////////////////////////////////////////////
MemoryStreamOutput::MemoryStreamOutput()
    : m_bytes_written(0)
    , m_actual_size(0)
{
}

/**
 * @brief ensures that the stream has requested size
 *
 * @param size requested size
 */
void MemoryStreamOutput::EnsureSize(ULong size)
{
    // ensure that m_buffer has length at least 'size'
    if (size > m_actual_size)
    {
        m_actual_size =  std::max<ULong>(size, UInt(1.5* m_actual_size));
        boost::shared_array<Byte> new_buffer(new Byte[static_cast<unsigned int>(m_actual_size)]);
        memcpy(new_buffer.get(), m_buffer.get(), static_cast<unsigned int>(m_bytes_written));
        m_buffer.swap(new_buffer);
    }
}

//////////////////////////////////////////////////////////////////////////
void MemoryStreamOutput::write(void const* data, ULong size)
{
    EnsureSize(m_bytes_written + size);
    memcpy(m_buffer.get() + m_bytes_written, data, size);
    m_bytes_written += size;
}

//////////////////////////////////////////////////////////////////////////
ULong MemoryStreamOutput::tell() const
{
    return m_bytes_written;
}

//////////////////////////////////////////////////////////////////////////
Byte const* MemoryStreamOutput::data() const
{
    return m_buffer.get();
}

//////////////////////////////////////////////////////////////////////////
void MemoryStreamOutput::close()
{
    m_buffer.reset();
}

//////////////////////////////////////////////////////////////////////////
boost::shared_array</*const*/ Byte> MemoryStreamOutput::shared_data() const
{
    return m_buffer;
}



/**
 * @brief constructs input memory stream
 *
 * @param data memory to be exposed as a memory stream, the memory is not owned
 *             by this object
 * @param size size of the memory
 */
MemoryStreamInput::MemoryStreamInput(void const* data, ULong size, bool take_ownership)
    : m_data(data)
    , m_size(size)
    , m_position(0)
    , m_take_ownership(take_ownership)
{
}

//////////////////////////////////////////////////////////////////////////
bool MemoryStreamInput::read(void* data, ULong size, ULong* read)
{
    ULong copy = min<ULong>(size, m_size-m_position);
    memcpy(data, static_cast<Byte const*>(m_data) + m_position, copy);

    if (read)
        *read = copy;

    m_position += copy;

    return (m_position == m_size) ? false : true;
}

//////////////////////////////////////////////////////////////////////////
ULong MemoryStreamInput::tell() const
{
    return m_position;
}

//////////////////////////////////////////////////////////////////////////
void MemoryStreamInput::close()
{
    if (m_take_ownership)
    {
        delete [] static_cast<Byte const*>(m_data);
        m_data = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
MemoryStreamInput::~MemoryStreamInput()
{
    close();
}

//////////////////////////////////////////////////////////////////////////
void MemoryStreamInput::seek(Int offset, StreamOffsetOrigin origin)
{
    switch(origin)
    {
    case OFFSET_FROM_CURRENT:
        m_position += offset;
        break;

    case OFFSET_FROM_END:
        m_position = m_position+offset;
        break;

    case OFFSET_FROM_BEGINNING:
        m_position = offset;
        break;
    }

    if (m_position>m_size)
        m_position=m_size ? m_size-1 : 0;
}


/**
 * @brief Constructor.
 *
 * @param buffer buffer to use for writing
 * @param size length of the buffer
 */
MemoryStreamOutputFixedSize::MemoryStreamOutputFixedSize(void* buffer, size_t size)
    : m_buffer(static_cast<Byte*>(buffer))
    , m_size(size)
    , m_current_offset(0)
{
}

//////////////////////////////////////////////////////////////////////////
void MemoryStreamOutputFixedSize::write(void const* data, ULong size)
{
    JAG_ASSERT(size+m_current_offset <= m_size && "buffer overrun");
    memcpy(m_buffer, data, size);
    m_current_offset += size;
}




//////////////////////////////////////////////////////////////////////////
MemoryStreamInputFromOutput::MemoryStreamInputFromOutput(
    boost::shared_ptr<MemoryStreamOutput> mem_out
)
    : m_mem_out(mem_out)
    , m_in(mem_out->data(), static_cast<UInt>(mem_out->tell()))
{}


}} //namespace jag::jstd

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __MEMORY_STREAM_H__1122159
#define __MEMORY_STREAM_H__1122159

#include <interfaces/streams.h>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#include <sstream>
#include <string>

namespace jag {
namespace jstd {

/// memory based implementation of ISeqStreamOutput
class MemoryStreamOutput
    : public ISeqStreamOutputControl
{
public:
    MemoryStreamOutput();
    /// retrieves actual stream content (continuous memory)
    Byte const* data() const;
    boost::shared_array</*const*/ Byte> shared_data() const;
    void close();

public:  //ISeqStreamOutput
    void write(void const* data, ULong size);
    ULong tell() const;
    void flush() {};

private:
    void EnsureSize(ULong size);

    ULong m_bytes_written;
    ULong m_actual_size;
    boost::shared_array<Byte> m_buffer;
};



/**
 * @brief Memory based implementation of ISeqStreamOutput with a fixed size buffer.
 *
 * If the amount of the written data exceeds the size of the buffer then it results
 * to undefined behavior (debug build asserts on this).
 */
class MemoryStreamOutputFixedSize
    : public ISeqStreamOutputControl
{
public:
    MemoryStreamOutputFixedSize(void* buffer, size_t size);
    /// retrieves actual stream content (continuous memory)
    Byte const* data() const { return m_buffer; }
    void close() {}

public:  //ISeqStreamOutput
    void write(void const* data, ULong size);
    ULong tell() const { return m_current_offset; }
    void flush() {};

private:
    Byte *const  m_buffer;
    size_t const m_size;
    size_t       m_current_offset;
};


/// memory based implementation of ISeqStreamInput
class MemoryStreamInput
    : public IStreamInput
{
public:
    MemoryStreamInput(void const* data, ULong size, bool take_ownership=false);
    ~MemoryStreamInput();

public: // ISeqStreamInput
    bool read(void* data, ULong size, ULong* read);
    ULong tell() const;
    void close();
    void seek(Int offset, StreamOffsetOrigin origin);

private:
    void const* m_data;
    ULong       m_size;
    ULong       m_position;
    bool        m_take_ownership;
};



//////////////////////////////////////////////////////////////////////////
class MemoryStreamInputFromOutput
    : public IStreamInput
{
    boost::shared_ptr<MemoryStreamOutput> m_mem_out;
    MemoryStreamInput    m_in;
public:
    MemoryStreamInputFromOutput(boost::shared_ptr<MemoryStreamOutput> mem_out);

public: // ISeqStreamInput
    bool read(void* data, ULong size, ULong* read) {
        return m_in.read(data, size, read);
    }

    ULong tell() const{
        return m_in.tell();
    }

    void close() {
        m_in.close();
    }

    void seek(Int offset, StreamOffsetOrigin origin) {
        m_in.seek(offset, origin);
    }
};

}} //namespace jag::jstd

#endif //__MEMORY_STREAM_H__1122159

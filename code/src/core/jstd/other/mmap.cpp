// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/mmap.h>
#include <core/errlib/errlib.h>
#include <core/generic/assert.h>
#include <interfaces/streams.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


namespace jag {
namespace jstd {

//
//
//
MMapFileStreamOutput::MMapFileStreamOutput(char const* fname, int view_size)
    : m_fname(fname)
    , m_base_ptr(0)
    , m_offset(0)
    , m_alloc_offset(0)
{
    m_hfile = ::open(fname, O_RDWR|O_CREAT|O_TRUNC /* cygwin? |O_BINARY*/, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (m_hfile == -1)
    {
        throw exception_io_error(msg_cannot_open_file())
            << errno_info(errno)
            << io_object_info(fname)
            << JAGLOC;
    }

    m_alloc_granularity = getpagesize();
    m_alloc_granularity = int((m_alloc_granularity+view_size-1)/m_alloc_granularity)*m_alloc_granularity;

    remap_core(m_alloc_granularity, 0);
}


//
//
//
void MMapFileStreamOutput::remap_core(size_t size, off_t offset)
{
    if (-1 == ::lseek(m_hfile, size-1, SEEK_SET))
    {
        throw exception_io_error(msg_cannot_seek_file())
            << errno_info(errno)
            << io_object_info(m_fname.c_str())
            << JAGLOC;
    }

    if (1 != ::write(m_hfile, "", 1))
    {
        throw exception_io_error(msg_cannot_write_to_file())
            << errno_info(errno)
            << io_object_info(m_fname.c_str())
            << JAGLOC;
    }

    m_base_ptr = static_cast<Byte*>(mmap(0,
                                          size,
                                          PROT_READ|PROT_WRITE,
                                          MAP_SHARED,
                                          m_hfile,
                                          offset));
    if (m_base_ptr == MAP_FAILED)
    {
        throw exception_io_error(msg_cannot_mmap_file())
            << errno_info(errno)
            << io_object_info(m_fname.c_str())
            << JAGLOC;
    }
}

//
//
//
void MMapFileStreamOutput::remap()
{
    assert(m_base_ptr);

    if (-1 == ::munmap(m_base_ptr, current_view_size()))
    {
        throw exception_io_error(msg_cannot_unmmap_file())
            << errno_info(errno)
            << io_object_info(m_fname.c_str())
            << JAGLOC;
    }

    m_alloc_offset += m_alloc_granularity;
    remap_core(m_alloc_offset+m_alloc_granularity, m_alloc_offset);
    m_offset = 0;
}


//
//
//
void MMapFileStreamOutput::write(void const *data, ULong size)
{
    UInt data_offset = 0;
    while(data_offset != size)
    {
        UInt nr_can_copy_bytes = current_view_size()-m_offset;
        if (size <= nr_can_copy_bytes)
        {
            memcpy(m_base_ptr+m_offset, static_cast<Byte const*>(data)+data_offset, size);
            m_offset += size;
            break;
        }
        else
        {
            memcpy(m_base_ptr+m_offset, static_cast<Byte const*>(data)+data_offset, nr_can_copy_bytes);
            data_offset += nr_can_copy_bytes;
            size -= nr_can_copy_bytes;
            remap();
        }
    }
}


//
//
//
void MMapFileStreamOutput::flush()
{
    // ???
}


//
//
//
void MMapFileStreamOutput::close()
{
    if (m_base_ptr)
    {
        JAG_PRECONDITION(m_hfile != -1);

        if (-1 == ::munmap(m_base_ptr, current_view_size()))
        {
            throw exception_io_error(msg_cannot_unmmap_file())
                << errno_info(errno)
                << io_object_info(m_fname.c_str())
                << JAGLOC;
        }
        m_base_ptr = 0;

        if (-1 == ::ftruncate(m_hfile, m_alloc_offset + m_offset))
        {
            throw exception_io_error(msg_cannot_seek_file())
                << errno_info(errno)
                << io_object_info(m_fname.c_str())
                << JAGLOC;
        }

        if (-1 == ::close(m_hfile))
        {
            throw exception_io_error(msg_cannot_close_file())
                << errno_info(errno)
                << io_object_info(m_fname.c_str())
                << JAGLOC;
        }
        m_hfile = 0;
    }
}


//
//
//
MMapFileStreamOutput::~MMapFileStreamOutput()
{
    try
    {
        close();
    }
    catch(...)
    {
        JAG_ASSERT(!"error during close()");
    }
}


}} // namespace jag::jstd

/** EOF @file */

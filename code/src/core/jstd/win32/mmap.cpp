// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/mmap.h>
#include <core/jstd/conversions.h>
#include <core/errlib/errlib.h>
#include <core/generic/bitutils.h>
#include <core/generic/assert.h>
#include <interfaces/streams.h>

namespace jag {
namespace jstd {

//
//
//
MMapFileStreamOutput::MMapFileStreamOutput(char const* fname, int view_size)
    : m_fname(fname)
    , m_hmmfile(0)
    , m_base_ptr(0)
    , m_offset(0)
    , m_alloc_offset(0)
{
    m_hfile = ::CreateFileW(FromUTF8(fname).to_utf16(),
                             GENERIC_WRITE|GENERIC_READ,
                             NULL,
                             0,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

    if (m_hfile == INVALID_HANDLE_VALUE)
    {
        throw exception_io_error(msg_cannot_open_file())
            << win_error_info(::GetLastError())
            << io_object_info(fname)
            << JAGLOC;
    }

    SYSTEM_INFO sys_info;
    ::GetSystemInfo(&sys_info);
    m_alloc_granularity = sys_info.dwAllocationGranularity;
    m_alloc_granularity = int((m_alloc_granularity+view_size-1)/m_alloc_granularity)*m_alloc_granularity;
    remap_core(m_alloc_granularity, 0);
}


//
//
//
void MMapFileStreamOutput::remap_core(UInt64 size, UInt64 offset)
{
    m_hmmfile = ::CreateFileMapping(m_hfile,
                                     NULL,
                                     PAGE_READWRITE,
                                     high32bits<>(size),
                                     low32bits<>(size),
                                     NULL);
    if (!m_hmmfile)
    {
        throw exception_io_error(msg_cannot_mmap_file())
            << win_error_info(::GetLastError())
            << io_object_info(m_fname.c_str())
            << JAGLOC;
    }

    m_base_ptr = static_cast<Byte*>(::MapViewOfFile(
                                        m_hmmfile,
                                        FILE_MAP_WRITE,
                                        high32bits<>(offset),
                                        low32bits<>(offset),
                                        0));

    if (!m_base_ptr)
    {
        throw exception_io_error(msg_cannot_mmap_file())
            << win_error_info(::GetLastError())
            << io_object_info(m_fname.c_str())
            << JAGLOC;
    }
}

//
//
//
void MMapFileStreamOutput::remap()
{
    JAG_PRECONDITION(m_base_ptr);
    JAG_PRECONDITION(m_hmmfile);

    if (!::UnmapViewOfFile(m_base_ptr)
         || !::CloseHandle(m_hmmfile))
    {
        throw exception_io_error(msg_cannot_unmmap_file())
            << win_error_info(::GetLastError())
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
    ULong data_offset = 0;
    while(data_offset != size)
    {
        ULong nr_can_copy_bytes = current_view_size()-m_offset;
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
    JAG_PRECONDITION(m_base_ptr);

    if (!::FlushViewOfFile(m_base_ptr, 0))
    {
        throw exception_io_error(msg_cannot_flush_file())
            << win_error_info(::GetLastError())
            << io_object_info(m_fname.c_str())
            << JAGLOC;
    }
}


//
//
//
void MMapFileStreamOutput::close()
{
    if (m_base_ptr)
    {
        JAG_PRECONDITION(m_hmmfile);
        JAG_PRECONDITION(m_hfile != INVALID_HANDLE_VALUE);

        if (!::UnmapViewOfFile(m_base_ptr)
             || !::CloseHandle(m_hmmfile))
        {
            throw exception_io_error(msg_cannot_unmmap_file())
                << win_error_info(::GetLastError())
                << io_object_info(m_fname.c_str())
                << JAGLOC;
        }
        m_base_ptr = 0;
        m_hmmfile = 0;

        UInt64 file_size = m_alloc_offset + m_offset;
        LONG move_high32 = high32bits(file_size);
        if (INVALID_SET_FILE_POINTER == ::SetFilePointer(m_hfile, low32bits(file_size), &move_high32, FILE_BEGIN)
            || !::SetEndOfFile(m_hfile))
        {
            throw exception_io_error(msg_cannot_seek_file())
                << win_error_info(::GetLastError())
                << io_object_info(m_fname.c_str())
                << JAGLOC;
        }

        if (!::CloseHandle(m_hfile))
        {
            throw exception_io_error(msg_cannot_close_file())
                << win_error_info(::GetLastError())
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

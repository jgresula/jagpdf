// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/fileso.h>
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>
#include <errno.h>
#include <sys/stat.h>

namespace jag {
namespace jstd {

//////////////////////////////////////////////////////////////////////////
File::File()
{
    m_handle = 0;
}

//////////////////////////////////////////////////////////////////////////
File::~File()
{
    try
    {
        if (m_handle)
        {
            close();
        }
    }
    catch (exception&)
    {
        // cannot do much here ...
    }
}


//////////////////////////////////////////////////////////////////////////
void File::create(
    Char const* file_name,
    UInt open_mode,
    UInt /*sharing*/
)
{
    JAG_PRECONDITION(!m_handle);

    m_name = file_name;

    char const* mode = 0;
    switch(open_mode)
    {
    case READ:
        mode = "rb";
        break;

    case READ | WRITE:
        mode = "r+b";
        break;

    case WRITE | TRUNCATE:
        mode = "wb";
        break;

    case READ | WRITE | TRUNCATE:
        mode = "w+b";
        break;

    case WRITE | APPEND:
        mode = "ab";
        break;

    case READ | WRITE | APPEND:
        mode = "a+b";
        break;

    default:
        throw exception_invalid_value(msg_invalid_openning_mode())
            << JAGLOC
            << io_object_info( m_name.c_str());
    }

    m_handle = fopen(file_name, mode);
    if (!m_handle )
    {
        throw exception_io_error(msg_cannot_open_file())
            << errno_info(errno)
            << io_object_info(m_name.c_str())
            << JAGLOC;
    }
}

//////////////////////////////////////////////////////////////////////////
void File::write(void const* data, ULong bytes) const
{
    JAG_PRECONDITION(m_handle);
    if (bytes)
    {
        UInt written = fwrite(data, 1, bytes, m_handle);
        if (written != bytes)
        {
            throw exception_io_error(msg_cannot_write_to_file())
                << JAGLOC
                << io_object_info(m_name.c_str())
                << errno_info(errno);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
bool File::read(void* data, ULong size, ULong* nr_read) const
{
    JAG_PRECONDITION(m_handle);
    UInt local_read = fread(data, 1, size, m_handle);

    if (ferror(m_handle))
    {
        throw exception_io_error(msg_cannot_read_from_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << errno_info(errno);
    }

    if (nr_read)
        *nr_read = local_read;

    return !feof(m_handle);
}

//////////////////////////////////////////////////////////////////////////
ULong File::tell() const
{
    JAG_PRECONDITION(m_handle);

    long offset = ftell(m_handle);
    if (offset == -1)
    {
        throw exception_io_error(msg_cannot_get_file_offset())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << errno_info(errno);
    }

    return offset;
}

//////////////////////////////////////////////////////////////////////////
void File::close()
{
    JAG_PRECONDITION(m_handle);

    if (fclose(m_handle))
    {
        m_handle = 0;
        throw exception_io_error(msg_cannot_close_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << errno_info(errno);
    }

    m_handle = 0;
}

//////////////////////////////////////////////////////////////////////////
void File::flush()
{
    JAG_PRECONDITION(m_handle);

    if (fflush(m_handle))
    {
        throw exception_io_error(msg_cannot_flush_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << errno_info(errno);
    }
}

//////////////////////////////////////////////////////////////////////////
void File::seek(Int offset, StreamOffsetOrigin origin)
{
    JAG_PRECONDITION(m_handle);

    int whence=0;
    switch(origin)
    {
    case OFFSET_FROM_CURRENT:
        whence = SEEK_CUR;
        break;

    case OFFSET_FROM_END:
        whence = SEEK_END;
        break;

    case OFFSET_FROM_BEGINNING:
        whence = SEEK_SET;
        break;
    }

    if (fseek(m_handle, offset, whence))
    {
        throw exception_io_error(msg_cannot_seek_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << errno_info(errno);
    }
}


//
//
// 
bool file_exists(Char const* path)
{
    struct stat path_stat;
    return (!::stat(path, &path_stat));
}


//
//
// 
bool is_directory(Char const* path)
{
    struct stat path_stat;
    if (!::stat(path, &path_stat))
    {
        return S_ISDIR(path_stat.st_mode);
    }

    return false;
}

//
//
// 
uintmax_t file_size(Char const* path)
{
    struct stat path_stat;
    if (::stat(path, &path_stat) || !S_ISREG(path_stat.st_mode))
        throw exception_io_error(msg_cannot_get_filesize())
            << JAGLOC
            << io_object_info(path)
            << errno_info(errno);

    return static_cast<uintmax_t>(path_stat.st_size);
}


  

}} // namespace jag::jstd


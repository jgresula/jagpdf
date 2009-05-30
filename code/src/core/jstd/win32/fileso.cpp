// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/win32/win32common.h>
#include <core/jstd/fileso.h>
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>
#include <core/jstd/conversions.h>


namespace jag {
namespace jstd {

//////////////////////////////////////////////////////////////////////////
File::File()
{
    m_handle = INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////
File::~File()
{
    try
    {
        if (m_handle != INVALID_HANDLE_VALUE)
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
    UInt sharing
)
{
    JAG_PRECONDITION(m_handle==INVALID_HANDLE_VALUE);

    m_name = file_name;

    DWORD desired_access;
    DWORD creation_disposition;
    bool append = false;

    switch(open_mode)
    {
    case READ:
        desired_access = GENERIC_READ;
        creation_disposition = OPEN_EXISTING;
        break;

    case READ | WRITE:
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = OPEN_EXISTING;
        break;

    case WRITE | TRUNCATE:
        desired_access = GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
        break;

    case READ | WRITE | TRUNCATE:
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
        break;

    case WRITE | APPEND:
        desired_access = GENERIC_WRITE;
        creation_disposition = OPEN_ALWAYS;
        append = true;
        break;

    case READ | WRITE | APPEND:
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = OPEN_ALWAYS;
        append = true;
        break;

    default:
        throw exception_invalid_value(msg_invalid_openning_mode())
            << JAGLOC
            << io_object_info(m_name.c_str());
    }


    DWORD share_mode = 0;
    share_mode |= (sharing == SHARE_READ) ? FILE_SHARE_READ : 0;
    share_mode |= (sharing == SHARE_WRITE) ? FILE_SHARE_WRITE : 0;

    m_handle = ::CreateFileW(FromUTF8(file_name).to_utf16(), desired_access, share_mode, NULL,
        creation_disposition, FILE_ATTRIBUTE_NORMAL, 0);

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        throw exception_io_error(msg_cannot_open_file())
            << win_error_info(::GetLastError())
            << io_object_info(m_name.c_str())
            << JAGLOC;
    }

    //TBD: the following code (i.e. seek functionality) should be moved to a separate method
    if (append)
    {
        if (INVALID_SET_FILE_POINTER == ::SetFilePointer(m_handle, 0, 0, FILE_END))
        {
            throw exception_io_error(msg_cannot_seek_file())
                << JAGLOC
                << io_object_info(m_name.c_str())
                << win_error_info(::GetLastError());
        }
    }
}

//////////////////////////////////////////////////////////////////////////
bool File::read(void* data, ULong size, ULong* nr_read) const
{
    JAG_PRECONDITION(m_handle != INVALID_HANDLE_VALUE);
    JAG_PRECONDITION(size<=0xffffffffu);

    DWORD local_read;
    if (!::ReadFile(m_handle, data, static_cast<DWORD>(size), &local_read, 0))
    {
        throw exception_io_error(msg_cannot_read_from_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << win_error_info(::GetLastError());
    }

    if (nr_read)
        *nr_read = local_read;

    return local_read ? true : false;
}

//////////////////////////////////////////////////////////////////////////
void File::write(void const* data, ULong bytes) const
{
    JAG_PRECONDITION(m_handle != INVALID_HANDLE_VALUE);
    JAG_PRECONDITION(bytes<=0xffffffffu);

    ULong written = 0;
    if (!::WriteFile(m_handle, data, static_cast<DWORD>(bytes), &written, 0))
    {
        throw exception_io_error(msg_cannot_write_to_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << win_error_info(::GetLastError());
    }
}

//////////////////////////////////////////////////////////////////////////
ULong File::tell() const
{
    JAG_PRECONDITION(m_handle != INVALID_HANDLE_VALUE);
    return  ::SetFilePointer(m_handle, 0, 0, FILE_CURRENT);
}

void File::flush()
{
    JAG_PRECONDITION(m_handle != INVALID_HANDLE_VALUE);
    if (! ::FlushFileBuffers(m_handle))
    {
        throw exception_io_error(msg_cannot_flush_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << win_error_info(::GetLastError());
    }
}

//////////////////////////////////////////////////////////////////////////
void File::close()
{
    JAG_PRECONDITION(m_handle != INVALID_HANDLE_VALUE);
    if (!::CloseHandle(m_handle))
    {
        m_handle = INVALID_HANDLE_VALUE;
        throw exception_io_error(msg_cannot_close_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << win_error_info(::GetLastError());
    }
    m_handle = INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////
void File::seek(Int offset, StreamOffsetOrigin origin)
{
    JAG_PRECONDITION(m_handle != INVALID_HANDLE_VALUE);

    DWORD move_method=0;
    switch(origin)
    {
    case OFFSET_FROM_CURRENT:
        move_method = FILE_CURRENT;
        break;

    case OFFSET_FROM_END:
        move_method = FILE_END;
        break;

    case OFFSET_FROM_BEGINNING:
        move_method = FILE_BEGIN;
        break;

    default:
        JAG_PRECONDITION(!"invalid origin");
    }

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(m_handle, offset, 0, move_method))
    {
        throw exception_io_error(msg_cannot_seek_file())
            << JAGLOC
            << io_object_info(m_name.c_str())
            << win_error_info(::GetLastError());
    }
}


//
//
//
bool file_exists(Char const* path)
{
    DWORD attr = ::GetFileAttributesW(FromUTF8(path).to_utf16());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        DWORD err = ::GetLastError();
        if (err != ERROR_SHARING_VIOLATION)
            return false;
    }
    return true;
}


//
//
//
bool is_directory(Char const* path)
{
    DWORD attr = ::GetFileAttributesW(FromUTF8(path).to_utf16());
    if (attr == INVALID_FILE_ATTRIBUTES)
        return false;

    return attr & FILE_ATTRIBUTE_DIRECTORY;
}

//
//
//
boost::uintmax_t file_size(Char const* path)
{
    WIN32_FILE_ATTRIBUTE_DATA finfo;
    if (!::GetFileAttributesExW(FromUTF8(path).to_utf16(),
                                ::GetFileExInfoStandard,
                                &finfo) ||
        (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        throw exception_io_error(msg_cannot_get_filesize())
            << JAGLOC
            << io_object_info(path)
            << win_error_info(::GetLastError());
    }

    return (static_cast<boost::uintmax_t>(finfo.nFileSizeHigh)
        << (sizeof(finfo.nFileSizeLow)*8))
        + finfo.nFileSizeLow;
}


  

}} // namespace jag::jstd

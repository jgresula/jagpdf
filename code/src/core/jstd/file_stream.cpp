// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/file_stream.h>
#include <core/generic/assert.h>

namespace jag { namespace jstd
{

//////////////////////////////////////////////////////////////////////////
FileStreamOutput::FileStreamOutput(char const* file_name)
    : m_closed(false)
{
    m_file.create(file_name, jstd::File::WRITE | jstd::File::TRUNCATE);
}

//////////////////////////////////////////////////////////////////////////
FileStreamOutput::~FileStreamOutput()
{
    if (!m_closed)
        close();
}

//////////////////////////////////////////////////////////////////////////
void FileStreamOutput::write(void const* data, ULong size)
{
    JAG_PRECONDITION(!m_closed);
    m_file.write(data, size);
}

//////////////////////////////////////////////////////////////////////////
ULong FileStreamOutput::tell() const
{
    JAG_PRECONDITION(!m_closed);
    return m_file.tell();
}

//////////////////////////////////////////////////////////////////////////
void FileStreamOutput::close()
{
    m_file.close();
    m_closed = true;
}

//////////////////////////////////////////////////////////////////////////
void FileStreamOutput::flush()
{
    JAG_PRECONDITION(!m_closed);
    m_file.flush();
}




//////////////////////////////////////////////////////////////////////////
FileStreamInput::FileStreamInput(Char const* file_name)
    : m_closed(false)
{
    m_file.create(file_name, jstd::File::READ);
}

//////////////////////////////////////////////////////////////////////////
FileStreamInput::~FileStreamInput()
{
    if (!m_closed)
        close();
}

//////////////////////////////////////////////////////////////////////////
bool FileStreamInput::read(void* data, ULong size, ULong* read)
{
    JAG_PRECONDITION(!m_closed);
    return m_file.read(data, size, read);
}

//////////////////////////////////////////////////////////////////////////
ULong FileStreamInput::tell() const
{
    JAG_PRECONDITION(!m_closed);
    return m_file.tell();
}

//////////////////////////////////////////////////////////////////////////
void FileStreamInput::close()
{
    JAG_PRECONDITION(!m_closed);
    m_file.close();
    m_closed = true;
}

void FileStreamInput::seek(Int offset, StreamOffsetOrigin origin)
{
    JAG_PRECONDITION(!m_closed);
    m_file.seek(offset, origin);
}


}} //namespace jag::jstd

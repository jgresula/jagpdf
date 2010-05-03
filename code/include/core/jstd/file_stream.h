// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FILE_STREAM__
#define __FILE_STREAM__

#include <interfaces/streams.h>
#include <core/jstd/fileso.h>

namespace jag { namespace jstd
{


/// file based implementation of ISeqStreamOutput
class FileStreamOutput
    : public ISeqStreamOutputControl
{
public:
    /**
     * @brief creates a file based instance of ISeqStreamOutput implementation
     *
     * @param file_name path to a underlying file
     */
    FileStreamOutput(char const* file_name);
    ~FileStreamOutput();

public:  //ISeqStream
    void write(void const* data, ULong size);
    ULong tell() const;
    void flush();

public:
    void close();

private:
    jstd::File m_file;
    bool m_closed;
};


/// file based implementation of ISeqStreamInput
class FileStreamInput
    : public IStreamInput
{
public:
    FileStreamInput(Char const* file_name);
    ~FileStreamInput();

public: //ISeqStreamInputControl
    bool read(void* data, ULong size, ULong* read = 0);
    ULong tell() const;
    void seek(Int offset, StreamOffsetOrigin origin);

public:
    void close();

private:
    jstd::File m_file;
    bool m_closed;
};

}} //namespace jag::jstd

#endif //__FILE_STREAM__

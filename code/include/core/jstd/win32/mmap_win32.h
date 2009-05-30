// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef MMAP_JG148_H__
#define MMAP_JG148_H__

#include <interfaces/stdtypes.h>
#include <interfaces/streams.h>
#include <string>
#include <windows.h>

namespace jag {
namespace jstd {

class MMapFileStreamOutput
    : public ISeqStreamOutputControl
{
public:
    MMapFileStreamOutput(char const* fname, int view_size=JAG_MMAP_VIEW_DEFAULT_SIZE);
    ~MMapFileStreamOutput();

public: // ISeqStreamOutput
    void write(void const *data, ULong size);
    ULong tell() const { return m_alloc_offset+m_offset; }
    void flush();
    void close();

private:
    void remap();
    void remap_core(UInt64 size, UInt64 offset);
    int current_view_size() const { return m_alloc_granularity; }

private:
    std::string m_fname;
    HANDLE m_hfile;               // handle to underlying file
    HANDLE m_hmmfile;             // handle to the current file mapping
    Byte*  m_base_ptr;            // base ptr for the current mapping
    ULong  m_offset;              // current offset from the base_ptr
    DWORD  m_alloc_granularity;   // allocation granularity - size of the current view
    ULong  m_alloc_offset;        // offset of the current mapping
};

}} // namespace jag::jstd

#endif // MMAP_JG148_H__
/** EOF @file */

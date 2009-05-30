// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef MMAP_JG147_H__
#define MMAP_JG147_H__

#include <interfaces/stdtypes.h>
#include <interfaces/streams.h>
#include <sys/types.h>
#include <string>

namespace jag {
namespace jstd {

class MMapFileStreamOutput
    : public ISeqStreamOutputControl
{
public:
    MMapFileStreamOutput(char const* fname, int view_size=JAG_MMAP_VIEW_DEFAULT_SIZE);
    ~MMapFileStreamOutput();

public: // ISeqStreamOutputControl
    void write(void const *data, ULong size);
    ULong tell() const { return m_alloc_offset+m_offset; }
    void flush();
    void close();

private:
    void remap();
    void remap_core(size_t size, off_t offset);
    int current_view_size() const { return m_alloc_granularity; }

private:
    std::string m_fname;
    int  m_hfile;                // handle to underlying file
    Byte*  m_base_ptr;          // base ptr for the current mapping
    UInt   m_offset;            // current offset from the base_ptr
    int m_alloc_granularity;   // allocation granularity - size of the current view
    int m_alloc_offset;      // offset of the current mapping
};

}} // namespace jag::jstd

#endif // MMAP_JG147_H__
/** EOF @file */

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __STREAMHELPERS_H__1221224
#define __STREAMHELPERS_H__1221224

#include <string>

namespace jag {
class ISeqStreamOutput;
class ISeqStreamInput;
class IStreamInput;

namespace jstd {

class MemoryStreamOutput;

/// creates a string from the memory stream contents
std::string GetStringFromMemoryStream(MemoryStreamOutput const& mem_stream);

/// writes a string to the memory stream
void WriteStringToSeqStream(ISeqStreamOutput& seq_stream, char const* str);

void copy_stream(ISeqStreamInput& src, ISeqStreamOutput& dest);

std::auto_ptr<IStreamInput> create_stream_from_memory(void const* mem, size_t memsize);


}} //namespace jag::jstd

#endif //__STREAMHELPERS_H__1221224


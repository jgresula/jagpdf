// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __OBJFMT_H__
#define __OBJFMT_H__

#include "objfmtbasic.h"
#include "interfaces/object_type.h"
#include <core/generic/noncopyable.h>
#include <unicode/utypes.h>

namespace jag {
namespace jstd
{
  class UnicodeConverterStream;
  class trans_affine_t;
}
namespace pdf {

//fwd
class IIndirectObject;
class IObjFmtReport;
class IndirectObjectRef;

/**
 * @brief writes pdf tokens to a ISeqStreamOutput
 *
 * intentionally copyable
 */
class ObjFmt
    : public noncopyable
{
public:
    ObjFmt(ISeqStreamOutput& out_stream, IObjFmtReport* report, jstd::UnicodeConverterStream&  utf8_to_16be);
    void flush() {}
    void encryption_stream(EncryptionStream* enc_stream) { m_fmt_basic.encryption_stream(enc_stream); }
    ObjFmtBasic& fmt_basic() { return m_fmt_basic; }

public:
    ObjFmt& object_end(ObjectType type, IIndirectObject& obj);
    ULong object_start(ObjectType type, IIndirectObject& obj);
    ObjFmt& ref(IIndirectObject const& iobject);
    ObjFmt& ref(IndirectObjectRef const& object);
    ObjFmt& ref_array(IndirectObjectRef const* objects, size_t num_objects);

public:
    ObjFmt& array_start() { m_fmt_basic.array_start(); return *this; }
    ObjFmt& array_end() { m_fmt_basic.array_end(); return *this; }
    ObjFmt& dict_start() { m_fmt_basic.dict_start(); return *this; }
    ObjFmt& dict_end() { m_fmt_basic.dict_end(); return *this; }
    ObjFmt& dict_key(Char const* key) { m_fmt_basic.dict_key(key); return *this; }
    ObjFmt& space()  { m_fmt_basic.space(); return *this; }
    ObjFmt& output(Int value) { m_fmt_basic.output(value); return *this; }
    ObjFmt& output(UInt value) { m_fmt_basic.output(value); return *this; }
    ObjFmt& output(double value)  { m_fmt_basic.output(value); return *this; }
    ObjFmt& output(Char const* value)  { m_fmt_basic.output(value); return *this; }
    ObjFmt& output_bool(bool value) { m_fmt_basic.output_bool(value); return *this; }
    ObjFmt& output_hex(UInt value, size_t bytes) { m_fmt_basic.output_hex(value, bytes); return *this; }
    ObjFmt& output(jstd::trans_affine_t const& value) { m_fmt_basic.output(value); return *this; }
    ObjFmt& name(Char const* value) { m_fmt_basic.name(value); return *this; }
    ObjFmt& rectangle(Double llx, Double lly, Double urx, Double ury) { m_fmt_basic.rectangle(llx, lly, urx, ury); return *this; }
    ObjFmt& null() { m_fmt_basic.null(); return *this; }

    ObjFmt& raw_text(Char const* value) { m_fmt_basic.raw_text(value); return *this; }
    ObjFmt& raw_bytes(void const* data, size_t length) {m_fmt_basic.raw_bytes(data, length); return *this;}
    ObjFmt& stream_data(void const* data, size_t length) { m_fmt_basic.stream_data(data, length); return *this; }

    ObjFmt& text_string(Char const* txt, size_t length, bool is_utf8=true) { m_fmt_basic.text_string(txt, length, is_utf8); return *this; }
    ObjFmt& text_string(std::string const& txt, bool is_utf8=true) { m_fmt_basic.text_string(txt, is_utf8); return *this; }
    ObjFmt& text_string(Char const* txt) { m_fmt_basic.text_string(txt); return *this; }
    ObjFmt& text_string_hex(Char const* txt, size_t length) { m_fmt_basic.text_string_hex(txt, length); return *this; }

    ObjFmt& unenc_text_string(Char const* txt, size_t length) { m_fmt_basic.unenc_text_string(txt, length); return *this; }
    ObjFmt& unenc_text_string(Char const* txt) { m_fmt_basic.unenc_text_string(txt); return *this; }
    ObjFmt& unenc_text_string_hex(Char const* txt, size_t length) { m_fmt_basic.unenc_text_string_hex(txt, length); return *this; }

    /// Code points.
    ObjFmt& string_object_2b(UInt16 const* txt, std::size_t length) { m_fmt_basic.string_object_2b(txt, length); return *this; }
    ObjFmt& string_object_2b(Char const* start, Char const* end) { m_fmt_basic.string_object_2b(start, end); return *this; }


    template<class HANDLE>
    ObjFmt& output_resource(HANDLE const& handle) {
        m_fmt_basic.output_resource(handle);
        return *this;
    }


private:
    ISeqStreamOutput*    m_stream;
    ObjFmtBasic            m_fmt_basic;
    IObjFmtReport*        m_report;
};


//
// Outputs a range of objects.
//
template<class It>
void output_array(ObjFmt& fmt, It it, It end)
{
    fmt.array_start();
    --end;
    for(; it!=end; ++it)
    {
        fmt.output(*it).space();
    }
    fmt.output(*it).array_end();
}

//
// Outputs a dictionary item consisting of a key and an array value. If the
// array is empty then nothing is outputted.
//
template<class It>
void output_array(char const*key, ObjFmt& fmt, It it, It end)
{
    if (it!=end)
    {
        fmt.dict_key(key);
        output_array(fmt, it, end);
    }
}

//
// Outputs a range of references
//
template<class It>
void output_ref_array(ObjFmt& fmt, It it, It end)
{
    fmt.array_start();
    --end;
    for(; it!=end; ++it)
    {
        fmt.ref(*it).space();
    }
    fmt.ref(*it).array_end();
}



}} //namespace jag::pdf

#endif //__OBJFMT_H__

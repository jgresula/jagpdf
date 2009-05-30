// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef UNICODE_H_JAG_2135__
#define UNICODE_H_JAG_2135__

#include <interfaces/stdtypes.h>
#include <core/generic/noncopyable.h>
#include <resources/interfaces/charencoding.h>
#include <interfaces/streams.h>
#include <boost/noncopyable.hpp>
#include <vector>
#include <memory>
#include <list>

struct UConverter;

namespace jag {
namespace jstd {


/// Retrives canonical name for given encoding.
char const* get_canonical_converter_name(Char const* enc);


//
// Encapsulates from/to unicode conversion
//
class UnicodeConverter
    : public noncopyable
{
public:
    UnicodeConverter(Char const* encoding);
    ~UnicodeConverter();
    std::auto_ptr<UnicodeConverter> clone() const;
    void reset();

public: // conversions
    Int next_code_point(Char const** source, Char const* end);
    Int from_codepoint(Int codepoint, Char* buffer, unsigned buffer_size);

public: // internal object
    UConverter* conv_internal() { return m_converter; }

private:
    UnicodeConverter(UConverter const* other);
    UConverter* m_converter;
};


//
//
//
class UnicodeConverterStream
    : public ISeqStreamOutput
{
public:
    UnicodeConverterStream(Char const* src,
                            Char const* dst,
                            ISeqStreamOutput* out=0,
                            bool write_bom = false);

    void reset(ISeqStreamOutput* out, bool write_bom=false);

public: //ISeqStreamOutput
    void write(void const* data, ULong size);
    ULong tell() const { return m_pos; }
    void flush() { m_out_stream->flush(); }

private:
    UnicodeConverter   m_src_conv;
    UnicodeConverter   m_dst_conv;
    ISeqStreamOutput*  m_out_stream;
    ULong m_pos;
    bool m_bom;
};



// fwd
class UnicodeToCP;

//
// Iterator of consistent strings (a string that can be encoded by a single
// codepage). Retrieved by class UnicodeToCP.
//
class UnicodeToCPIterator
{
public:
    UnicodeToCPIterator(UnicodeToCP& obj, Char const* begin, Char const* end);
    EnumCharacterEncoding *next(std::vector<Char>& str);

private:
    void consistent_string(UConverter* cnv, std::vector<Char>& str);

private:
    UnicodeToCP* m_obj;
    char const* m_current;
    char const* m_end;
};


//
// Maps unicode to a specified set of code pages. The input is utf-8 encoded.
//
// Currently, only 8-bit codepages are supported.
//
class UnicodeToCP
    : boost::noncopyable
{
public:
    // the encoding sequence is not copied
    UnicodeToCP(EnumCharacterEncoding const* begin,
                EnumCharacterEncoding const* end);

    ~UnicodeToCP();
    UnicodeToCPIterator create_iterator(Char const* begin,
                                        Char const* end);

    friend class UnicodeToCPIterator;
private:
    struct rec_t {
        rec_t(EnumCharacterEncoding enc_, UConverter* conv_)
            : enc(enc_), conv(conv_) {}
        EnumCharacterEncoding enc;
        UConverter* conv;
    };

private:
    typedef std::list<rec_t> List;
    typedef List::iterator RecordIter;

    EnumCharacterEncoding const* m_begin_enc;
    EnumCharacterEncoding const* m_end_enc;
    UConverter* m_cnv_utf8;
    List m_records;
};



}} //namespace jag::jstd


#endif //UNICODE_H_JAG_2135__

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __ENCRYPTION__STREAM_H__
#define __ENCRYPTION__STREAM_H__

#include <interfaces/streams.h>
#include <boost/scoped_ptr.hpp>

#include <stack>

namespace jag { namespace pdf
{

class ISecurityHandler;
class IIndirectObject;

/**
 * @brief ISeqStreamOutput implementation providing encryption
 *
 * Implements algorithm 3.1 (3.5.1 General Encryption Algorithm). Is parametrized
 * by the following stream and a security handler which provides the encryption
 * key.
 *
 * To behave correctly, object_start() and object_end() method must called properly.
 *
 * The following stream is the top of the stack of streams. The stack of stream
 * can be modified by push_stream() and pop_stream().
 *
 * This is a general implementation. Now, it supports arcfour. However it can be
 * extended by specifying type of encryption to be used.
 */
class EncryptionStream
    : public jag::ISeqStreamOutput
{
public:
    EncryptionStream(ISeqStreamOutput& out_stream, ISecurityHandler& sec_handler);
    void object_start(IIndirectObject& obj);
    void object_end(IIndirectObject& obj);
    /// causes reset of the encryption stream
    void reseed() { m_encrypter.reset(); }

    void push_stream(ISeqStreamOutput& stream);
    void pop_stream();
    ISeqStreamOutput& top_stream() const;

public: //ISeqStreamOutput
    void write(void const* data, ULong size);
    ULong tell() const;
    void flush();

private:
    void ensure_enc_stream() const;


private:
    std::stack<ISeqStreamOutput*>                m_stream_stack;
    ISecurityHandler&                            m_sec_handler;
    mutable boost::scoped_ptr<ISeqStreamOutput>    m_encrypter;
    Int                                        m_current_obj_number;
    Int                                        m_current_generation_number;
};

}}  // namespace jag::pdf


#endif//__ENCRYPTION__STREAM_H__

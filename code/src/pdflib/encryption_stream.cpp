// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "encryption_stream.h"
#include "interfaces/security_handler.h"
#include "interfaces/indirect_object.h"

#include <core/generic/minmax.h>
#include <core/generic/macros.h>
#include <core/generic/assert.h>
#include <core/jstd/arcfour_stream.h>
#include <core/jstd/md5.h>

namespace jag { namespace pdf
{

/**
 * @brief Constructor
 *
 * @param out_stream the encrypted data are written to this stream
 * @param sec_handler security handler to use
 */
EncryptionStream::EncryptionStream(ISeqStreamOutput& out_stream, ISecurityHandler& sec_handler)
    : m_sec_handler(sec_handler)
    , m_current_obj_number(-1)
    , m_current_generation_number(-1)
{
    push_stream(out_stream);
}

/**
 * @brief invoke this method whenever an indirect object is about to be outputted
 *
 * @param obj object which is going to output now
 */
void EncryptionStream::object_start(IIndirectObject& obj)
{
    JAG_ASSERT_MSG(
           m_current_obj_number == -1
        && m_current_generation_number == -1
        , "nested object started"
   );

    m_current_obj_number = obj.object_number();
    m_current_generation_number = obj.generation_number();
}

/**
* @brief invoke this method whenever output of an indirect object has been finished
*
* @param obj an indirect object
*/
void EncryptionStream::object_end(IIndirectObject& obj)
{
    JAG_UNUSED_FUNCTION_ARGUMENT(obj);
    JAG_ASSERT_MSG(
           obj.object_number() == m_current_obj_number
        && obj.generation_number() == m_current_generation_number
        , "object start/end mismatch"
   );

    m_current_obj_number = -1;
    m_current_generation_number = -1;
    m_encrypter.reset();
}

//////////////////////////////////////////////////////////////////////////
void EncryptionStream::write(void const* data, ULong size)
{
    ensure_enc_stream();
    m_encrypter->write(data, size);
}

//////////////////////////////////////////////////////////////////////////
ULong EncryptionStream::tell() const
{
    JAG_ASSERT(!"oops, not sure here");
    return top_stream().tell();
}

//////////////////////////////////////////////////////////////////////////
void EncryptionStream::flush()
{
    JAG_ASSERT(!"oops, not sure here");
    top_stream().flush();
}

/**
 * @brief makes sure that the encryptions stream is ready
 */
void EncryptionStream::ensure_enc_stream() const
{
    if (m_encrypter)
        return;

    JAG_ASSERT(m_current_obj_number!=-1 && m_current_generation_number!=-1);

    Byte const* enc_key_from_sec_handler = m_sec_handler.encoding_key();
    int sec_handler_key_bit_length = m_sec_handler.key_bit_length();

    int key_byte_length = sec_handler_key_bit_length/8;
    Byte enc_key[16+5];
    memcpy(enc_key, enc_key_from_sec_handler, key_byte_length);

    //Algorithm 3.1 Encryption of data using the RC4 or AES algorithms
    //    1. Obtain the object number and generation number from the object identifier of the
    //       string or stream to be encrypted (see Section 3.2.9, �Indirect Objects�). If the
    //       string is a direct object, use the identifier of the indirect object containing it.
    //    2. Treating the object number and generation number as binary integers, extend the
    //       original n-byte encryption key to n + 5 bytes by appending the low-order 3 bytes
    //       of the object number and the low-order 2 bytes of the generation number in that
    //       order, low-order byte first. (n is 5 unless the value of V in the encryption dictionary
    //       is greater than 1, in which case n is the value of Length divided by 8.)
    enc_key[key_byte_length] = static_cast<Byte>(m_current_obj_number);
    enc_key[key_byte_length+1] = static_cast<Byte>(m_current_obj_number>>8);
    enc_key[key_byte_length+2] = static_cast<Byte>(m_current_obj_number>>16);
    enc_key[key_byte_length+3] = static_cast<Byte>(m_current_generation_number);
    enc_key[key_byte_length+4] = static_cast<Byte>(m_current_generation_number>>8);

    //    3. Initialize the MD5 hash function and pass the result of step 2 as input to this function.
    jstd::md5_byte_t md5sum[16];
    jstd::md5_calculate(enc_key, key_byte_length+5, md5sum);

    //    4. Use the first (n + 5) bytes, up to a maximum of 16, of the output from the MD5
    //       hash as the key for the RC4 or AES symmetric key algorithms, along with the
    //       string or stream data to be encrypted.
    int key_bit_length = 8*min(key_byte_length+5, 16);
    m_encrypter.reset(new jstd::ArcFourStream(top_stream(), md5sum, key_bit_length));

    //       If using the AES algorithm, the Cipher Block Chaining (CBC) mode, which requires
    //       an initialization vector, is used. The block size parameter is set to 16 bytes,
    //       and the initialization vector is a 16-byte random number that is stored as the first
    //       16 bytes of the encrypted stream or string.
    //    The output is the encrypted data to be stored in the PDF file.
}

/// retrieves the current following stream
ISeqStreamOutput& EncryptionStream::top_stream() const
{
    return *m_stream_stack.top();
}

/// pushes a stream on the top of the stack (and makes it the following)
void EncryptionStream::push_stream(ISeqStreamOutput& stream)
{
    m_stream_stack.push(&stream);
}

/// pops a stream from the top of the stack
void EncryptionStream::pop_stream()
{
    m_stream_stack.pop();
}


}}  // namespace jag::pdf

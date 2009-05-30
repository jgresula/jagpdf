// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//



//http://www.koders.com/c/fid6A6E9922FF8ABB4A7F6310D78F9C12C4BB5A52BE.aspx?s=pdf+encryption
#include "precompiled.h"
#include "standard_security_handler.h"
#include "objfmt.h"
#include "docwriterimpl.h"
#include <core/generic/assert.h>
#include <core/generic/stringutils.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>


#include <core/errlib/errlib.h>
#include <msg_pdflib.h>
#include <core/jstd/md5.h>
#include <core/jstd/arcfour_stream.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/optionsparser.h>
#include <boost/spirit/include/classic_symbols.hpp>
using namespace jag::jstd;
using namespace boost;

namespace
{
    const unsigned int revision_3_perm_mask = 0xF00;
}

namespace jag {
namespace pdf {

namespace
{
  const unsigned NO_PRINT =         ~((1u<<2) | (1u<<11));
  const unsigned NO_MODIFY =        ~(1u<<3);
  const unsigned NO_COPY =          ~(1u<<4);
  const unsigned NO_MODIFY_EX =     ~(1u<<5);
  const unsigned NO_FORMS =         ~(1u<<8);
  const unsigned NO_ACCESSIBILITY = ~(1u<<9);
  const unsigned NO_ASSEMBLE =      ~(1u<<10);
  const unsigned NO_HIRES =         ~(1u<<11);

  unsigned const g_permissions[] = {
      NO_PRINT, NO_MODIFY, NO_COPY, NO_MODIFY_EX,
      NO_FORMS, NO_ACCESSIBILITY, NO_ASSEMBLE, NO_HIRES, 0
  };

  struct PermissionBits
      : public spirit::classic::symbols<unsigned>
  {
      PermissionBits()
      {
          add
              ("no_print", NO_PRINT)                          // bit 3
              ("no_modify", NO_MODIFY)                        // bit 4
              ("no_copy", NO_COPY)                            // bit 5
              ("no_modify_ex", NO_MODIFY_EX)                  // bit 6
              ("no_forms", NO_FORMS)                          // bit 9
              ("no_extract_accessibility", NO_ACCESSIBILITY)  // bit 10
              ("no_assemble", NO_ASSEMBLE)                    // bit 11
              ("no_hires_print", NO_HIRES)                    // bit 12
              ;
      }
  } g_permission_bits;
}

/**
 * @brief Constructor.
 *
 * @param body body
 * @param owner_pwd owner password
 * @param user_pwd user password
 */
StandardSecurityHandler::StandardSecurityHandler(DocWriterImpl& body)
    : IndirectObjectImpl(body)
    , m_permissions(0xFFFFFFFC)
{
#    ifdef JAG_DEBUG
    memset(m_enc_key, 0, sizeof(m_enc_key));
    memset(m_o_dict_field, 0, sizeof(m_enc_key));
#    endif //JAG_DEBUG

    IProfileInternal const& cfg = body.exec_context().config();

    Char const* owner_pwd = cfg.get("stdsh.pwd_owner");
    Char const* user_pwd = cfg.get("stdsh.pwd_user");
    Char const* permissions = cfg.get("stdsh.permissions");

    if (!is_empty(permissions))
    {
        ParsedResult const& p =
            parse_options(permissions, ParseArgs(0, &g_permission_bits));

        for(unsigned const* pperm = g_permissions; *pperm; ++pperm)
        {
            if (p.has_explicit_value(*pperm))
                m_permissions &= *pperm;
        }
    }

    if (body.version() >= 4)
    {
        m_revision = 3;
        m_algorithm = 2;
        m_key_bit_length = 128;
    }
    else
    {
        m_revision = 2;
        m_algorithm = 1;
        m_key_bit_length = 40;
        m_permissions |= revision_3_perm_mask;
    }

    // these three functions must go exactly in this order
    compute_owner_password(owner_pwd ? owner_pwd : user_pwd, user_pwd);
    compute_encryption_key(user_pwd ? user_pwd : "", m_enc_key);
    compute_user_password();
}

//////////////////////////////////////////////////////////////////////////
IIndirectObject& StandardSecurityHandler::indirect_object()
{
    return *this;
}

//////////////////////////////////////////////////////////////////////////
void StandardSecurityHandler::on_output_definition()
{
    ObjFmt& writer = object_writer();
    writer
        .dict_start()
        .dict_key("Filter").output("Standard")
        .dict_key("V").space().output(m_algorithm)
        .dict_key("R").space().output(/*static_cast<int>(*/m_revision)
        .dict_key("O").unenc_text_string(static_cast<char*>(static_cast<void*>(m_o_dict_field)), 32)
        .dict_key("U").unenc_text_string( static_cast<char*>(static_cast<void*>(m_u_dict_field)), 32)
        .dict_key("P").space().output(m_permissions)
    ;

    if (m_algorithm==2 || m_algorithm==3)
        writer.dict_key("Length").space().output(m_key_bit_length);

    writer.dict_end();
}

namespace //anonymous
{

/// padding string (algorithm 3.2, step 1)
const unsigned char pwd_padding_string[32] = {
    0x28, 0xBF, 0x4E, 0x5E, 0x4E, 0x75, 0x8A, 0x41, 0x64, 0x00, 0x4E, 0x56,
    0xFF, 0xFA, 0x01, 0x08, 0x2E, 0x2E, 0x00, 0xB6, 0xD0, 0x68, 0x3E, 0x80,
    0x2F, 0x0C, 0xA9, 0xFE, 0x64, 0x53, 0x69, 0x7A };


/**
 * @brief calculates padding string as described in algorithm 3.2, step 1
 *
 * @param pwd password to pad
 * @param padded padded password
 */
void pad_pwd(char const* pwd, unsigned char padded[32])
{
    size_t pwd_len = std::min<size_t>(32, strlen(pwd));
    memcpy(padded, pwd, pwd_len);
    memcpy(padded + pwd_len, pwd_padding_string, 32-pwd_len);
}
} // anonymous namespace


/**
 * @brief Computes o value of the encryption dictionary (Algorithm 3.3).
 *
 * @param owner_pwd owner password
 * @param user_pwd user password
 */
void StandardSecurityHandler::compute_owner_password(char const* owner_pwd, char const* user_pwd)
{
    // Algorithm 3.3 Computing the encryption dictionary�s O (owner password) value

    //   1. Pad or truncate the owner password string as described in step 1 of Algorithm 3.2.
    //      If there is no owner password, use the user password instead. (See implementation
    //      note 26 in Appendix H.)
    unsigned char padded_pwd[32];
    pad_pwd(owner_pwd, padded_pwd);


    //   2. Initialize the MD5 hash function and pass the result of step 1 as input to this function.
    jstd::MD5Hash::Sum    md5sum;
    jstd::md5_calculate(padded_pwd, 32, md5sum);

    //   3. (Revision 3 or greater) Do the following 50 times: Take the output from the previous
    //        MD5 hash and pass it as input into a new MD5 hash
    if (m_revision >= 3)
    {
        jstd::md5_state_t ctx;
        for(int i=50; i--;)
        {
            jstd::md5_init(&ctx);
            jstd::md5_append(&ctx, md5sum, 16);
            jstd::md5_finish(&ctx, md5sum);
        }
    }

    //   4. Create an RC4 encryption key using the first n bytes of the output from the final
    //        MD5 hash, where n is always 5 for revision 2 but, for revision 3 or greater, depends
    //        on the value of the encryption dictionary�s Length entry.
    jstd::MemoryStreamOutputFixedSize mem_out(m_o_dict_field, 32);
    jstd::ArcFourStream arcfour(mem_out, md5sum, m_key_bit_length);

    //  5. Pad or truncate the user password string as described in step 1 of Algorithm 3.2.
    pad_pwd(user_pwd, padded_pwd);

    //  6. Encrypt the result of step 5, using an RC4 encryption function with the encryption
    //     key obtained in step 4.
    arcfour.write(padded_pwd, 32);

    //  7. (Revision 3 or greater) Do the following 19 times: Take the output from the previous
    //     invocation of the RC4 function and pass it as input to a new invocation of the
    //     function; use an encryption key generated by taking each byte of the encryption
    //     key obtained in step 4 and performing an XOR (exclusive or) operation between
    //     that byte and the single-byte value of the iteration counter (from 1 to 19).
    if (m_revision >= 3)
    {
        cycle19(&m_o_dict_field[0], 32, &md5sum[0]);
    }

    //  8. Store the output from the final invocation of the RC4 function as the value of the
    //     O entry in the encryption dictionary.
    // done in step 6
}

void StandardSecurityHandler::cycle19(Byte* in, int len, Byte* enc_key)
{
    JAG_ASSERT_MSG(len <= 32, "constant exceeded");
    unsigned char    tmp[32];
    unsigned char* input = in;
    unsigned char* output = tmp;
    const int key_byte_length = m_key_bit_length/8;

    for(int i=1; i<=19; ++i)
    {
        // use an encryption key generated by taking each byte of the encryption
        // key [..] and performing an XOR (exclusive or) operation between
        // that byte and the single-byte value of the iteration counter (from 1 to 19).
        jstd::MD5Hash::Sum    key;
        for(int j=0; j<key_byte_length; ++j)
            key[j] = enc_key[j] ^ static_cast<Byte>(i);

        jstd::MemoryStreamOutputFixedSize mem_out(output, len);
        jstd::ArcFourStream arcfour(mem_out, key, m_key_bit_length);
        arcfour.write(input, len);
        std::swap(input, output);
    }
    memcpy(in, tmp, len);
}

//////////////////////////////////////////////////////////////////////////
//
/**
 * @brief Computes o value of the encryption dictionary (Algorithm 3.4).
 *
 * pre: m_enc_key must be already computed
 */
void StandardSecurityHandler::compute_user_password()
{
    // pre
    JAG_ASSERT(sizeof(m_enc_key) != std::count(m_enc_key, m_enc_key+sizeof(m_enc_key), 0));

    if (m_revision == 2)
    {
        //Algorithm 3.4 Computing the encryption dictionary's U (user password) value (Revision 2)
        //    1. Create an encryption key based on the user password string, as described in Algorithm
        //       3.2.
        //ready in 'm_enc_key'

        //    2. Encrypt the 32-byte padding string shown in step 1 of Algorithm 3.2, using an
        //       RC4 encryption function with the encryption key from the preceding step.
        //    3. Store the result of step 2 as the value of the U entry in the encryption dictionary.
        jstd::MemoryStreamOutputFixedSize mem_out(m_u_dict_field, 32);
        jstd::ArcFourStream arcfour(mem_out, m_enc_key, m_key_bit_length);
        arcfour.write(pwd_padding_string, 32);
    }
    else // m_revision >= 3
    {
        //Algorithm 3.5 Computing the encryption dictionary's U (user password) value (Revision 3
        //    or greater)
        //1. Create an encryption key based on the user password string, as described in Algorithm
        //    3.2.

        //ready in 'm_enc_key'

        //2. Initialize the MD5 hash function and pass the 32-byte padding
        //string shown in step 1 of Algorithm 3.2 as input to this function.
        jstd::MD5Hash md5_hash;
        md5_hash.append(pwd_padding_string, 32);

        //3. Pass the first element of the file's file identifier array (the
        //value of the ID entry in the document's trailer dictionary; see Table
        //3.13 on page 73) to the hash function and finish the hash. (See
        //implementation note 25 in Appendix H.)
        md5_hash.append(doc().file_id(), sizeof(DocWriterImpl::FileID));
        md5_hash.finish();

        //4. Encrypt the 16-byte result of the hash, using an RC4 encryption
        //function with the encryption key from step 1.
        jstd::MemoryStreamOutputFixedSize mem_out(m_u_dict_field, 32);
        jstd::ArcFourStream arcfour(mem_out, m_enc_key, m_key_bit_length);
        arcfour.write(md5_hash.sum(), 16);

        //5. Do the following 19 times: Take the output from the previous
        //invocation of the RC4 function and pass it as input to a new
        //invocation of the function; use an encryption key generated by taking
        //each byte of the original encryption key (obtained in step 1) and
        //performing an XOR (exclusive or) operation between that byte and the
        //single-byte value of the iteration counter (from 1 to 19).
        cycle19(m_u_dict_field, 16, m_enc_key);

        //6. Append 16 bytes of arbitrary padding to the output from the final
        //invocation of the RC4 function and store the 32-byte result as the
        //value of the U entry in the encryption dictionary.

        // this step is actually not needed, we could left the second 16 bytes
        // as they are but we want deterministic tests
        memcpy(m_u_dict_field+16, pwd_padding_string, 16);
    }
}


/**
 * @brief Computes an encryption key (Algorithm 3.2).
 *
 * pre: assumes m_o_dict_field is assigned
 *
 * @param pwd password required by step 1 of the algorithm
 * @param key computed encryption key (its length can be retrieved from key_bit_length()
 */
void StandardSecurityHandler::compute_encryption_key(char const* pwd, unsigned char key[16])
{
    // pre
    JAG_ASSERT(sizeof(m_enc_key) == std::count(m_enc_key, m_enc_key+sizeof(m_enc_key), 0));
    JAG_ASSERT(sizeof(m_o_dict_field) != std::count(m_o_dict_field, m_o_dict_field+sizeof(m_o_dict_field), 0));

    //Algorithm 3.2 Computing an encryption key
    //    1. Pad or truncate the password string to exactly 32 bytes. If the password string is
    //       more than 32 bytes long, use only its first 32 bytes; if it is less than 32 bytes long,
    //       pad it by appending the required number of additional bytes from the beginning
    //       of the following padding string:
    //       <padding string snipped>
    //       That is, if the password string is n bytes long, append the first 32 - n bytes of the
    //       padding string to the end of the password string. If the password string is empty
    //       (zero-length), meaning there is no user password, substitute the entire padding
    //       string in its place.
    unsigned char padded_pwd[32];
    pad_pwd(pwd, padded_pwd);

    //    2. Initialize the MD5 hash function and pass the result of step 1 as input to this function.
    jstd::MD5Hash md5_hash;
    md5_hash.append(padded_pwd, 32);

    //  3. Pass the value of the encryption dictionary�s O entry to the MD5 hash function.
    //       (Algorithm 3.3 shows how the O value is computed.)
    md5_hash.append(m_o_dict_field, 32);

    //    4. Treat the value of the P entry as an unsigned 4-byte integer and pass these bytes to
    //       the MD5 hash function, low-order byte first.
    for(int i=0; i<25; i+=8)
    {
        unsigned char val = static_cast<unsigned char>(m_permissions >> i);
        md5_hash.append(&val, 1);
    }

    //    5. Pass the first element of the fil's file identifier array (the value of the ID entry in
    //       the document's trailer dictionary; see Table 3.13 on page 73) to the MD5 hash
    //       function. (See implementation note 25 in Appendix H.)
    md5_hash.append(doc().file_id(), sizeof(DocWriterImpl::FileID));

    //    6. (Revision 3 or greater) If document metadata is not being encrypted, pass 4 bytes
    //        with the value 0xFFFFFFFF to the MD5 hash function.
    if (m_revision >= 3)
    {
        // md5_hash.append(0xFFFFFFFF, 4);
    }

    //    7. Finish the hash.
    jstd::MD5Hash::Sum& md5sum = md5_hash.finish();

    //    8. (Revision 3 or greater) Do the following 50 times: Take the output from the previous
    //       MD5 hash and pass the first n bytes of the output as input into a new MD5
    //       hash, where n is the number of bytes of the encryption key as defined by the value
    //       of the encryption dictionary's Length entry.
    const int key_byte_length = m_key_bit_length/8;

    if (m_revision >= 3)
    {
        jstd::md5_state_t ctx;
        for(int i=50; i--;)
        {
            jstd::md5_init(&ctx);
            jstd::md5_append(&ctx, md5sum, key_byte_length /*16*/);
            jstd::md5_finish(&ctx, md5sum);
        }
    }

    //    9. Set the encryption key to the first n bytes of the output from the final MD5 hash,
    //       where n is always 5 for revision 2 but, for revision 3 or greater, depends on the value
    //       of the encryption dictionary�s Length entry.
    memcpy(key, md5sum, key_byte_length);
}


}} //namespace jag::pdf


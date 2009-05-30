// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __STANDARD_SECURITY_HANDLER_H__
#define __STANDARD_SECURITY_HANDLER_H__

#include "indirectobjectimpl.h"
#include "interfaces/security_handler.h"
#include <boost/shared_ptr.hpp>

namespace jag
{
class ISeqStreamOutput;

namespace pdf
{

class IIndirectObject;
class DocWriterImpl;

/// standard security handler as described in 3.5.2
class StandardSecurityHandler
    : public ISecurityHandler
    , public IndirectObjectImpl
{
public:
    /// handler revision
    explicit StandardSecurityHandler(DocWriterImpl& body);

public: //IIndirectObject
    void on_output_definition();
    DEFINE_VISITABLE;

public: //ISecurityHandler
    int    key_bit_length() const { return m_key_bit_length; }
    Byte const* encoding_key() { return m_enc_key; }
    IIndirectObject& indirect_object();

private:
    void compute_owner_password(char const* owner_pwd, char const* user_pwd);
    void compute_user_password();
    void compute_encryption_key(char const* pwd, unsigned char key[16]);
    void cycle19(Byte* input, int len, Byte* enc_key);

private:
    int        m_revision;
    int        m_key_bit_length;
    int        m_algorithm;        // V field
    UInt    m_permissions;
    Byte    m_o_dict_field[32];    // O field of the encryption dictionary
    Byte    m_u_dict_field[32];    // U field of the encryption dictionary
    Byte    m_enc_key[16];
};

}} //namespace jag::pdf


#endif //__STANDARD_SECURITY_HANDLER_H__

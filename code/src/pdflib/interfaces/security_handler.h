// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __SECURITY_HANDLER_H__
#define __SECURITY_HANDLER_H__

#include <interfaces/stdtypes.h>
#include <core/generic/noncopyable.h>

namespace jag { namespace pdf
{

/// security handler
class ISecurityHandler
    : public noncopyable
{
public:
    /// retrieves bit length of the encoding key
    virtual int    key_bit_length() const = 0;
    /// retrieves encoding key
    virtual Byte const* encoding_key() = 0;
    /// retrieves IIndirectObject interface
    virtual IIndirectObject& indirect_object() = 0;
};


}} //namespace jag::pdf

#endif //__SECURITY_HANDLER_H__

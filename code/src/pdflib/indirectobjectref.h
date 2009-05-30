// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __INDIRECTOBJECTREF_H_JG853__
#define __INDIRECTOBJECTREF_H_JG853__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "interfaces/indirect_object.h"
#include <core/generic/assert.h>
#include <vector>

namespace jag {
namespace pdf {

class IndirectObjectRef;
bool is_valid(IndirectObjectRef const& obj);

class IndirectObjectRef
{
public:
    explicit IndirectObjectRef(IIndirectObject const& obj)
        : m_object_number(obj.object_number())
        , m_generation_number(obj.generation_number())
    {}

    /// creates an invalid reference
    IndirectObjectRef()
        : m_object_number(-1)
    {}

    Int object_number() const {
        return m_object_number;
    }

    Int generation_number() const {
        return m_generation_number;
    }

    void reset(IndirectObjectRef const& other) {
        m_object_number = other.m_object_number;
        m_generation_number = other.m_generation_number;
    }

    void reset(IIndirectObject const& other) {
        m_object_number = other.object_number();
        m_generation_number = other.generation_number();
    }

private:
    Int    m_object_number;
    Int    m_generation_number;
};

typedef std::vector<IndirectObjectRef> ObjectRefs;


inline bool is_valid(IndirectObjectRef const& obj)
{
    return obj.object_number() != -1;
}

}} //namespace jag::pdf


#endif //__indirectobjectref_h_JG853__

